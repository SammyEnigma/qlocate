#include "dialog.h"
#include "ui_dialog.h"
#include <QProcess>
#include <QSystemTrayIcon>
#include <QCloseEvent>
#include <QDir>
#include <QMessageBox>
#include <QListWidgetItem>
#include <QTimer>
#include <QMenu>
#include <QFileIconProvider>
#include <QDebug>

Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog)
{
    // initialize misc. variables
    ui->setupUi(this);
    reallyQuit = false;
    locate = NULL;
    originalLabelPalette = ui->labelStatus->palette();
    iconProvider = new QFileIconProvider;

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
    connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(toggleDialogVisible(QSystemTrayIcon::ActivationReason)));
    trayIcon->setIcon(QIcon(":/images/edit-find.svg"));
    trayIcon->setVisible(true);
    QMenu* trayIconContextMenu = new QMenu;
    trayIconContextMenu->addAction("Update Database", this, SLOT(startUpdateDB()));
    trayIconContextMenu->addAction("Quit", this, SLOT(quit()));
    trayIcon->setContextMenu(trayIconContextMenu);

    // initialize list widget context menu
    // the user can right click on a found file to pop up the context menu
    // the context menu can be used to open the selected file (this is "Open File")
    // or to open the folder in which the selected file is (this is "Open Folder")
    ui->listWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->listWidget, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showContextMenu(QPoint)));
    listWidgetContextMenu = new QMenu(this);
    listWidgetContextMenu->addAction("Open File", this, SLOT(openFile()));
    listWidgetContextMenu->addAction("Open Folder", this, SLOT(openFolder()));

    // initialize the checkboxes for various options
    oldCaseSensitive = false;
    oldUseRegExp = false;
    oldSearchOnlyHome = true;
    oldShowFullPath = false;
    connect(ui->checkBoxCaseSensitive, SIGNAL(toggled(bool)), this, SLOT(startLocate()));
    connect(ui->checkBoxRegExp, SIGNAL(toggled(bool)), this, SLOT(startLocate()));
    connect(ui->checkBoxSearchOnlyHome, SIGNAL(toggled(bool)), this, SLOT(startLocate()));
    connect(ui->checkBoxShowFullPath, SIGNAL(toggled(bool)), this, SLOT(startLocate()));
    connect(ui->listWidget, SIGNAL(activated(QModelIndex)), this, SLOT(openFile()));

    locate = new QProcess(this);
    connect(locate, SIGNAL(readyReadStandardOutput()), this, SLOT(readLocateOutput()));
    connect(locate, SIGNAL(finished(int)), this, SLOT(locateFinished(int)));
}

Dialog::~Dialog()
{
    delete iconProvider;
    delete ui;
}

void Dialog::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void Dialog::startLocate()
{
    if (oldSearchString == ui->lineEdit->text() &&
        oldShowFullPath == ui->checkBoxShowFullPath->isChecked() &&
        oldUseRegExp == ui->checkBoxRegExp->isChecked() &&
        oldCaseSensitive  == ui->checkBoxCaseSensitive->isChecked() &&
        oldSearchOnlyHome == ui->checkBoxSearchOnlyHome->isChecked())
    {
        return;
    }

    oldShowFullPath = ui->checkBoxShowFullPath->isChecked();
    oldUseRegExp = ui->checkBoxRegExp->isChecked();
    oldCaseSensitive  = ui->checkBoxCaseSensitive->isChecked();
    oldSearchOnlyHome = ui->checkBoxSearchOnlyHome->isChecked();
    oldSearchString = ui->lineEdit->text();
    if (locate->state() != QProcess::NotRunning)
    {
        locate->terminate();
        locate->waitForFinished();
    }
    ui->listWidget->clear();
    
    ui->labelStatus->setPalette(originalLabelPalette);

    if (ui->lineEdit->text().isEmpty() || ui->lineEdit->text() == tr("<type here>"))
    {
        ui->labelStatus->setText(tr("Ready."));
        return;
    }

    lastPartialLine.clear();
    ui->labelStatus->setText(tr("Searching..."));
    nextEllipsisCount = 1;
    animateEllipsisTimer->start();

    // the arguments to pass to locate
    QStringList args;
    args << "--existing" << "--basename";
    if (!ui->checkBoxCaseSensitive->isChecked())
        args << "--ignore-case";
    if (ui->checkBoxRegExp->isChecked())
        args << "--regexp";
    args << ui->lineEdit->text();
    locate->start("locate", args);
}

void Dialog::toggleDialogVisible(QSystemTrayIcon::ActivationReason reason)
{
    if (QSystemTrayIcon::Trigger == reason)
    {
        if (!isVisible())
        {
            ui->lineEdit->selectAll();
            ui->lineEdit->setFocus();
        }
        setVisible(!isVisible());
    }
}

void Dialog::closeEvent(QCloseEvent *event)
{
    if (locate)
        locate->terminate();
    if (!reallyQuit)
    {
        hide();
        event->ignore();
    }
}

void Dialog::readLocateOutput()
{
    lastPartialLine += QString::fromUtf8(locate->readAllStandardOutput());
    QStringList list = lastPartialLine.split('\n');
    lastPartialLine = list.back();
    list.pop_back();
    if (ui->checkBoxSearchOnlyHome->isChecked())
        list = list.filter(QRegExp(QString("^%1/").arg(QRegExp::escape(QDir::homePath()))));

    foreach (const QString& filename, list)
    {
        QListWidgetItem* item = new QListWidgetItem;
        item->setIcon(iconProvider->icon(QFileInfo(filename)));
        if (ui->checkBoxShowFullPath->isChecked())
        {
            item->setData(Qt::DisplayRole, filename);
        }
        else
        {
            item->setData(Qt::DisplayRole, filename.mid(filename.lastIndexOf('/')+1));
            item->setData(Qt::ToolTipRole, filename);
        }
        ui->listWidget->addItem(item);
    }
}

void Dialog::quit()
{
    reallyQuit = true;
    close();
}

void Dialog::openFile()
{
    int role = ui->checkBoxShowFullPath->isChecked() ? Qt::DisplayRole : Qt::ToolTipRole;
    if (ui->listWidget->currentItem() && ui->listWidget->currentItem()->isSelected())
        QProcess::startDetached("/usr/bin/xdg-open", QStringList(ui->listWidget->currentIndex().data(role).toString()));
}

void Dialog::openFolder()
{
    int role = ui->checkBoxShowFullPath->isChecked() ? Qt::DisplayRole : Qt::ToolTipRole;
    if (ui->listWidget->currentItem() && ui->listWidget->currentItem()->isSelected())
        QProcess::startDetached("/usr/bin/xdg-open", QStringList(ui->listWidget->currentIndex().data(role).toString().remove(QRegExp("/[^/]+$"))));
}

void Dialog::startUpdateDB()
{
    QProcess::startDetached("gksudo", QStringList("updatedb"));
}

void Dialog::showContextMenu(QPoint p)
{
    listWidgetContextMenu->exec(ui->listWidget->mapToGlobal(p));
}

void Dialog::locateFinished(int /*exitCode*/)
{
    animateEllipsisTimer->stop();
    
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

void Dialog::animateEllipsis()
{
    QString text;
    switch(nextEllipsisCount)
    {
    case 1: text = "Searching.";   nextEllipsisCount = 2; break;
    case 2: text = "Searching..";  nextEllipsisCount = 3; break;
    case 3: text = "Searching..."; nextEllipsisCount = 1; break;
    }
    ui->labelStatus->setText(text);
}
