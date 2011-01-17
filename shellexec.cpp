#include "mainwindow.h"

#ifdef Q_OS_WIN32
#include <windows.h>
#else
#include <QProcess>
#include <QDir>
#endif

void MainWindow::openFile(QString filename)
{
#ifdef Q_OS_WIN32
    ShellExecuteW(winId(), 0, filename.toStdWString().c_str(), 0, 0, SW_SHOW);
#else
    QProcess::startDetached("xdg-open", QStringList(filename));
#endif
}

void MainWindow::showFile(QString filename)
{
#ifdef Q_OS_WIN32
    ShellExecuteW(winId(), 0, L"explorer", QString("/select,\"%1\"").arg(filename).toStdWString().c_str(), 0, SW_SHOW);
#else
    filename.resize(filename.lastIndexOf(QDir::separator()) + 1);
    openFile(filename);
#endif
}
