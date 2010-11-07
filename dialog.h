#ifndef DIALOG_H
#define DIALOG_H

#include <QSystemTrayIcon>
#include <QDialog>

class QProcess;
class QFileIconProvider;
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
    void startLocate();
    void toggleDialogVisible(QSystemTrayIcon::ActivationReason);
    void readLocateOutput();
    void quit();
    void openFile();
    void openFolder();
    void startUpdateDB();
    void showContextMenu(QPoint p);
    void locateFinished(int exitCode);
    void animateEllipsis();

private:
    void changeEvent(QEvent *e);
    void closeEvent(QCloseEvent *event);

private:
    Ui::Dialog *ui;
    QProcess* locate;
    QString lastPartialLine;
    bool reallyQuit;
    QString oldSearchString;
    bool oldCaseSensitive;
    bool oldUseRegExp;
    bool oldSearchOnlyHome;
    bool oldShowFullPath;
    QMenu* listWidgetContextMenu;
    QPalette originalLabelPalette;
    QFileIconProvider* iconProvider;
    QTimer* animateEllipsisTimer;
    int nextEllipsisCount;
};

#endif // DIALOG_H
