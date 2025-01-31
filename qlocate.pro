#-------------------------------------------------
#
# Project created by QtCreator 2011-01-11T23:03:41
#
#-------------------------------------------------

QT       += core gui
CONFIG  += qxt
QXT     += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = qlocate
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    dragawarelistwidget.cpp

HEADERS  += mainwindow.h \
    dragawarelistwidget.h

FORMS    += mainwindow.ui

RESOURCES += \
    res.qrc

OTHER_FILES += \
    edit-find.svg

# Uncomment the following line to compile without Qxt (Global hotkey will not
# be available):
DEFINES += DISABLE_QXT

!contains(DEFINES, DISABLE_QXT) {
    INCLUDEPATH += Qxt
}

