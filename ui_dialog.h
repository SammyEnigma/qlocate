/********************************************************************************
** Form generated from reading UI file 'dialog.ui'
**
** Created: Sat Oct 23 11:18:25 2010
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
    QSpacerItem *horizontalSpacer;
    QHBoxLayout *horizontalLayout_3;
    QCheckBox *checkBoxCloseAftrLaunch;
    QCheckBox *checkBoxSearchOnlyHome;
    QSpacerItem *horizontalSpacer_2;
    QHBoxLayout *horizontalLayout_4;
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

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_2->addItem(horizontalSpacer);


        verticalLayout->addLayout(horizontalLayout_2);

        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setSpacing(6);
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        checkBoxCloseAftrLaunch = new QCheckBox(Dialog);
        checkBoxCloseAftrLaunch->setObjectName(QString::fromUtf8("checkBoxCloseAftrLaunch"));
        checkBoxCloseAftrLaunch->setChecked(true);

        horizontalLayout_3->addWidget(checkBoxCloseAftrLaunch);

        checkBoxSearchOnlyHome = new QCheckBox(Dialog);
        checkBoxSearchOnlyHome->setObjectName(QString::fromUtf8("checkBoxSearchOnlyHome"));
        checkBoxSearchOnlyHome->setChecked(true);

        horizontalLayout_3->addWidget(checkBoxSearchOnlyHome);

        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_3->addItem(horizontalSpacer_2);


        verticalLayout->addLayout(horizontalLayout_3);

        horizontalLayout_4 = new QHBoxLayout();
        horizontalLayout_4->setSpacing(6);
        horizontalLayout_4->setObjectName(QString::fromUtf8("horizontalLayout_4"));
        horizontalSpacer_3 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_4->addItem(horizontalSpacer_3);

        pushButton = new QPushButton(Dialog);
        pushButton->setObjectName(QString::fromUtf8("pushButton"));

        horizontalLayout_4->addWidget(pushButton);


        verticalLayout->addLayout(horizontalLayout_4);


        retranslateUi(Dialog);
        QObject::connect(listWidget, SIGNAL(doubleClicked(QModelIndex)), Dialog, SLOT(onOpenFile(QModelIndex)));
        QObject::connect(find, SIGNAL(clicked()), Dialog, SLOT(onFind()));
        QObject::connect(lineEdit, SIGNAL(editingFinished()), find, SLOT(click()));
        QObject::connect(pushButton, SIGNAL(clicked()), Dialog, SLOT(onQuit()));

        QMetaObject::connectSlotsByName(Dialog);
    } // setupUi

    void retranslateUi(QDialog *Dialog)
    {
        Dialog->setWindowTitle(QApplication::translate("Dialog", "Dialog", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("Dialog", "Filter:", 0, QApplication::UnicodeUTF8));
        find->setText(QApplication::translate("Dialog", "Find", 0, QApplication::UnicodeUTF8));
        checkBoxCaseSensitive->setText(QApplication::translate("Dialog", "Case sensitive", 0, QApplication::UnicodeUTF8));
        checkBoxRegExp->setText(QApplication::translate("Dialog", "Use regular expressions", 0, QApplication::UnicodeUTF8));
        checkBoxCloseAftrLaunch->setText(QApplication::translate("Dialog", "Close after launch", 0, QApplication::UnicodeUTF8));
        checkBoxSearchOnlyHome->setText(QApplication::translate("Dialog", "Search only home", 0, QApplication::UnicodeUTF8));
        pushButton->setText(QApplication::translate("Dialog", "Quit", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class Dialog: public Ui_Dialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DIALOG_H
