if(NOT Qt5_FOUND)
  return()
endif()

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
  debugwindow.cpp
  dialog.cpp
  main.cpp
  mainwindow.cpp
  resultstorage.cpp
  scanthread.cpp
  testingprogress.cpp
  testselection.cpp
)

add_executable(GUI ${SRCS} resources.qrc)
target_link_libraries(GUI Qt5::Gui Qt5::Core Qt5::Widgets alpide)
set_target_properties(GUI PROPERTIES AUTORCC TRUE)
install(TARGETS GUI RUNTIME DESTINATION bin)
