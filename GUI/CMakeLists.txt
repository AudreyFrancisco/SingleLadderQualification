find_package(Qt5 COMPONENTS Core Gui Widgets REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(SRCS 
  activitystatus.cpp
  calibrationpb.cpp
  checkpbconfig.cpp
  components.cpp
  databasefailure.cpp
  databaseselection.cpp
  dbnotice.cpp
  dialog.cpp
  main.cpp
  mainwindow.cpp
  resultstorage.cpp
  scanconfiguration.cpp
  scanthread.cpp
  testingprogress.cpp
  testselection.cpp
)

add_executable(GUI ${SRCS})
target_link_libraries(GUI Qt5::Gui Qt5::Core Qt5::Widgets alpide_analysis)

install(TARGETS GUI
  RUNTIME DESTINATION bin)
