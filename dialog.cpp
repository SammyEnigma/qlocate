#include "dialog.h"
#include "ui_dialog.h"
#include <QProcess>
#include <QSystemTrayIcon>
#include <QCloseEvent>
#include <QDir>
#include <QMessageBox>

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

    locate = new QProcess(this);
    connect(locate, SIGNAL(finished(int)), this, SLOT(onLocateFinished(int)));
    connect(locate, SIGNAL(readyReadStandardOutput()), this, SLOT(onLocateReadyReadStdOut()));
    ui->listWidget->clear();
    ui->find->setEnabled(false);

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

    ui->find->setEnabled(true);
    locate->deleteLater();
}

void Dialog::onOpenFile(QModelIndex ii)
{
    QProcess::startDetached(
            "/usr/bin/xdg-open",
            QStringList(ii.data().toString())
            );

    if (ui->checkBoxCloseAftrLaunch->isChecked())
        hide();
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
    lastPartialLine += locate->readAllStandardOutput();
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
