#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QSystemTrayIcon>
#include <QMainWindow>
#include <QVariantList>

namespace Ui {
    class MainWindow;
}

class QProcess;
class QFileIconProvider;
#ifndef DISABLE_QXT
class QxtGlobalShortcut;
#endif

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    /// Start a search.
    void startSearching();

    /// Stop a search if one is ongoing.
    void stopSearching();

    /// Used by the system tray.
    void toggleVisible(QSystemTrayIcon::ActivationReason);

    /// Hides the window if focused, or shows it if hidden. If it was visible
    /// but not focused, it "jumps" to the foreground. This is the most expected
    /// behavior from similar apps.
    void toggleVisible();

    /// Read new lines from locate's stdout if available.
    void readLocateOutput();

    /// Show an error message in the status bar if locate process fails.
    void locateProcessError();

    /// Quit the application. Note that closing the window closes it to the tray.
    void quit();

    /// Open a file from the list widget (same as if you double-clicked it in
    /// Explorer/a file manager).
    void openFile();

    /// Show a file in its current folder.
    void showFile();

    /// Start the updatedb tool. Might use gksu on Ubuntu.
    void startUpdateDB();

    /// Show the context menu (on right click in the list widget)
    void showContextMenu(QPoint p);

    /// Animate the ellipsis that acts as our "Searching..." animation.
    void animateEllipsis();

    /// When full paths is on, the full paths are shown in the list widget - i.e.
    /// /path/to/file.txt instead of file.txt.
    void toggleFullPaths();

#ifndef DISABLE_QXT
    /// Shows a dialog to change the global hotkey for showing the qlocate window.
    void changeGlobalHotkey();
#endif

    /// Restores the settings to their defaults.
    void restoreDefaults();

private:
    bool event(QEvent *event);
    bool eventFilter(QObject *obj, QEvent *ev);
    void changeEvent(QEvent *e);
    void closeEvent(QCloseEvent *event);
    QString currentFilename();
    void showFile(QString filename);
    void restoreSettings();
    void saveSettings();
    void setLabelText(const QString& text);
    void setRedLabelText(const QString& text);
    virtual void keyPressEvent(QKeyEvent *event);

private:
    Ui::MainWindow *ui;
    QProcess* locate;
    bool reallyQuit;
    QVariantList lastState;
    QMenu* listWidgetContextMenu;
    QPalette originalLabelPalette;
    QFileIconProvider* iconProvider;
    QTimer* animateEllipsisTimer;
    int nextEllipsisCount;
    QTimer* readLocateOutputTimer;
    QString homePath;
    bool isSearching;
    bool isLabelTextRed;
    bool isListBoxCleared;
#ifndef DISABLE_QXT
    QxtGlobalShortcut* globalHotKey;
#endif
};

#endif // MAINWINDOW_H
