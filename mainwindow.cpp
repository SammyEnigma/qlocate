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
    connect(autoStartSearchTimer, SIGNAL(timeout()), this, SLOT(startLocate()));
    connect(ui->lineEdit, SIGNAL(textEdited(QString)), autoStartSearchTimer, SLOT(start()));

    // initialize the tray icon
    // the application resides in the tray and when
    // user clicks on the tray icon the dialog is shown
    // this is to speed things up (no process loading and
    // initialization, so the app is more responsive)
    QSystemTrayIcon* trayIcon = new QSystemTrayIcon(this);
    connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(toggleVisible(QSystemTrayIcon::ActivationReason)));
    trayIcon->setIcon(QIcon(":/images/edit-find.svg"));
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
    connect(ui->listWidget, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showContextMenu(QPoint)));
    listWidgetContextMenu = new QMenu(this);
    listWidgetContextMenu->addAction(QIcon(":/images/document-open.svg"), tr("Open File"), this, SLOT(openFile()));
    listWidgetContextMenu->addAction(QIcon(":/images/folder-visiting.svg"), tr("Open Folder"), this, SLOT(showFile()));

    // initialize the checkboxes for various options
    oldCaseSensitive = false;
    oldUseRegExp = false;
    oldSearchOnlyHome = true;
    oldSpaceIsWildcard = false;
    connect(ui->checkBoxCaseSensitive, SIGNAL(toggled(bool)), this, SLOT(startLocate()));
    connect(ui->checkBoxRegExp, SIGNAL(toggled(bool)), this, SLOT(startLocate()));
    connect(ui->checkBoxSearchOnlyHome, SIGNAL(toggled(bool)), this, SLOT(startLocate()));
    connect(ui->checkBoxShowFullPath, SIGNAL(toggled(bool)), this, SLOT(toggleFullPaths()));
    connect(ui->checkBoxSpaceIsWildcard, SIGNAL(toggled(bool)), this, SLOT(startLocate()));
    connect(ui->listWidget, SIGNAL(activated(QModelIndex)), this, SLOT(openFile()));

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

void MainWindow::startLocate()
{
    if (oldSearchString == ui->lineEdit->text() &&
        oldUseRegExp == ui->checkBoxRegExp->isChecked() &&
        oldCaseSensitive == ui->checkBoxCaseSensitive->isChecked() &&
        oldSearchOnlyHome == ui->checkBoxSearchOnlyHome->isChecked() &&
        oldSpaceIsWildcard == ui->checkBoxSpaceIsWildcard->isChecked())
    {
        return;
    }

    oldUseRegExp = ui->checkBoxRegExp->isChecked();
    oldCaseSensitive  = ui->checkBoxCaseSensitive->isChecked();
    oldSearchOnlyHome = ui->checkBoxSearchOnlyHome->isChecked();
    oldSpaceIsWildcard = ui->checkBoxSpaceIsWildcard->isChecked();
    oldSearchString = ui->lineEdit->text();
    if (locate->state() != QProcess::NotRunning)
    {
        locate->terminate();
        locate->waitForFinished();
    }
    ui->listWidget->clear();

    ui->labelStatus->setPalette(originalLabelPalette);
    readLocateOutputTimer->stop();

    if (ui->lineEdit->text().isEmpty() || ui->lineEdit->text() == tr("<type here>"))
    {
        ui->labelStatus->setText(tr("Ready."));
        return;
    }

    ui->labelStatus->setText(tr("Searching..."));
    nextEllipsisCount = 1;
    animateEllipsisTimer->start();

    QString query = ui->lineEdit->text();
    if (ui->checkBoxSpaceIsWildcard->isChecked() && -1 != query.indexOf(' '))
    {
        if (ui->checkBoxRegExp->isChecked())
            query = ".*" + query.replace(" ", ".*") + ".*";
        else
            query = "*" + query.replace(" ", "*") + "*";
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
    args << "--existing" << "--basename";
    if (!ui->checkBoxCaseSensitive->isChecked())
        args << "--ignore-case";
    if (ui->checkBoxRegExp->isChecked())
        args << "--regexp";
#endif
    args << query;
    locate->start("locate", args);
}

void MainWindow::toggleVisible(QSystemTrayIcon::ActivationReason reason)
{
    if (QSystemTrayIcon::Trigger == reason)
    {
        if (isVisible() && isActiveWindow())
        {
            hide();
        }
        else
        {
            show();
            raise();
            activateWindow();
        }
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (locate)
        locate->terminate();
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
        locateFinished();
}

void MainWindow::quit()
{
    reallyQuit = true;
    close();
}

void MainWindow::openFile()
{
    if (ui->listWidget->currentItem() && ui->listWidget->currentItem()->isSelected())
        openFile(currentFilename());
}

void MainWindow::showFile()
{
    if (ui->listWidget->currentItem() && ui->listWidget->currentItem()->isSelected())
        showFile(currentFilename());
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
    listWidgetContextMenu->exec(ui->listWidget->mapToGlobal(p));
}

void MainWindow::locateFinished()
{
    int count = ui->listWidget->count();
    if (count > 1)
    {
        ui->labelStatus->setText(QString(tr("%1 files were found.").arg(ui->listWidget->count())));
    }
    else if (count == 1)
    {
        ui->labelStatus->setText(tr("1 file was found."));
    }
    else
    {
        QPalette palette = originalLabelPalette;
        palette.setColor(ui->labelStatus->foregroundRole(), Qt::red);
        ui->labelStatus->setPalette(palette);
        ui->labelStatus->setText(tr("Nothing was found."));
    }
}

void MainWindow::animateEllipsis()
{
    if (locate->state() == QProcess::NotRunning)
    {
        animateEllipsisTimer->stop();
        return;
    }

    QString text;
    switch(nextEllipsisCount)
    {
    case 1: text = "Searching.";   nextEllipsisCount = 2; break;
    case 2: text = "Searching..";  nextEllipsisCount = 3; break;
    case 3: text = "Searching..."; nextEllipsisCount = 1; break;
    }
    ui->labelStatus->setText(text);
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
    ui->checkBoxSpaceIsWildcard->setChecked(settings.value("Options/SpaceIsWildcard", ui->checkBoxSpaceIsWildcard->isChecked()).toBool());
    ui->checkBoxSaveWindowPosition->setChecked(settings.value("Options/SaveWindowPosition", ui->checkBoxSaveWindowPosition->isChecked()).toBool());
    if (ui->checkBoxSaveWindowPosition->isChecked())
        restoreGeometry(settings.value("Window/Geometry", saveGeometry()).toByteArray());
}

void MainWindow::saveSettings()
{
    QSettings settings;
    settings.setValue("Options/CaseSensitive", ui->checkBoxCaseSensitive->isChecked());
    settings.setValue("Options/RegExp", ui->checkBoxRegExp->isChecked());
    settings.setValue("Options/SearchOnlyHome", ui->checkBoxSearchOnlyHome->isChecked());
    settings.setValue("Options/ShowFullPath", ui->checkBoxShowFullPath->isChecked());
    settings.setValue("Options/SpaceIsWildcard", ui->checkBoxSpaceIsWildcard->isChecked());
    settings.setValue("Options/SaveWindowPosition", ui->checkBoxSaveWindowPosition->isChecked());
    settings.setValue("Window/Geometry", saveGeometry());
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
