/********************************************************************************
** Form generated from reading UI file 'dialog.ui'
**
** Created: Sun Oct 24 15:58:26 2010
**      by: Qt User Interface Compiler version 4.6.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DIALOG_H
#define UI_DIALOG_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QCheckBox>
#include <QtGui/QDialog>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QListWidget>
#include <QtGui/QPushButton>
#include <QtGui/QSpacerItem>
#include <QtGui/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_Dialog
{
public:
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout;
    QLabel *label;
    QLineEdit *lineEdit;
    QPushButton *find;
    QListWidget *listWidget;
    QHBoxLayout *horizontalLayout_2;
    QCheckBox *checkBoxCaseSensitive;
    QCheckBox *checkBoxRegExp;
    QCheckBox *checkBoxSearchOnlyHome;
    QSpacerItem *horizontalSpacer;
    QHBoxLayout *horizontalLayout_3;
    QCheckBox *checkBoxCloseAfterLaunch;
    QSpacerItem *horizontalSpacer_2;
    QHBoxLayout *horizontalLayout_4;
    QPushButton *pushButtonOpenFile;
    QPushButton *pushButtonOpenFolder;
    QPushButton *pushButtonUpdateDB;
    QSpacerItem *horizontalSpacer_3;
    QPushButton *pushButton;

    void setupUi(QDialog *Dialog)
    {
        if (Dialog->objectName().isEmpty())
            Dialog->setObjectName(QString::fromUtf8("Dialog"));
        Dialog->resize(504, 303);
        verticalLayout = new QVBoxLayout(Dialog);
        verticalLayout->setSpacing(6);
        verticalLayout->setContentsMargins(11, 11, 11, 11);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setSpacing(6);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        label = new QLabel(Dialog);
        label->setObjectName(QString::fromUtf8("label"));

        horizontalLayout->addWidget(label);

        lineEdit = new QLineEdit(Dialog);
        lineEdit->setObjectName(QString::fromUtf8("lineEdit"));

        horizontalLayout->addWidget(lineEdit);

        find = new QPushButton(Dialog);
        find->setObjectName(QString::fromUtf8("find"));
        find->setEnabled(true);

        horizontalLayout->addWidget(find);


        verticalLayout->addLayout(horizontalLayout);

        listWidget = new QListWidget(Dialog);
        listWidget->setObjectName(QString::fromUtf8("listWidget"));

        verticalLayout->addWidget(listWidget);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setSpacing(6);
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        checkBoxCaseSensitive = new QCheckBox(Dialog);
        checkBoxCaseSensitive->setObjectName(QString::fromUtf8("checkBoxCaseSensitive"));

        horizontalLayout_2->addWidget(checkBoxCaseSensitive);

        checkBoxRegExp = new QCheckBox(Dialog);
        checkBoxRegExp->setObjectName(QString::fromUtf8("checkBoxRegExp"));

        horizontalLayout_2->addWidget(checkBoxRegExp);

        checkBoxSearchOnlyHome = new QCheckBox(Dialog);
        checkBoxSearchOnlyHome->setObjectName(QString::fromUtf8("checkBoxSearchOnlyHome"));
        checkBoxSearchOnlyHome->setChecked(true);

        horizontalLayout_2->addWidget(checkBoxSearchOnlyHome);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_2->addItem(horizontalSpacer);


        verticalLayout->addLayout(horizontalLayout_2);

        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setSpacing(6);
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        checkBoxCloseAfterLaunch = new QCheckBox(Dialog);
        checkBoxCloseAfterLaunch->setObjectName(QString::fromUtf8("checkBoxCloseAfterLaunch"));
        checkBoxCloseAfterLaunch->setChecked(true);

        horizontalLayout_3->addWidget(checkBoxCloseAfterLaunch);

        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_3->addItem(horizontalSpacer_2);


        verticalLayout->addLayout(horizontalLayout_3);

        horizontalLayout_4 = new QHBoxLayout();
        horizontalLayout_4->setSpacing(6);
        horizontalLayout_4->setObjectName(QString::fromUtf8("horizontalLayout_4"));
        pushButtonOpenFile = new QPushButton(Dialog);
        pushButtonOpenFile->setObjectName(QString::fromUtf8("pushButtonOpenFile"));

        horizontalLayout_4->addWidget(pushButtonOpenFile);

        pushButtonOpenFolder = new QPushButton(Dialog);
        pushButtonOpenFolder->setObjectName(QString::fromUtf8("pushButtonOpenFolder"));

        horizontalLayout_4->addWidget(pushButtonOpenFolder);

        pushButtonUpdateDB = new QPushButton(Dialog);
        pushButtonUpdateDB->setObjectName(QString::fromUtf8("pushButtonUpdateDB"));

        horizontalLayout_4->addWidget(pushButtonUpdateDB);

        horizontalSpacer_3 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_4->addItem(horizontalSpacer_3);

        pushButton = new QPushButton(Dialog);
        pushButton->setObjectName(QString::fromUtf8("pushButton"));

        horizontalLayout_4->addWidget(pushButton);


        verticalLayout->addLayout(horizontalLayout_4);


        retranslateUi(Dialog);
        QObject::connect(find, SIGNAL(clicked()), Dialog, SLOT(onFind()));
        QObject::connect(lineEdit, SIGNAL(editingFinished()), find, SLOT(click()));
        QObject::connect(pushButton, SIGNAL(clicked()), Dialog, SLOT(onQuit()));
        QObject::connect(pushButtonOpenFile, SIGNAL(clicked()), Dialog, SLOT(onOpenFile()));
        QObject::connect(listWidget, SIGNAL(doubleClicked(QModelIndex)), pushButtonOpenFile, SLOT(click()));
        QObject::connect(pushButtonOpenFolder, SIGNAL(clicked()), Dialog, SLOT(onOpenFolder()));
        QObject::connect(pushButtonUpdateDB, SIGNAL(clicked()), Dialog, SLOT(onUpdateDB()));

        QMetaObject::connectSlotsByName(Dialog);
    } // setupUi

    void retranslateUi(QDialog *Dialog)
    {
        Dialog->setWindowTitle(QApplication::translate("Dialog", "Search", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("Dialog", "Filter:", 0, QApplication::UnicodeUTF8));
        find->setText(QApplication::translate("Dialog", "Find", 0, QApplication::UnicodeUTF8));
        checkBoxCaseSensitive->setText(QApplication::translate("Dialog", "Case sensitive", 0, QApplication::UnicodeUTF8));
        checkBoxRegExp->setText(QApplication::translate("Dialog", "Use regular expressions", 0, QApplication::UnicodeUTF8));
        checkBoxSearchOnlyHome->setText(QApplication::translate("Dialog", "Search only home", 0, QApplication::UnicodeUTF8));
        checkBoxCloseAfterLaunch->setText(QApplication::translate("Dialog", "Close after launch", 0, QApplication::UnicodeUTF8));
        pushButtonOpenFile->setText(QApplication::translate("Dialog", "Open", 0, QApplication::UnicodeUTF8));
        pushButtonOpenFolder->setText(QApplication::translate("Dialog", "Open Folder", 0, QApplication::UnicodeUTF8));
        pushButtonUpdateDB->setText(QApplication::translate("Dialog", "UpdateDB", 0, QApplication::UnicodeUTF8));
        pushButton->setText(QApplication::translate("Dialog", "Quit", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class Dialog: public Ui_Dialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DIALOG_H
