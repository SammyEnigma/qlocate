#ifndef DIALOG_H
#define DIALOG_H

#include <QSystemTrayIcon>
#include <QDialog>

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
    void onTrayIconActivated(QSystemTrayIcon::ActivationReason);
    void onLocateReadyReadStdOut();
    void onQuit();
    void onOpenFile();
    void onOpenFolder();
    void onUpdateDB();
    void onContextMenu(QPoint p);

private:
    void changeEvent(QEvent *e);
    void closeEvent(QCloseEvent *event);

private:
    Ui::Dialog *ui;
    QProcess* locate;
    QString lastPartialLine;
    bool quit;
    QString oldFindString;
    bool oldCaseSensitive;
    bool oldUseRegExp;
    bool oldSearchOnlyHome;
    QMenu* listWidgetContextMenu;
};

#endif // DIALOG_H
