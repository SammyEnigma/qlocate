#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QProcess>
#include <QSystemTrayIcon>
#include <QCloseEvent>
#include <QDir>
#include <QListWidgetItem>
#include <QTimer>
#include <QFileIconProvider>
#include <QDesktopWidget>
#include <QSettings>
#include <QStyle>
#include <QxtGlobalShortcut>
#include <QInputDialog>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    // initialize misc. variables
    ui->setupUi(this);

    reallyQuit = false;
    locate = NULL;
    originalLabelPalette = ui->labelStatus->palette();
    iconProvider = new QFileIconProvider;
    homePath = QDir::toNativeSeparators(QDir::homePath()) + QDir::separator();
    isSearching = false;

    // show the app is busy searching by making an animation of sorts,
    // this is done by showing ellipsis after "Searching "one at a time
    // "Searching.", "Searching..", "Searching..."
    animateEllipsisTimer = new QTimer(this);
    connect(animateEllipsisTimer, SIGNAL(timeout()), this, SLOT(animateEllipsis()));
    animateEllipsisTimer->setInterval(333);

    // initialize the auto-search timer
    // there is no search button in our app
    // application starts searching automatically
    // a fixed time interval after last key typed by user
    QTimer* autoStartSearchTimer = new QTimer(this);
    autoStartSearchTimer->setInterval(500);
    autoStartSearchTimer->setSingleShot(true);
    connect(autoStartSearchTimer, SIGNAL(timeout()), this, SLOT(startSearching()));
    connect(ui->lineEdit, SIGNAL(textEdited(QString)), autoStartSearchTimer, SLOT(start()));

    // initialize the tray icon
    // the application resides in the tray and when
    // user clicks on the tray icon the dialog is shown
    // this is to speed things up (no process loading and
    // initialization, so the app is more responsive)
    QSystemTrayIcon* trayIcon = new QSystemTrayIcon(this);
    connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(toggleVisible(QSystemTrayIcon::ActivationReason)));
    trayIcon->setIcon(qApp->windowIcon());
    trayIcon->setVisible(true);
    trayIcon->setContextMenu(ui->menuFile);

    // connect the file menu signals
    connect(ui->actionUpdate_Database, SIGNAL(triggered()), this, SLOT(startUpdateDB()));
    connect(ui->actionQuit, SIGNAL(triggered()), this, SLOT(quit()));

    // initialize list widget context menu
    // the user can right click on a found file to pop up the context menu
    // the context menu can be used to open the selected file (this is "Open File")
    // or to open the folder in which the selected file is (this is "Open Folder")
    ui->listWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    listWidgetContextMenu = new QMenu(this);
    listWidgetContextMenu->addAction(tr("Open"), this, SLOT(openFile()));
    listWidgetContextMenu->addAction(style()->standardIcon(QStyle::SP_DirOpenIcon), tr("Show in Folder"), this, SLOT(showFile()));
    connect(ui->listWidget, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showContextMenu(QPoint)));
    connect(ui->listWidget, SIGNAL(activated(QModelIndex)), this, SLOT(openFile()));

    // initialize the checkboxes for various options
    connect(ui->checkBoxCaseSensitive, SIGNAL(toggled(bool)), autoStartSearchTimer, SLOT(start()));
    connect(ui->checkBoxRegExp, SIGNAL(toggled(bool)), autoStartSearchTimer, SLOT(start()));
    connect(ui->checkBoxSearchOnlyHome, SIGNAL(toggled(bool)), autoStartSearchTimer, SLOT(start()));
    connect(ui->checkBoxShowFullPath, SIGNAL(toggled(bool)), this, SLOT(toggleFullPaths()));
    connect(ui->checkBoxSmartWildcard, SIGNAL(toggled(bool)), autoStartSearchTimer, SLOT(start()));
    connect(ui->checkBoxMatchWholePath, SIGNAL(toggled(bool)), autoStartSearchTimer, SLOT(start()));

    locate = new QProcess(this);
    connect(locate, SIGNAL(readyReadStandardOutput()), this, SLOT(readLocateOutput()));
    connect(locate, SIGNAL(finished(int)), this, SLOT(readLocateOutput()));

    readLocateOutputTimer = new QTimer(this);
    readLocateOutputTimer->setInterval(0);
    readLocateOutputTimer->setSingleShot(true);
    connect(readLocateOutputTimer, SIGNAL(timeout()), this, SLOT(readLocateOutput()));

    // place the window at the center of the screen
    QRect available_geom = QDesktopWidget().availableGeometry();
    QRect current_geom = frameGeometry();
    setGeometry(available_geom.width() / 2 - current_geom.width() / 2,
                            available_geom.height() / 2 - current_geom.height() / 2,
                            current_geom.width(),
                            current_geom.height());

    // activate the global shortcut
    globalHotKey = new QxtGlobalShortcut(this);
    globalHotKey->setEnabled(true);
    connect(globalHotKey, SIGNAL(activated()), this, SLOT(toggleVisible()));
    connect(ui->actionGlobal_Hotkey, SIGNAL(triggered()), this, SLOT(changeGlobalHotkey()));

    restoreSettings();
}

