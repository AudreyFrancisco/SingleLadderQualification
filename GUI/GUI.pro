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
    ../../GUI_lib/new-alpide-software/TThresholdScan.cpp\
 ../../GUI_lib/new-alpide-software/Common.cpp\
    ../../GUI_lib/new-alpide-software/TFifoAnalysis.cpp\
    ../../GUI_lib/new-alpide-software/TFifoTest.cpp\
   ../../GUI_lib/new-alpide-software/TDigitalAnalysis.cpp\
    ../../GUI_lib/new-alpide-software/TDigitalScan.cpp\
    ../../GUI_lib/new-alpide-software/TScanConfig.cpp \
    ../../GUI_lib/new-alpide-software/TScanAnalysis.cpp \
    ../../GUI_lib/new-alpide-software/TScan.cpp \
   ../../GUI_lib/new-alpide-software/TReadoutBoardMOSAIC.cpp \
    ../../GUI_lib/new-alpide-software/TReadoutBoard.cpp \
    ../../GUI_lib/new-alpide-software/THisto.cpp \
    ../../GUI_lib/new-alpide-software/TConfig.cpp \
    ../../GUI_lib/new-alpide-software/TChipConfig.cpp \
    ../../GUI_lib/new-alpide-software/TBoardConfigMOSAIC.cpp \
    ../../GUI_lib/new-alpide-software/TBoardConfigDAQ.cpp \
    ../../GUI_lib/new-alpide-software/TBoardConfig.cpp \
    ../../GUI_lib/new-alpide-software/TAlpide.cpp \
    ../../GUI_lib/new-alpide-software/SetupHelpers.cpp \
    ../../GUI_lib/new-alpide-software/BoardDecoder.cpp \
    ../../GUI_lib/new-alpide-software/AlpideDecoder.cpp \
    ../../GUI_lib/new-alpide-software/AlpideConfig.cpp \
    ../../GUI_lib/new-alpide-software/MosaicSrc/pulser.cpp \
   ../../GUI_lib/new-alpide-software/MosaicSrc/pexception.cpp \
    ../../GUI_lib/new-alpide-software/MosaicSrc/mwbbslave.cpp \
    ../../GUI_lib/new-alpide-software/MosaicSrc/mtriggercontrol.cpp \
   ../../GUI_lib/new-alpide-software/MosaicSrc/mruncontrol.cpp \
    ../../GUI_lib/new-alpide-software/MosaicSrc/mexception.cpp \
  ../../GUI_lib/new-alpide-software/MosaicSrc/mdatasave.cpp \
     ../../GUI_lib/new-alpide-software/MosaicSrc/mdatareceiver.cpp \
    ../../GUI_lib/new-alpide-software/MosaicSrc/mdatagenerator.cpp \
   ../../GUI_lib/new-alpide-software/MosaicSrc/mboard.cpp \
    ../../GUI_lib/new-alpide-software/MosaicSrc/ipbusudp.cpp \
   ../../GUI_lib/new-alpide-software/MosaicSrc/ipbus.cpp \
    ../../GUI_lib/new-alpide-software/MosaicSrc/i2csyspll.cpp \
  ../../GUI_lib/new-alpide-software/MosaicSrc/i2cslave.cpp \
    ../../GUI_lib/new-alpide-software/MosaicSrc/i2cbus.cpp \
    ../../GUI_lib/new-alpide-software/MosaicSrc/controlinterface.cpp \
    ../../GUI_lib/new-alpide-software/MosaicSrc/alpidercv.cpp \
   ../../GUI_lib/new-alpide-software/MosaicSrc/TAlpideDataParser.cpp \
    ../../GUI_lib/new-alpide-software/TReadoutBoardDAQ.cpp \
    ../../GUI_lib/new-alpide-software/USBHelpers.cpp \
    ../../GUI_lib/new-alpide-software/USB.cpp \
    ../../GUI_lib/new-alpide-software/TBoardConfigRU.cpp \
    ../../GUI_lib/new-alpide-software/TReadoutBoardRU.cpp \
   ../../GUI_lib/new-alpide-software/ReadoutUnitSrc/TRuDctrlModule.cpp \
    ../../GUI_lib/new-alpide-software/ReadoutUnitSrc/TRuTransceiverModule.cpp \
   ../../GUI_lib/new-alpide-software/ReadoutUnitSrc/TRuWishboneModule.cpp\
   ../../GUI_lib/new-alpide-software/TThresholdAnalysis.cpp \
    dialog.cpp \
    testselection.cpp

