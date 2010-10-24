#ifndef DIALOG_H
#define DIALOG_H

#include <QSystemTrayIcon>
#include <QDialog>

class QProcess;
class QTimer;

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
    void onTrayIconActivated(QSystemTrayIcon::ActivationReason);
    void onLocateReadyReadStdOut();
    void onQuit();
    void onOpenFile();
    void onOpenFolder();

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
    QTimer* timer;
};

#endif // DIALOG_H
