if(NOT Qt5_FOUND)
  return()
endif()

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(SRCS
  main.cpp
  mainwindow.cpp
  ScanData.cpp
)

add_executable(TestDataExplorer ${SRCS})
target_link_libraries(TestDataExplorer Qt5::Gui Qt5::Core Qt5::Widgets alpide)
set_target_properties(TestDataExplorer PROPERTIES AUTORCC TRUE)
install(TARGETS TestDataExplorer RUNTIME DESTINATION bin)
