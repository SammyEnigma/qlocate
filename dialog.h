#ifndef DIALOG_H
#define DIALOG_H

#include <QSystemTrayIcon>
#include <QDialog>

class QProcess;
class QFileIconProvider;

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
    void toggleDialogVisible(QSystemTrayIcon::ActivationReason);
    void readLocateOutput();
    void quit();
    void openFile();
    void openFolder();
    void startUpdateDB();
    void showContextMenu(QPoint p);
    void locateFinished(int exitCode);

private:
    void changeEvent(QEvent *e);
    void closeEvent(QCloseEvent *event);

private:
    Ui::Dialog *ui;
    QProcess* locate;
    QString lastPartialLine;
    bool reallyQuit;
    QString oldFindString;
    bool oldCaseSensitive;
    bool oldUseRegExp;
    bool oldSearchOnlyHome;
    bool oldShowFullPath;
    QMenu* listWidgetContextMenu;
    QPalette originalLabelPalette;
    QFileIconProvider* iconProvider;
};

#endif // DIALOG_H
