cmake_minimum_required(VERSION 3.2)


 message(STATUS "Looking for Qt5")
 find_package(Qt5 REQUIRED COMPONENTS Core Gui Widgets Xml)
 message(STATUS "Using Qt ${Qt5_VERSION}")

find_package(ROOT REQUIRED)
message(STATUS "Using ROOT ${ROOT_VERSION}")

if(NOT Qt5_FOUND)
  return()
endif()


set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

#add_directory(/home/alice/alice/new-alpide-software/DataBaseSrc)

set(SRCS

  main.cpp
  mainwindow.cpp
)

add_executable(dbGUI ${SRCS})
#add_library(alucms SHARED ${SRCS})




target_link_libraries(dbGUI curl xml2 Qt5::Gui Qt5::Core Qt5::Widgets alpide ${ROOT_LIBRARIES} pthread)

target_include_directories(dbGUI PUBLIC ${ROOT_INCLUDE_DIRS})

install(TARGETS dbGUI 
  RUNTIME DESTINATION bin)


