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
#include <QInputDialog>
#include <QMessageBox>
#include <QDesktopServices>
#include <QUrl>
#include <QxtGlobalShortcut>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    reallyQuit = false;
    locate = NULL;
    originalLabelPalette = ui->labelStatus->palette();
    iconProvider = new QFileIconProvider;
    homePath = QDir::toNativeSeparators(QDir::homePath()) + QDir::separator();
    isSearching = false;

    // Show the app is busy searching, by making an animation of sorts. This is
    // done by showing ellipsis after "Searching ", one at a time "Searching.",
    // then "Searching..", then "Searching...", then "Searching." again, and so
    // on.
    animateEllipsisTimer = new QTimer(this);
    connect(animateEllipsisTimer, SIGNAL(timeout()), this, SLOT(animateEllipsis()));
    animateEllipsisTimer->setInterval(333);

    // Initialize the auto-search timer. There is no search button in our
    // application. Instead, it starts searching automatically some time after
    // the user stops typing.
    QTimer* autoStartSearchTimer = new QTimer(this);
    autoStartSearchTimer->setInterval(500);
    autoStartSearchTimer->setSingleShot(true);
    connect(autoStartSearchTimer, SIGNAL(timeout()), this, SLOT(startSearching()));
    connect(ui->lineEdit, SIGNAL(textEdited(QString)), autoStartSearchTimer, SLOT(start()));

    // Initialize the tray icon. The application resides in the tray and when
    // the user clicks on the tray icon the dialog is shown. This is to speed
    // things up (no process loading and initialization, so the app is more
    // responsive).
    QSystemTrayIcon* trayIcon = new QSystemTrayIcon(this);
    connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(toggleVisible(QSystemTrayIcon::ActivationReason)));
    trayIcon->setIcon(qApp->windowIcon());
    trayIcon->setVisible(true);
    trayIcon->setContextMenu(ui->menuFile);

    // Connect the file menu signals.
    connect(ui->actionUpdate_Database, SIGNAL(triggered()), this, SLOT(startUpdateDB()));
    connect(ui->actionQuit, SIGNAL(triggered()), this, SLOT(quit()));

    // Initialize list widget context menu. The user can right click on a found
    // file to pop up the context menu. The context menu can be used to open
    // the selected file (this is "Open File"), or to open the folder in which
    // the selected file is (this is "Open Folder").
    ui->listWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    listWidgetContextMenu = new QMenu(this);
    listWidgetContextMenu->addAction(tr("Open"), this, SLOT(openFile()));
    listWidgetContextMenu->addAction(style()->standardIcon(QStyle::SP_DirOpenIcon), tr("Show in Folder"), this, SLOT(showFile()));
    connect(ui->listWidget, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showContextMenu(QPoint)));
    connect(ui->listWidget, SIGNAL(activated(QModelIndex)), this, SLOT(openFile()));

    // Handle up and down arrow in line edit and list widget. We want to switch
    // between the line edit and the list widget with the up and down arrows.
    ui->lineEdit->installEventFilter(this);
    ui->listWidget->installEventFilter(this);

    // Initialize the checkboxes for various options.
    connect(ui->checkBoxCaseSensitive, SIGNAL(toggled(bool)), autoStartSearchTimer, SLOT(start()));
    connect(ui->checkBoxRegExp, SIGNAL(toggled(bool)), autoStartSearchTimer, SLOT(start()));
    connect(ui->checkBoxSearchOnlyHome, SIGNAL(toggled(bool)), autoStartSearchTimer, SLOT(start()));
    connect(ui->checkBoxShowFullPath, SIGNAL(toggled(bool)), this, SLOT(toggleFullPaths()));
    connect(ui->checkBoxSmartWildcard, SIGNAL(toggled(bool)), autoStartSearchTimer, SLOT(start()));
    connect(ui->checkBoxMatchWholePath, SIGNAL(toggled(bool)), autoStartSearchTimer, SLOT(start()));

    // Initialize the "locate" process.
    locate = new QProcess(this);
    connect(locate, SIGNAL(readyReadStandardOutput()), this, SLOT(readLocateOutput()));
    connect(locate, SIGNAL(finished(int)), this, SLOT(readLocateOutput()));
    connect(locate, SIGNAL(error(QProcess::ProcessError)), this, SLOT(locateProcessError()));

    // Because updatedb skips encrypted home folders, we have created our
    // own database, and we need to tell locate to use it in addition to the
    // default one by setting the LOCATE_PATH environment variable.
    QProcessEnvironment env = locate->processEnvironment();
    env.insert("LOCATE_PATH", QString("%1/.config/qlocate/mlocate.db").arg(QDir::homePath()));
    locate->setProcessEnvironment(env);

    // Initialize the reading from locate's stdout.
    readLocateOutputTimer = new QTimer(this);
    readLocateOutputTimer->setInterval(0);
    readLocateOutputTimer->setSingleShot(true);
    connect(readLocateOutputTimer, SIGNAL(timeout()), this, SLOT(readLocateOutput()));

    // Place the window at the center of the screen.
    QRect availableGeometry = QDesktopWidget().availableGeometry();
    QRect currentGeometry = frameGeometry();
    setGeometry(availableGeometry.width() / 2 - currentGeometry.width() / 2,
                availableGeometry.height() / 2 - currentGeometry.height() / 2,
                currentGeometry.width(),
                currentGeometry.height());

    // Activate the global shortcut.
    globalHotKey = new QxtGlobalShortcut(this);
    globalHotKey->setEnabled(true);
    connect(globalHotKey, SIGNAL(activated()), this, SLOT(toggleVisible()));
    connect(ui->actionGlobal_Hotkey, SIGNAL(triggered()), this, SLOT(changeGlobalHotkey()));

    // Connect 'restore defaults'.
    connect(ui->actionRestore_Defaults, SIGNAL(triggered()), this, SLOT(resetSettings()));

    resetSettings();
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
    if (isLabelTextRed) {
        ui->labelStatus->setPalette(originalLabelPalette);
        isLabelTextRed = false;
    }
    ui->labelStatus->setText(text);
}

