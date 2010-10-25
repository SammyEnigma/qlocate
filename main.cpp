#include <QtGui/QApplication>
#include "dialog.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setWindowIcon(QIcon(":/images/edit-find.svg"));
    Dialog w;
//    w.show();
    return a.exec();
}
