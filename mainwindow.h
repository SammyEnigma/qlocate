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
    void toggleVisible(QSystemTrayIcon::ActivationReason);
    void readLocateOutput();
    void quit();
    void openFile();
    void showFile();
    void startUpdateDB();
    void showContextMenu(QPoint p);
    void locateFinished();
    void animateEllipsis();
    void toggleFullPaths();

private:
    bool event(QEvent *event);
    void changeEvent(QEvent *e);
    void closeEvent(QCloseEvent *event);
    QString currentFilename();
    void openFile(QString filename);
    void showFile(QString filename);
    void restoreSettings();
    void saveSettings();

private:
    Ui::MainWindow *ui;
    QProcess* locate;
    bool reallyQuit;
    QString oldSearchString;
    bool oldCaseSensitive;
    bool oldUseRegExp;
    bool oldSearchOnlyHome;
    QMenu* listWidgetContextMenu;
    QPalette originalLabelPalette;
    QFileIconProvider* iconProvider;
    QTimer* animateEllipsisTimer;
    int nextEllipsisCount;
    QTimer* readLocateOutputTimer;
    QString homePath;
};

#endif // MAINWINDOW_H