void MainWindow::setRedLabelText(const QString& text)
{
    if (!isLabelTextRed) {
        QPalette palette = originalLabelPalette;
        palette.setColor(ui->labelStatus->foregroundRole(), Qt::red);
        ui->labelStatus->setPalette(palette);
        isLabelTextRed = true;
    }
    ui->labelStatus->setText(text);
}

void MainWindow::startSearching()
{
    // If the query (and checkboxes) are the same, no need to restart the same
    // query.
    QVariantList state;
    state
            << ui->checkBoxCaseSensitive->isChecked()
            << ui->checkBoxRegExp->isChecked()
            << ui->checkBoxSearchOnlyHome->isChecked()
            << ui->checkBoxMatchWholePath->isChecked()
            << ui->checkBoxSmartWildcard->isChecked()
            << ui->lineEdit->text();
    if (state == lastState) {
        return;
    }
    lastState = state;

    // If a previous search is running stop it.
    stopSearching();

    // If there is no query just clear the list and stop any previous searches.
    if (ui->lineEdit->text().isEmpty() || ui->lineEdit->text() == tr("<type here>")) {
        ui->listWidget->clear();
        setLabelText(tr("Ready."));
        return;
    }

    QString query = ui->lineEdit->text();
    if (ui->checkBoxSmartWildcard->isChecked() && !ui->checkBoxRegExp->isChecked()) {
        query.replace(' ', '*');
        if (query[0] != '*' && query[query.size()-1] != '*') {
            query = '*' + query + '*';
        }
    }

    // The arguments to pass to locate.
    QStringList args;
#ifdef Q_OS_WIN32
    args << "-lfd";
    if (ui->checkBoxRegExp->isChecked()) {
        if (ui->checkBoxCaseSensitive->isChecked()) {
            args << "-rc";
        } else {
            args << "-r";
        }
    }
    args << "--";
#else
    args << "--existing";
    if (!ui->checkBoxMatchWholePath->isChecked()) {
        args << "--basename";
    }
    if (!ui->checkBoxCaseSensitive->isChecked()) {
        args << "--ignore-case";
    }
    if (ui->checkBoxRegExp->isChecked()) {
        args << "--regexp";
    }
#endif
    args << query;

    setLabelText(tr("Searching (press Esc to stop)..."));
    nextEllipsisCount = 1;
    animateEllipsisTimer->start();
    isSearching = true;
    setCursor(Qt::BusyCursor);
    isListBoxCleared = false;

    locate->start("locate", args);
}

