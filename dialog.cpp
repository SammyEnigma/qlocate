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

Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog)
{
    ui->setupUi(this);
    quit = false;
    QTimer* timer = new QTimer(this);
    timer->setInterval(500);
    timer->setSingleShot(true);
    connect(timer, SIGNAL(timeout()), this, SLOT(onFind()));
    locate = NULL;

    connect(ui->lineEdit, SIGNAL(textEdited(QString)), timer, SLOT(start()));
    connect(ui->lineEdit, SIGNAL(textEdited(QString)), this, SLOT(undoRedSearchbox()));

    // setup the tray icon
    QSystemTrayIcon* trayIcon = new QSystemTrayIcon(this);
    connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(onTrayIconActivated(QSystemTrayIcon::ActivationReason)));
    trayIcon->setVisible(true);
    trayIcon->setIcon(QIcon(":/images/edit-find.svg"));
    QMenu* trayIconContextMenu = new QMenu;
    trayIconContextMenu->addAction("Update Database", this, SLOT(onUpdateDB()));
    trayIconContextMenu->addAction("Quit", this, SLOT(onQuit()));
    trayIcon->setContextMenu(trayIconContextMenu);

    oldCaseSensitive = false;
    oldUseRegExp = false;
    oldSearchOnlyHome = true;
    oldShowFullPath = false;
    searchBoxIsRed = false;

    // set widget context menu
    ui->listWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->listWidget, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(onContextMenu(QPoint)));
    listWidgetContextMenu = new QMenu(this);
    listWidgetContextMenu->addAction("Open File", this, SLOT(onOpenFile()));
    listWidgetContextMenu->addAction("Open Folder", this, SLOT(onOpenFolder()));
}

Dialog::~Dialog()
{
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

void Dialog::onFind()
{
    if (oldFindString == ui->lineEdit->text() &&
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
    oldFindString = ui->lineEdit->text();
    if (locate)
    {
        locate->terminate();
        locate->waitForFinished();
        delete locate;
        locate = NULL;
    }
    ui->listWidget->clear();

    if (ui->lineEdit->text().isEmpty())
        return;

    locate = new QProcess(this);
    connect(locate, SIGNAL(readyReadStandardOutput()), this, SLOT(onLocateReadyReadStdOut()));
    connect(locate, SIGNAL(finished(int)), this, SLOT(locateFinished(int)));

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

void Dialog::onTrayIconActivated(QSystemTrayIcon::ActivationReason reason)
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
    if (!quit)
    {
        hide();
        event->ignore();
    }
}

void Dialog::onLocateReadyReadStdOut()
{
    lastPartialLine += QString::fromUtf8(locate->readAllStandardOutput());
    QStringList list = lastPartialLine.split('\n');
    lastPartialLine = list.back();
    list.pop_back();
    if (ui->checkBoxSearchOnlyHome->isChecked())
        list = list.filter(QRegExp(QString("^%1/").arg(QRegExp::escape(QDir::homePath()))));
    QFileIconProvider provider;
    for (QStringList::const_iterator ii = list.begin(); ii != list.end(); ii++)
    {
        const QString& filename = *ii;
        QListWidgetItem* item = new QListWidgetItem;
        item->setIcon(provider.icon(QFileInfo(filename)));
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

void Dialog::onQuit()
{
    quit = true;
    close();
}

void Dialog::onOpenFile()
{
    int role = ui->checkBoxShowFullPath->isChecked() ? Qt::DisplayRole : Qt::ToolTipRole;
    if (ui->listWidget->currentItem() && ui->listWidget->currentItem()->isSelected())
        QProcess::startDetached("/usr/bin/xdg-open", QStringList(ui->listWidget->currentIndex().data(role).toString()));
}

void Dialog::onOpenFolder()
{
    int role = ui->checkBoxShowFullPath->isChecked() ? Qt::DisplayRole : Qt::ToolTipRole;
    if (ui->listWidget->currentItem() && ui->listWidget->currentItem()->isSelected())
        QProcess::startDetached("/usr/bin/xdg-open", QStringList(ui->listWidget->currentIndex().data(role).toString().remove(QRegExp("/[^/]+$"))));
}

void Dialog::onUpdateDB()
{
    QProcess::startDetached("gksudo", QStringList("updatedb"));
}

void Dialog::onContextMenu(QPoint p)
{
    listWidgetContextMenu->exec(ui->listWidget->mapToGlobal(p));
}

void Dialog::locateFinished(int /*exitCode*/)
{
    if (ui->listWidget->count() == 0 && ui->lineEdit->text() == oldFindString)
    {
        QPalette palette = ui->lineEdit->palette();
        palette.setColor(QPalette::Text, Qt::red);
        ui->lineEdit->setPalette(palette);
        searchBoxIsRed = true;
    }
}

void Dialog::undoRedSearchbox()
{
    if (searchBoxIsRed)
    {
        ui->lineEdit->setPalette(QPalette());
        searchBoxIsRed = false;
    }
}
