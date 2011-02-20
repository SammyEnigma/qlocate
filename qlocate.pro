#-------------------------------------------------
#
# Project created by QtCreator 2011-01-11T23:03:41
#
#-------------------------------------------------

QT       += core gui

TARGET = qlocate
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    shellexec.cpp \
    dragawarelistwidget.cpp

HEADERS  += mainwindow.h \
    dragawarelistwidget.h

FORMS    += mainwindow.ui

RESOURCES += \
    res.qrc

OTHER_FILES += \
    edit-find.svg
