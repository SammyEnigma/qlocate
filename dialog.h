#ifndef DIALOG_H
#define DIALOG_H

#include <QSystemTrayIcon>
#include <QDialog>
#include <QModelIndex>

class QProcess;

namespace Ui {
    class Dialog;
}

class Dialog : public QDialog {
    Q_OBJECT
public:
    Dialog(QWidget *parent = 0);
    ~Dialog();

private slots:
    void onFind();
    void onLocateFinished(int);
    void onOpenFile(QModelIndex i);
    void onTrayIconActivated(QSystemTrayIcon::ActivationReason);
    void onLocateReadyReadStdOut();
    void onQuit();

private:
    void changeEvent(QEvent *e);
    void closeEvent(QCloseEvent *event);

private:
    Ui::Dialog *ui;
    QProcess* locate;
    QSystemTrayIcon *trayIcon;
    QString homeDir;
    QString lastPartialLine;
    bool quit;
};

#endif // DIALOG_H