HEADERS  += mainwindow.h \
   ../../GUI_lib/new-alpide-software/TThresholdScan.h\
    ../../GUI_lib/new-alpide-software/TDigitalAnalysis.h\
    ../../GUI_lib/new-alpide-software/TFifoAnalysis.h\
    ../../GUI_lib/new-alpide-software/TFifoTest.h\
     ../../GUI_lib/new-alpide-software/TDigitalScan.h\
   ../../GUI_lib/new-alpide-software/TScanConfig.h \
    ../../GUI_lib/new-alpide-software/TScanAnalysis.h \
    ../../GUI_lib/new-alpide-software/TScan.h \
    ../../GUI_lib/new-alpide-software/TReadoutBoardMOSAIC.h \
    ../../GUI_lib/new-alpide-software/TReadoutBoardDAQ.h \
    ../../GUI_lib/new-alpide-software/TReadoutBoard.h \
   ../../GUI_lib/new-alpide-software/THisto.h \
    ../../GUI_lib/new-alpide-software/TConfig.h \
    ../../GUI_lib/new-alpide-software/TChipConfig.h \
   ../../GUI_lib/new-alpide-software/MosaicSrc/wishbonebus.h \
    ../../GUI_lib/new-alpide-software/MosaicSrc/wbb.h \
    ../../GUI_lib/new-alpide-software/MosaicSrc/pulser.h \
    ../../GUI_lib/new-alpide-software/MosaicSrc/pexception.h \
   ../../GUI_lib/new-alpide-software/MosaicSrc/mwbbslave.h \
    ../../GUI_lib/new-alpide-software/MosaicSrc/mtriggercontrol.h \
    ../../GUI_lib/new-alpide-software/MosaicSrc/mruncontrol.h \
    ../../GUI_lib/new-alpide-software/MosaicSrc/mexception.h \
    ../../GUI_lib/new-alpide-software/MosaicSrc/mdatasave.h \
    ../../GUI_lib/new-alpide-software/MosaicSrc/mdatareceiver.h \
    ../../GUI_lib/new-alpide-software/MosaicSrc/mdatagenerator.h \
    ../../GUI_lib/new-alpide-software/MosaicSrc/mboard.h \
    ../../GUI_lib/new-alpide-software/MosaicSrc/ipbusudp.h \
    ../../GUI_lib/new-alpide-software/MosaicSrc/ipbus.h \
    ../../GUI_lib/new-alpide-software/MosaicSrc/i2csyspll.h \
    ../../GUI_lib/new-alpide-software/MosaicSrc/i2cslave.h \
    ../../GUI_lib/new-alpide-software/MosaicSrc/i2cbus.h \
    ../../GUI_lib/new-alpide-software/MosaicSrc/controlinterface.h \
    ../../GUI_lib/new-alpide-software/MosaicSrc/alpidercv.h \
    ../../GUI_lib/new-alpide-software/MosaicSrc/TAlpideDataParser.h \
    ../../GUI_lib/new-alpide-software/AlpideConfig.h \
    ../../GUI_lib/new-alpide-software/AlpideDecoder.h \
    ../../GUI_lib/new-alpide-software/BoardDecoder.h \
   ../../GUI_lib/new-alpide-software/SetupHelpers.h \
    ../../GUI_lib/new-alpide-software/TAlpide.h \
    ../../GUI_lib/new-alpide-software/TBoardConfig.h \
    ../../GUI_lib/new-alpide-software/TBoardConfigDAQ.h \
    ../../GUI_lib/new-alpide-software/TBoardConfigMOSAIC.h \
    ../../GUI_lib/new-alpide-software/TBoardConfigRU.h \
    ../../GUI_lib/new-alpide-software/TReadoutBoardRU.h \
    ../../GUI_lib/new-alpide-software/TThresholdAnalysis.h\
    ../../GUI_lib/new-alpide-software/ReadoutUnitSrc/TRuDctrlModule.h \
    ../../GUI_lib/new-alpide-software/ReadoutUnitSrc/TRuTransceiverModule.h \
    ../../GUI_lib/new-alpide-software/ReadoutUnitSrc/TRuWishboneModule.h \
 ../../GUI_lib/new-alpide-software/Common.h\
    ../../GUI_lib/new-alpide-software/ReadoutUnitSrc/UsbDev.hpp \
    dialog.h \
    testselection.h

FORMS    += mainwindow.ui \
    dialog.ui \
    testselection.ui

INCLUDEPATH += $(ROOTSYS)/include

LIBS += -L/usr/local/lib -lusb-1.0\
        -L$(ROOTSYS)/lib  -lGui -lGpad -lHist -lGraf -lGraf3d -lTree -lRint -lPostscript -lMatrix -lPhysics -lMathCore -lRIO -lNet -lThread -lCore -pthread -lm -ldl -rdynamic

DISTFILES +=



