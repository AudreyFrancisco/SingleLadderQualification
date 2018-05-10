find_package(Qt5Core)
find_package(Qt5Gui)
find_package(Qt5Xml)
find_package(Qt5Widgets)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(SRCS
  src/main.cpp
  src/pbMainWindow.cpp
  src/setValidator.cpp
  src/optionsDialog.cpp
  ../pbif.cpp
)

include_directories(
/usr/local/include
/usr/include/qt5/QtXml
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
)

add_executable(pbGUI ${SRCS})
target_link_libraries(pbGUI Qt5::Gui Qt5::Core Qt5::Xml Qt5::Widgets alucms curl alpide alpide_analysis mosaic powerboard xml2 usb-1.0 tinyxml)

install(TARGETS pbGUI
  RUNTIME DESTINATION bin)
