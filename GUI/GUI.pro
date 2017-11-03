#-------------------------------------------------
#
# Project created by QtCreator 2017-05-03T11:24:41
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = GUI
TEMPLATE = app

CONFIG += c++11 -Wall -pedantic

DEFINES += VERSION="\\\"$(shell git describe --dirty --always)\\\""

QMAKE_CXXFLAGS *= $(shell root-config --cflags)
QMAKE_CFLAGS   *= $(shell root-config --cflags)
QMAKE_LDFLAGS  *= $(shell root-config --ldflags)

SOURCES += main.cpp\
           mainwindow.cpp\
           dialog.cpp \
           testselection.cpp \
           scanthread.cpp \
    scanconfiguration.cpp \
    testingprogress.cpp \
    checkpbconfig.cpp \
    calibrationpb.cpp \
    databaseselection.cpp

HEADERS  += mainwindow.h \
            dialog.h \
            testselection.h \
            scanthread.h \
    scanconfiguration.h \
    testingprogress.h \
    checkpbconfig.h \
    calibrationpb.h \
    databaseselection.h

FORMS    += mainwindow.ui \
            dialog.ui \
            testselection.ui \
    scanconfiguration.ui \
    testingprogress.ui \
    checkpbconfig.ui \
    calibrationpb.ui \
    databaseselection.ui

INCLUDEPATH += $(ROOTSYS)/include /usr/local/include

LIBS += -L/usr/local/lib -lusb-1.0 \
        -L$$PWD/../DataBaseSrc \
        -L$$PWD/../MosaicSrc/libmosaic \
        -L$$PWD/../MosaicSrc/libpowerboard \
        $(shell root-config --glibs)

DISTFILES +=

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../release/ -lalpide -lalpide_analysis
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../debug/ -lalpide -lalpide_analysis
else:unix: LIBS += -L$$PWD/../ -lalpide -lalpide_analysis -lalucms -lcurl -lxml2

INCLUDEPATH += $$PWD/../
DEPENDPATH += $$PWD/../

INCLUDEPATH += $$PWD/../inc
DEPENDPATH += $$PWD/../inc

INCLUDEPATH += $$PWD/../DataBaseSrc/
DEPENDPATH += $$PWD/../DataBaseSrc/

INCLUDEPATH += $$PWD/../MosaicSrc/
DEPENDPATH += $$PWD/../MosaicSrc/

INCLUDEPATH += $$PWD/../MosaicSrc/libmosaic/include
DEPENDPATH += $$PWD/../MosaicSrc/libmosaic/include

INCLUDEPATH += $$PWD/../MosaicSrc/libpowerboard/include
DEPENDPATH += $$PWD/../MosaicSrc/libpowerboard/include

INCLUDEPATH += $$PWD/../ReadoutUnitSrc/
DEPENDPATH += $$PWD/../ReadoutUnitSrc/

INCLUDEPATH += /usr/include/libxml2
DEPENDPATH += /usr/include/libxml2