MainWindow::~MainWindow()
{
    saveSettings();
    delete iconProvider;
    delete ui;
}

void MainWindow::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void MainWindow::setLabelText(const QString& text)
{
    if (isLabelTextRed)
    {
        ui->labelStatus->setPalette(originalLabelPalette);
        isLabelTextRed = false;
    }
    ui->labelStatus->setText(text);
}

void MainWindow::setRedLabelText(const QString& text)
{
    if (!isLabelTextRed)
    {
        QPalette palette = originalLabelPalette;
        palette.setColor(ui->labelStatus->foregroundRole(), Qt::red);
        ui->labelStatus->setPalette(palette);
        isLabelTextRed = true;
    }
    ui->labelStatus->setText(text);
}

void MainWindow::startSearching()
{
    // if the query (and checkboxes) are the same,
    // no need to restart the same query
    QVariantList state;
    state
            << ui->checkBoxCaseSensitive->isChecked()
            << ui->checkBoxRegExp->isChecked()
            << ui->checkBoxSearchOnlyHome->isChecked()
            << ui->checkBoxMatchWholePath->isChecked()
            << ui->checkBoxSmartWildcard->isChecked()
            << ui->lineEdit->text();
    if (state == lastState)
        return;
    lastState = state;

    // if a previous search is running stop it
    stopSearching();

    // if there is no query just clear the list and stop any previous searches
    if (ui->lineEdit->text().isEmpty() || ui->lineEdit->text() == tr("<type here>"))
    {
        ui->listWidget->clear();
        setLabelText(tr("Ready."));
        return;
    }

    QString query = ui->lineEdit->text();
    if (ui->checkBoxSmartWildcard->isChecked() && !ui->checkBoxRegExp->isChecked())
    {
        query.replace(' ', '*');
        if (query[0] != '*' && query[query.size()-1] != '*')
            query = '*' + query + '*';
    }

    // the arguments to pass to locate
    QStringList args;
#ifdef Q_OS_WIN32
    args << "-lfd";
    if (ui->checkBoxRegExp->isChecked())
    {
        if (ui->checkBoxCaseSensitive->isChecked())
            args << "-rc";
        else
            args << "-r";
    }
    args << "--";
#else
    args << "--existing";
    if (!ui->checkBoxMatchWholePath->isChecked())
        args << "--basename";
    if (!ui->checkBoxCaseSensitive->isChecked())
        args << "--ignore-case";
    if (ui->checkBoxRegExp->isChecked())
        args << "--regexp";
#endif
    args << query;
    locate->start("locate", args);

    setLabelText(tr("Searching (press Esc to stop)..."));
    nextEllipsisCount = 1;
    animateEllipsisTimer->start();
    isSearching = true;
    setCursor(Qt::BusyCursor);
    isListBoxCleared = false;
}

void MainWindow::stopSearching()
{
    if (!isSearching)
        return;
    if (locate->state() != QProcess::NotRunning)
    {
        locate->terminate();
        locate->waitForFinished();
    }
    animateEllipsisTimer->stop();
    readLocateOutputTimer->stop();
    isSearching = false;
    unsetCursor();
}

