#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QSystemTrayIcon>
#include <QMainWindow>

namespace Ui {
    class MainWindow;
}

class QProcess;
class QFileIconProvider;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void startLocate();
    void toggleDialogVisible(QSystemTrayIcon::ActivationReason);
    void readLocateOutput();
    void quit();
    void openFile();
    void openFolder();
    void startUpdateDB();
    void showContextMenu(QPoint p);
    void locateFinished();
    void animateEllipsis();
    void reject();

private:
    void changeEvent(QEvent *e);
    void closeEvent(QCloseEvent *event);
    QString currentFilename();

private:
    Ui::MainWindow *ui;
    QProcess* locate;
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
    QTimer* readLocateOutputTimer;
    QString homePath;
};

#endif // MAINWINDOW_H
