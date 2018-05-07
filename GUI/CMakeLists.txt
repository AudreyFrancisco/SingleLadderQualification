find_package(Qt5Core)
find_package(Qt5Gui)
find_package(Qt5Widgets)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(SRCS 
main.cpp
mainwindow.cpp
dialog.cpp
testselection.cpp
scanthread.cpp
scanconfiguration.cpp
testingprogress.cpp
checkpbconfig.cpp
calibrationpb.cpp
databaseselection.cpp
resultstorage.cpp
activitystatus.cpp
dbnotice.cpp
components.cpp
databasefailure.cpp
)

execute_process(COMMAND root-config --cflags OUTPUT_VARIABLE ROOTFLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${ROOTFLAGS}")
execute_process(COMMAND root-config --ldflags OUTPUT_VARIABLE ROOTLDFLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
set(CMAKE_LD_FLAGS "${CMAKE_LD_FLAGS} ${ROOTLDFLAGS}")

include_directories(
/usr/local/include
/usr/include/qt5/QtWidgets
..
../inc
../DataBaseSrc
../MosaicSrc
../MosaicSrc/libmosaic/include
../MosaicSrc/libpowerboard/include
../ReadoutUnit
../ScopeControlSrc
../ScopeControlSrc/serial/include
#/usr/include/libxml2
)

add_executable(GUI ${SRCS})
target_link_libraries(GUI Qt5::Gui Qt5::Core Qt5::Widgets alucms curl alpide alpide_analysis mosaic powerboard xml2 usb-1.0 tinyxml)

install(TARGETS GUI
  RUNTIME DESTINATION bin)