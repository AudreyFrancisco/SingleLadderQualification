find_package(Qt5 COMPONENTS Core Gui Xml Widgets REQUIRED)

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

add_executable(pbGUI ${SRCS})
target_include_directories(pbGUI PRIVATE ..)
target_link_libraries(pbGUI Qt5::Gui Qt5::Core Qt5::Xml Qt5::Widgets mosaic powerboard)

install(TARGETS pbGUI
  RUNTIME DESTINATION bin)
