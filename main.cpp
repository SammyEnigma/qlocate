#include <QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setWindowIcon(QIcon(":/images/edit-find.svg"));
    a.setOrganizationName("CodeThesis");
    a.setOrganizationDomain("codethesis.com");
    a.setApplicationName("QLocate");
    MainWindow w;
//    w.show();

    return a.exec();
}