void MainWindow::stopSearching()
{
    if (!isSearching) {
        return;
    }
    if (locate->state() != QProcess::NotRunning) {
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
    if (QSystemTrayIcon::Trigger == reason) {
        toggleVisible();
    }
}

void MainWindow::toggleVisible()
{
    if (isVisible() && isActiveWindow() && !isMinimized()) {
        close();
    } else {
        show();
        raise();
        activateWindow();
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    // If there is a search going on stop it.
    if (isSearching) {
        stopSearching();
        setLabelText("Stopped.");
    }

    if (!reallyQuit) {
        hide();
        event->ignore();
    }
}

void MainWindow::readLocateOutput()
{
    // Getting the icon for each file seems to be a lengthy operation, taking
    // much longer than adding the item to the list, so getting the icons is
    // made a separate cycle which can be interrupted if there are pending
    // events (like keyboard input and such). That way the app is not
    // unresponsive.
    QVector<QListWidgetItem*> items;
    while (locate->canReadLine() && !qApp->hasPendingEvents()) {
        QString filename = QString::fromUtf8(locate->readLine()).trimmed();

        if (ui->checkBoxSearchOnlyHome->isChecked() && filename.indexOf(homePath) != 0) {
            continue;
        }

        QListWidgetItem* item = new QListWidgetItem;
        item->setIcon(iconProvider->icon(QFileInfo(filename)));
        if (ui->checkBoxShowFullPath->isChecked()) {
            item->setData(Qt::DisplayRole, filename);
        } else {
            item->setData(Qt::DisplayRole, filename.mid(filename.lastIndexOf(QDir::separator())+1));
        }
        item->setData(Qt::ToolTipRole, filename);

        items.push_back(item);
    }

    // Now we add the collected filenames and icons to the list widget.
    // This doesn't seem to take much time.
    if (!isListBoxCleared) {
        ui->listWidget->clear();
        isListBoxCleared = true;
    }
    foreach(QListWidgetItem* item, items) {
        ui->listWidget->addItem(item);
    }

    // If there more lines can be read, we schedule a timer to return to this
    // function after we have processed any pending events.
    if (locate->canReadLine()) {
        readLocateOutputTimer->start();
    } else if (QProcess::NotRunning == locate->state()) {
        // If there no more lines can be read, the locate process
        // might have ended.
        stopSearching();

        switch (ui->listWidget->count()) {
        case 0:
            setRedLabelText(tr("Nothing was found."));
            break;
        case 1:
            setLabelText(tr("1 file was found."));
            break;
        default:
            setLabelText(tr("%1 files were found.").arg(ui->listWidget->count()));
            break;
        }
    }
}

void MainWindow::locateProcessError()
{
    stopSearching();
    setRedLabelText(tr("Subprocess failed: ") + locate->errorString());
}

void MainWindow::quit()
{
    reallyQuit = true;
    close();
}

void MainWindow::openFile()
{
    foreach (QListWidgetItem* ii, ui->listWidget->selectedItems()) {
        QDesktopServices::openUrl(QUrl::fromLocalFile(ii->data(Qt::ToolTipRole).toString()));
    }
}

void MainWindow::showFile()
{
    foreach (QListWidgetItem* ii, ui->listWidget->selectedItems()) {
        showFile(ii->data(Qt::ToolTipRole).toString());
    }
}

void MainWindow::startUpdateDB()
{
#ifdef Q_OS_WIN32
    QProcess::startDetached("updatedb");
#else
    // updatedb skips encrypted home folders (with good reason). This means
    // we need to create a separate mlocate database inside the encrypted home,
    // that's why we have those weird params.
    QDir().mkpath(QString("%1/.config/qlocate/").arg(QDir::homePath()));
    QProcess::startDetached(QString("updatedb -l 0 -o %1/.config/qlocate/mlocate.db -U %1").arg(QDir::homePath()));
#endif
}

void MainWindow::showContextMenu(QPoint p)
{
    if (!ui->listWidget->selectedItems().empty()) {
        listWidgetContextMenu->exec(ui->listWidget->mapToGlobal(p));
    }
}

void MainWindow::animateEllipsis()
{
    QString text;
    switch(nextEllipsisCount) {
    case 1:
        text = tr("Searching (press Esc to stop).");
        nextEllipsisCount = 2;
        break;
    case 2:
        text = tr("Searching (press Esc to stop)..");
        nextEllipsisCount = 3;
        break;
    case 3:
        text = tr("Searching (press Esc to stop)...");
        nextEllipsisCount = 1;
        break;
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

bool MainWindow::eventFilter(QObject *obj, QEvent *ev)
{
    // When user types down inside the line edit, we change the focus to the
    // list widget (and select the first entry). This is done for ergonomic
    // reasons, so user can just click down multiple times to choose a file,
    // instead of having to click tab first. Likewise, if the up-arrow is
    // pressed in the list widget while the current item is the uppermost,
    // focus goes to the line edit.
    if (ev->type() == QEvent::KeyPress) {
        if (obj == ui->lineEdit) {
            if (ui->listWidget->count() > 0) {
                QKeyEvent *keyEvent = static_cast<QKeyEvent*>(ev);
                if (keyEvent->key() == Qt::Key_Down) {
                    // If current row is not set then set it to first row.
                    if (ui->listWidget->currentRow() < 0) {
                        ui->listWidget->setCurrentRow(0);
                    }
                    ui->listWidget->setFocus(Qt::OtherFocusReason);
                    return true;
                }
            }
        } else if (obj == ui->listWidget) {
            QKeyEvent *keyEvent = static_cast<QKeyEvent*>(ev);
            if (keyEvent->key() == Qt::Key_Up && ui->listWidget->currentRow() == 0) {
                ui->lineEdit->setFocus(Qt::OtherFocusReason);
                ui->listWidget->setCurrentRow(-1);
                return true;
            }
        }
    }

    // pass the event on to the parent class
    return QMainWindow::eventFilter(obj, ev);
}

void MainWindow::restoreSettings()
{
    QSettings settings;
    settings.beginGroup("Options");
    if (settings.contains("CaseSensitive")) {
        ui->checkBoxCaseSensitive->setChecked(settings.value("CaseSensitive").toBool());
    }
    if (settings.contains("RegExp")) {
        ui->checkBoxRegExp->setChecked(settings.value("RegExp").toBool());
    }
    if (settings.contains("SearchOnlyHome")) {
        ui->checkBoxSearchOnlyHome->setChecked(settings.value("SearchOnlyHome").toBool());
    }
    if (settings.contains("ShowFullPath")) {
        ui->checkBoxShowFullPath->setChecked(settings.value("ShowFullPath").toBool());
    }
    if (settings.contains("SpaceIsWildcard")) {
        ui->checkBoxSmartWildcard->setChecked(settings.value("SpaceIsWildcard").toBool());
    }
    if (settings.contains("MatchWholePath")) {
        ui->checkBoxMatchWholePath->setChecked(settings.value("MatchWholePath").toBool());
    }
    if (settings.contains("SaveWindowPosition")) {
        ui->checkBoxSaveWindowPosition->setChecked(settings.value("SaveWindowPosition").toBool());
    }
    if (settings.contains("GlobalHotkey")) {
        globalHotKey->setShortcut(QKeySequence::fromString(settings.value("GlobalHotkey").toString()));
    }
    settings.endGroup();
    if (ui->checkBoxSaveWindowPosition->isChecked() && settings.contains("Window/Geometry")) {
        restoreGeometry(settings.value("Window/Geometry", saveGeometry()).toByteArray());
    }
}

void MainWindow::saveSettings()
{
    QSettings settings;
    settings.beginGroup("Options");
    settings.setValue("CaseSensitive", ui->checkBoxCaseSensitive->isChecked());
    settings.setValue("RegExp", ui->checkBoxRegExp->isChecked());
    settings.setValue("SearchOnlyHome", ui->checkBoxSearchOnlyHome->isChecked());
    settings.setValue("ShowFullPath", ui->checkBoxShowFullPath->isChecked());
    settings.setValue("SpaceIsWildcard", ui->checkBoxSmartWildcard->isChecked());
    settings.setValue("MatchWholePath", ui->checkBoxMatchWholePath->isChecked());
    settings.setValue("SaveWindowPosition", ui->checkBoxSaveWindowPosition->isChecked());
    settings.setValue("GlobalHotkey", globalHotKey->shortcut().toString());
    settings.endGroup();
    settings.setValue("Window/Geometry", saveGeometry());
}

void MainWindow::toggleFullPaths()
{
    for (int ii=0; ii<ui->listWidget->count(); ii++) {
        QListWidgetItem* item = ui->listWidget->item(ii);
        QString filename = item->data(Qt::ToolTipRole).toString();
        if (ui->checkBoxShowFullPath->isChecked()) {
            item->setData(Qt::DisplayRole, filename);
        } else {
            item->setData(Qt::DisplayRole, filename.mid(filename.lastIndexOf(QDir::separator())+1));
        }
    }
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (isSearching && event->key() == Qt::Key_Escape) {
        stopSearching();
        setLabelText("Stopped.");
    }

    QMainWindow::keyPressEvent(event);
}

void MainWindow::changeGlobalHotkey()
{
    QString lastGlobalHotKey = globalHotKey->shortcut().toString();
    bool failed = false;
    do {
        bool ok;
        QString text = QInputDialog::getText(this, tr("Change Global Hotkey"),
                                             tr("Global Hotkey:"), QLineEdit::Normal,
                                             lastGlobalHotKey, &ok);
        failed = (ok && !globalHotKey->setShortcut(QKeySequence::fromString(text)));

        // If setting the new hotkey fails, revert to the old hotkey.
        if (failed) {
            QMessageBox::warning(this, "", tr("Could not register global hotkey: '%1'").arg(text));
            globalHotKey->setShortcut(QKeySequence::fromString(lastGlobalHotKey));
        }
    } while (failed);
}

void MainWindow::showFile(QString filename)
{
#ifdef Q_OS_WIN32
    QProcess::startDetached(QString("explorer /select,\"%1\"").arg(filename));
#else
    filename.resize(filename.lastIndexOf(QDir::separator()) + 1);
    QDesktopServices::openUrl(QUrl::fromLocalFile(filename));
#endif
}

void MainWindow::resetSettings()
{
    ui->checkBoxCaseSensitive->setChecked(false);
    ui->checkBoxRegExp->setChecked(false);
#ifdef Q_OS_WIN32
    ui->checkBoxSearchOnlyHome->setChecked(false);
#else
    ui->checkBoxSearchOnlyHome->setChecked(true);
#endif
    ui->checkBoxShowFullPath->setChecked(false);
    ui->checkBoxSmartWildcard->setChecked(true);
    ui->checkBoxMatchWholePath->setChecked(false);
    ui->checkBoxSaveWindowPosition->setChecked(true);
    globalHotKey->setShortcut(QKeySequence::fromString("Meta+G"));
}
