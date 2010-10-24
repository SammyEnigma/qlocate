#include "dialog.h"
#include "ui_dialog.h"
#include <QProcess>
#include <QSystemTrayIcon>
#include <QCloseEvent>
#include <QDir>
#include <QMessageBox>
#include <QListWidgetItem>
#include <QTimer>

Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog)
{
    ui->setupUi(this);
    trayIcon = new QSystemTrayIcon(this);
    connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(onTrayIconActivated(QSystemTrayIcon::ActivationReason)));
    trayIcon->setVisible(true);
    trayIcon->setIcon(QIcon(":/images/edit-find.svg"));
    homeDir = QDir::homePath();
    quit = false;
    timer = new QTimer(this);
    timer->setInterval(1000);
    timer->setSingleShot(true);
    connect(timer, SIGNAL(timeout()), ui->find, SIGNAL(clicked()));
    connect(ui->lineEdit, SIGNAL(textEdited(QString)), timer, SLOT(start()));
    locate = NULL;
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
    if (ui->lineEdit->text().isEmpty())
        return;

    delete locate;
    locate = new QProcess(this);
    connect(locate, SIGNAL(finished(int)), this, SLOT(onLocateFinished(int)));
    connect(locate, SIGNAL(readyReadStandardOutput()), this, SLOT(onLocateReadyReadStdOut()));
    ui->listWidget->clear();

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

void Dialog::onLocateFinished(int nExitCode)
{
    // check exit code for errors
    if (nExitCode == 0)
    {
        onLocateReadyReadStdOut();
    }
    else
    {
        QMessageBox::critical(this, "qlocate - there was an error.", QString("locate finished with exit code %1").arg(nExitCode));
        ui->listWidget->clear();
    }

    locate->deleteLater();
    locate = NULL;
}

void Dialog::onTrayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
    if (QSystemTrayIcon::Trigger == reason)
    {
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
        list = list.filter(QRegExp(QString("^%1").arg(QRegExp::escape(homeDir))));
    ui->listWidget->addItems(list);
}

void Dialog::onQuit()
{
    quit = true;
    close();
}

void Dialog::onOpenFile()
{
    if ( ui->listWidget->currentItem()->isSelected())
    {
         QProcess::startDetached(
                 "/usr/bin/xdg-open",
                 QStringList(ui->listWidget->currentIndex().data().toString())
                 );

         if (ui->checkBoxCloseAfterLaunch->isChecked())
             hide();
    }
}

void Dialog::onOpenFolder()
{
    if ( ui->listWidget->currentItem()->isSelected())
    {
        QProcess::startDetached(
                 "/usr/bin/xdg-open",
                 QStringList(ui->listWidget->currentIndex().data().toString().remove(QRegExp("/[^/]+$")))
                 );

        if (ui->checkBoxCloseAfterLaunch->isChecked())
            hide();
    }
}
