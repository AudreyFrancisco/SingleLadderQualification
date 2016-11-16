# Created by and for Qt Creator. This file was created for editing the project sources only.
# You may attempt to use it for building too, by modifying this file here.

TARGET = new-alpide-software

TEMPLATE = lib

DEFINES += NEW_ALPIDE_SOFTWARE_LIBRARY

HEADERS = \
   $$PWD/MosaicSrc/alpide3rcv.h \
   $$PWD/MosaicSrc/controlinterface.h \
   $$PWD/MosaicSrc/i2cbus.h \
   $$PWD/MosaicSrc/i2cslave.h \
   $$PWD/MosaicSrc/i2csyspll.h \
   $$PWD/MosaicSrc/ipbus.h \
   $$PWD/MosaicSrc/ipbusudp.h \
   $$PWD/MosaicSrc/mdatagenerator.h \
   $$PWD/MosaicSrc/mdatareceiver.h \
   $$PWD/MosaicSrc/mdatasave.h \
   $$PWD/MosaicSrc/mexception.h \
   $$PWD/MosaicSrc/mruncontrol.h \
   $$PWD/MosaicSrc/mtriggercontrol.h \
   $$PWD/MosaicSrc/mwbbslave.h \
   $$PWD/MosaicSrc/pexception.h \
   $$PWD/MosaicSrc/pulser.h \
   $$PWD/MosaicSrc/wbb.h \
   $$PWD/MosaicSrc/wishbonebus.h \
   $$PWD/AlpideConfig.h \
   $$PWD/AlpideDecoder.h \
   $$PWD/BoardDecoder.h \
   $$PWD/TAlpide.h \
   $$PWD/TBoardConfig.h \
   $$PWD/TBoardConfigDAQ.h \
   $$PWD/TBoardConfigMOSAIC.h \
   $$PWD/TChipConfig.h \
   $$PWD/TConfig.h \
   $$PWD/TReadoutBoard.h \
   $$PWD/TReadoutBoardDAQ.h \
   $$PWD/TReadoutBoardMOSAIC.h \
   $$PWD/USB.h \
   $$PWD/USBHelpers.h

SOURCES = \
   $$PWD/MosaicSrc/alpide3rcv.cpp \
   $$PWD/MosaicSrc/controlinterface.cpp \
   $$PWD/MosaicSrc/i2cbus.cpp \
   $$PWD/MosaicSrc/i2cslave.cpp \
   $$PWD/MosaicSrc/i2csyspll.cpp \
   $$PWD/MosaicSrc/ipbus.cpp \
   $$PWD/MosaicSrc/ipbusudp.cpp \
   $$PWD/MosaicSrc/mdatagenerator.cpp \
   $$PWD/MosaicSrc/mdatareceiver.cpp \
   $$PWD/MosaicSrc/mdatasave.cpp \
   $$PWD/MosaicSrc/mexception.cpp \
   $$PWD/MosaicSrc/mruncontrol.cpp \
   $$PWD/MosaicSrc/mtriggercontrol.cpp \
   $$PWD/MosaicSrc/mwbbslave.cpp \
   $$PWD/MosaicSrc/pexception.cpp \
   $$PWD/MosaicSrc/pulser.cpp \
   $$PWD/AlpideConfig.cpp \
   $$PWD/AlpideDecoder.cpp \
   $$PWD/BoardDecoder.cpp \
   $$PWD/TAlpide.cpp \
   $$PWD/TBoardConfig.cpp \
   $$PWD/TBoardConfigDAQ.cpp \
   $$PWD/TBoardConfigMOSAIC.cpp \
   $$PWD/TChipConfig.cpp \
   $$PWD/TConfig.cpp \
   $$PWD/ThresholdScan.cpp \
   $$PWD/TReadoutBoard.cpp \
   $$PWD/TReadoutBoardDAQ.cpp \
   $$PWD/TReadoutBoardMOSAIC.cpp \
   $$PWD/USB.cpp \
   $$PWD/USBHelpers.cpp

INCLUDEPATH = \
    $$PWD/. \
    $$PWD/MosaicSrc

macx: {
    message("Compiling new-alpide-software for OSX")
    INCLUDEPATH += /usr/local/Cellar/libusb/1.0.20/include/
    LIBS += -L/usr/local/lib -lusb-1.0
}

unix:!macx {
    message("Compiling for unix")
    INCLUDEPATH += $$PWD/../libusb-1.0.9/local/include/
    LIBS += -L$$PWD/../libusb-1.0.9/local/lib -lusb-1.0
}