void MainWindow::toggleVisible(QSystemTrayIcon::ActivationReason reason)
{
    if (QSystemTrayIcon::Trigger == reason)
        toggleVisible();
}

void MainWindow::toggleVisible()
{
    if (isVisible() && isActiveWindow() && !isMinimized())
    {
        close();
    }
    else
    {
        show();
        raise();
        activateWindow();
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    // if there is a search going on stop it
    if (isSearching)
    {
        stopSearching();
        setLabelText("Stopped.");
    }

    if (!reallyQuit)
    {
        hide();
        event->ignore();
    }
}

void MainWindow::readLocateOutput()
{
    // getting the icon for each file seems to be a lengthy operation
    // taking much longer than adding the item to the list
    // so getting the icons is made a separate cycle which can be interrupted
    // if there are pending events like keyboard input and such
    // that way the app is not unresponsive
    QVector<QListWidgetItem*> items;
    while (locate->canReadLine() && !qApp->hasPendingEvents())
    {
        QString filename = QString::fromUtf8(locate->readLine()).trimmed();

        if (ui->checkBoxSearchOnlyHome->isChecked() && filename.indexOf(homePath) != 0)
            continue;

        QListWidgetItem* item = new QListWidgetItem;
        item->setIcon(iconProvider->icon(QFileInfo(filename)));
        if (ui->checkBoxShowFullPath->isChecked())
            item->setData(Qt::DisplayRole, filename);
        else
            item->setData(Qt::DisplayRole, filename.mid(filename.lastIndexOf(QDir::separator())+1));
        item->setData(Qt::ToolTipRole, filename);

        items.push_back(item);
    }

    // now we add the collected filenames and icons to the list widget
    // this doesn't seem to take much time
    if (!isListBoxCleared)
    {
        ui->listWidget->clear();
        isListBoxCleared = true;
    }
    foreach(QListWidgetItem* item, items)
        ui->listWidget->addItem(item);

    // if there still are lines to be read then the first loop
    // was interrupted, so we schedule a timer to return to
    // this function after we have processed the pending events
    if (locate->canReadLine())
        readLocateOutputTimer->start();
    // if there are no more lines to be read, the locate process
    // might have ended
    else if (QProcess::NotRunning == locate->state())
    {
        stopSearching();

        switch (ui->listWidget->count())
        {
        case 0: setRedLabelText(tr("Nothing was found.")); break;
        case 1: setLabelText(tr("1 file was found.")); break;
        default: setLabelText(tr("%1 files were found.").arg(ui->listWidget->count())); break;
        }
    }
}

void MainWindow::quit()
{
    reallyQuit = true;
    close();
}

void MainWindow::openFile()
{
    foreach (QListWidgetItem* ii, ui->listWidget->selectedItems())
        openFile(ii->data(Qt::ToolTipRole).toString());
}

void MainWindow::showFile()
{
    foreach (QListWidgetItem* ii, ui->listWidget->selectedItems())
        showFile(ii->data(Qt::ToolTipRole).toString());
}

void MainWindow::startUpdateDB()
{
#ifdef Q_OS_WIN32
    openFile("updatedb");
#else
    QProcess::startDetached("gksudo updatedb");
#endif
}

void MainWindow::showContextMenu(QPoint p)
{
    if (!ui->listWidget->selectedItems().empty())
        listWidgetContextMenu->exec(ui->listWidget->mapToGlobal(p));
}

void MainWindow::animateEllipsis()
{
    QString text;
    switch(nextEllipsisCount)
    {
    case 1: text = tr("Searching (press Esc to stop).");   nextEllipsisCount = 2; break;
    case 2: text = tr("Searching (press Esc to stop)..");  nextEllipsisCount = 3; break;
    case 3: text = tr("Searching (press Esc to stop)..."); nextEllipsisCount = 1; break;
    }
    setLabelText(text);
}

QString MainWindow::currentFilename()
{
    int role = ui->checkBoxShowFullPath->isChecked() ? Qt::DisplayRole : Qt::ToolTipRole;
    return ui->listWidget->currentIndex().data(role).toString();
}

bool MainWindow::event(QEvent *e)
{
    bool res = QMainWindow::event(e);
    switch (e->type()) {
    case QEvent::WindowActivate:
        ui->lineEdit->selectAll();
        ui->lineEdit->setFocus();
        break;
    default:
        break;
    }

    return res;
}

void MainWindow::restoreSettings()
{
    QSettings settings;
    ui->checkBoxCaseSensitive->setChecked(settings.value("Options/CaseSensitive", ui->checkBoxCaseSensitive->isChecked()).toBool());
    ui->checkBoxRegExp->setChecked(settings.value("Options/RegExp", ui->checkBoxRegExp->isChecked()).toBool());
    ui->checkBoxSearchOnlyHome->setChecked(settings.value("Options/SearchOnlyHome", ui->checkBoxSearchOnlyHome->isChecked()).toBool());
    ui->checkBoxShowFullPath->setChecked(settings.value("Options/ShowFullPath", ui->checkBoxShowFullPath->isChecked()).toBool());
    ui->checkBoxSmartWildcard->setChecked(settings.value("Options/SpaceIsWildcard", ui->checkBoxSmartWildcard->isChecked()).toBool());
    ui->checkBoxMatchWholePath->setChecked(settings.value("Options/MatchWholePath", ui->checkBoxMatchWholePath->isChecked()).toBool());
    ui->checkBoxSaveWindowPosition->setChecked(settings.value("Options/SaveWindowPosition", ui->checkBoxSaveWindowPosition->isChecked()).toBool());
    if (ui->checkBoxSaveWindowPosition->isChecked())
        restoreGeometry(settings.value("Window/Geometry", saveGeometry()).toByteArray());
    globalHotKey->setShortcut(QKeySequence::fromString(settings.value("Options/GlobalHotkey", "Meta+G").toString()));
}

void MainWindow::saveSettings()
{
    QSettings settings;
    settings.setValue("Options/CaseSensitive", ui->checkBoxCaseSensitive->isChecked());
    settings.setValue("Options/RegExp", ui->checkBoxRegExp->isChecked());
    settings.setValue("Options/SearchOnlyHome", ui->checkBoxSearchOnlyHome->isChecked());
    settings.setValue("Options/ShowFullPath", ui->checkBoxShowFullPath->isChecked());
    settings.setValue("Options/SpaceIsWildcard", ui->checkBoxSmartWildcard->isChecked());
    settings.setValue("Options/MatchWholePath", ui->checkBoxMatchWholePath->isChecked());
    settings.setValue("Options/SaveWindowPosition", ui->checkBoxSaveWindowPosition->isChecked());
    settings.setValue("Window/Geometry", saveGeometry());
    settings.setValue("Options/GlobalHotkey", globalHotKey->shortcut().toString());
}

void MainWindow::toggleFullPaths()
{
    for (int ii=0; ii<ui->listWidget->count(); ii++)
    {
        QListWidgetItem* item = ui->listWidget->item(ii);
        QString filename = item->data(Qt::ToolTipRole).toString();
        if (ui->checkBoxShowFullPath->isChecked())
            item->setData(Qt::DisplayRole, filename);
        else
            item->setData(Qt::DisplayRole, filename.mid(filename.lastIndexOf(QDir::separator())+1));
    }
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (isSearching && event->key() == Qt::Key_Escape)
    {
        stopSearching();
        setLabelText("Stopped.");
    }

    QMainWindow::keyPressEvent(event);
}

void MainWindow::changeGlobalHotkey()
{
    QString lastGlobalHotKey = globalHotKey->shortcut().toString();
    while (1)
    {
        bool ok;
        QString text = QInputDialog::getText(this, tr("Change Global Hotkey"),
                                             tr("Global Hotkey:"), QLineEdit::Normal,
                                             lastGlobalHotKey, &ok);

        if (ok && !globalHotKey->setShortcut(QKeySequence::fromString(text)))
        {
            QMessageBox::warning(this, "", tr("Could not register global hotkey: '%1'").arg(text));
            globalHotKey->setShortcut(QKeySequence::fromString(lastGlobalHotKey));
        }
        else
        {
            break;
        }
    }
}
