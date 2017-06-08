#-------------------------------------------------
#
# Project created by QtCreator 2017-05-03T11:24:41
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = GUI
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp\
    ../TThresholdScan.cpp\
    ../Common.cpp\
    ../TFifoAnalysis.cpp\
    ../TFifoTest.cpp\
    ../TDigitalAnalysis.cpp\
    ../TDigitalScan.cpp\
    ../TScanConfig.cpp \
    ../TScanAnalysis.cpp \
    ../TScan.cpp \
    ../TReadoutBoardMOSAIC.cpp \
    ../TReadoutBoard.cpp \
    ../THisto.cpp \
    ../TConfig.cpp \
    ../TChipConfig.cpp \
    ../TBoardConfigMOSAIC.cpp \
    ../TBoardConfigDAQ.cpp \
    ../TBoardConfig.cpp \
    ../TAlpide.cpp \
    ../SetupHelpers.cpp \
    ../BoardDecoder.cpp \
    ../AlpideDecoder.cpp \
    ../AlpideConfig.cpp \
    ../MosaicSrc/pulser.cpp \
    ../MosaicSrc/pexception.cpp \
    ../MosaicSrc/mwbbslave.cpp \
    ../MosaicSrc/mtriggercontrol.cpp \
    ../MosaicSrc/mruncontrol.cpp \
    ../MosaicSrc/mexception.cpp \
    ../MosaicSrc/mdatasave.cpp \
    ../MosaicSrc/mdatareceiver.cpp \
    ../MosaicSrc/mdatagenerator.cpp \
    ../MosaicSrc/mboard.cpp \
    ../MosaicSrc/ipbusudp.cpp \
    ../MosaicSrc/ipbus.cpp \
    ../MosaicSrc/i2csyspll.cpp \
    ../MosaicSrc/i2cslave.cpp \
    ../MosaicSrc/i2cbus.cpp \
    ../MosaicSrc/controlinterface.cpp \
    ../MosaicSrc/alpidercv.cpp \
    ../MosaicSrc/TAlpideDataParser.cpp \
    ../TReadoutBoardDAQ.cpp \
    ../USBHelpers.cpp \
    ../USB.cpp \
    ../TBoardConfigRU.cpp \
    ../TReadoutBoardRU.cpp \
    ../ReadoutUnitSrc/TRuDctrlModule.cpp \
    ../ReadoutUnitSrc/TRuTransceiverModule.cpp \
    ../ReadoutUnitSrc/TRuWishboneModule.cpp\
    ../TThresholdAnalysis.cpp \
    dialog.cpp \
    testselection.cpp

HEADERS  += mainwindow.h \
    ../TThresholdScan.h\
    ../TDigitalAnalysis.h\
    ../TFifoAnalysis.h\
    ../TFifoTest.h\
    ../TDigitalScan.h\
    ../TScanConfig.h \
    ../TScanAnalysis.h \
    ../TScan.h \
    ../TReadoutBoardMOSAIC.h \
    ../TReadoutBoardDAQ.h \
    ../TReadoutBoard.h \
    ../THisto.h \
    ../TConfig.h \
    ../TChipConfig.h \
    ../MosaicSrc/wishbonebus.h \
    ../MosaicSrc/wbb.h \
    ../MosaicSrc/pulser.h \
    ../MosaicSrc/pexception.h \
    ../MosaicSrc/mwbbslave.h \
    ../MosaicSrc/mtriggercontrol.h \
    ../MosaicSrc/mruncontrol.h \
    ../MosaicSrc/mexception.h \
    ../MosaicSrc/mdatasave.h \
    ../MosaicSrc/mdatareceiver.h \
    ../MosaicSrc/mdatagenerator.h \
    ../MosaicSrc/mboard.h \
    ../MosaicSrc/ipbusudp.h \
    ../MosaicSrc/ipbus.h \
    ../MosaicSrc/i2csyspll.h \
    ../MosaicSrc/i2cslave.h \
    ../MosaicSrc/i2cbus.h \
    ../MosaicSrc/controlinterface.h \
    ../MosaicSrc/alpidercv.h \
    ../MosaicSrc/TAlpideDataParser.h \
    ../AlpideConfig.h \
    ../AlpideDecoder.h \
    ../BoardDecoder.h \
    ../SetupHelpers.h \
    ../TAlpide.h \
    ../TBoardConfig.h \
    ../TBoardConfigDAQ.h \
    ../TBoardConfigMOSAIC.h \
    ../TBoardConfigRU.h \
    ../TReadoutBoardRU.h \
    ../TThresholdAnalysis.h\
    ../ReadoutUnitSrc/TRuDctrlModule.h \
    ../ReadoutUnitSrc/TRuTransceiverModule.h \
    ../ReadoutUnitSrc/TRuWishboneModule.h \
    ../Common.h\
    ../ReadoutUnitSrc/UsbDev.hpp \
    dialog.h \
    testselection.h

FORMS    += mainwindow.ui \
    dialog.ui \
    testselection.ui

INCLUDEPATH += $(ROOTSYS)/include

LIBS += -L/usr/local/lib -lusb-1.0\
        -L$(ROOTSYS)/lib  -lGui -lGpad -lHist -lGraf -lGraf3d -lTree -lRint -lPostscript -lMatrix -lPhysics -lMathCore -lRIO -lNet -lThread -lCore -pthread -lm -ldl -rdynamic

DISTFILES +=
