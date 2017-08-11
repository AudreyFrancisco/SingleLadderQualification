#
#	Project definition for the powerboard GUI
#

TEMPLATE = app
INCLUDEPATH += ..
# CONFIG += qt warn_on release thread
CONFIG += qt warn_on thread
QMAKE_CXXFLAGS += -std=gnu++11

INCLUDEPATH += .. ../../libpowerboard/include ../../libmosaic/include
LIBS += -L../../libpowerboard -lpowerboard -L../../libmosaic -lmosaic

QMAKE_CFLAGS_WARN_ON += -Wno-unused-parameter
QMAKE_CXXFLAGS_WARN_ON += -Wno-unused-parameter
QT += xml widgets

# Input
HEADERS += 	./src/pbMainWindow.h \
			./src/setValidator.h \
			./src/optionsDialog.h \
			../pbif.h

SOURCES += ./src/main.cpp \
			./src/pbMainWindow.cpp \
			./src/setValidator.cpp \
			./src/optionsDialog.cpp \
			../pbif.cpp


TARGET		= pbGUI



unix {
	MOC_DIR = ./.moc
	OBJECTS_DIR = ./.obj
}
