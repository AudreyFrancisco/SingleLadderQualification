#-------------------------------------------------
#
# Project created by QtCreator 2016-11-28T20:17:41
#
#-------------------------------------------------

QT       += widgets core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Alpide-Scope
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    renderarea.cpp \
    unixsocket.cpp

HEADERS  += mainwindow.h \
    renderarea.h \
    unixsocket.h

FORMS    += mainwindow.ui
