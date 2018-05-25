cmake_minimum_required(VERSION 3.2)
project(alptest)

# user options
option(DISABLE_ROOT "Disable ROOT")
option(DISABLE_QT "Disable QT")

# by default build optimized code with debug symbols
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE RELWITHDEBINFO)
endif()
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

# look for Qt, required unless disabled explicitly
if(DISABLE_QT STREQUAL "OFF")
  message(STATUS "Looking for Qt5")
  find_package(Qt5 REQUIRED COMPONENTS Core Gui Widgets Xml)
  message(STATUS "Using Qt ${Qt5_VERSION}")
elseif()
  message(STATUS "GUI disabled")
endif()

# look for ROOT and warn if unavailable
if(DISABLE_ROOT STREQUAL "OFF")
  message(STATUS "Looking for ROOT")
  # try first with ROOTConfig.cmake,
  # then fallback to FindROOT.cmake (non-cmake installation of ROOT)
  find_package(ROOT CONFIG QUIET)
  if(NOT ROOT_FOUND)
    list(APPEND CMAKE_MODULE_PATH $ENV{ROOTSYS}/etc/cmake)
    find_package(ROOT QUIET REQUIRED)
  endif()
else()
  message(STATUS "ROOT disabled")
endif()
if(ROOT_FOUND)
  message(STATUS "Using ROOT ${ROOT_VERSION}")
else()
  message(WARNING "Compilation without ROOT results in limited functionality!")
endif()

# set C++ standard ensuring compatibility with ROOT
set(CMAKE_CXX_STANDARD 11)
if(ROOT_cxx17_FOUND)
  set(CMAKE_CXX_STANDARD 17)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
elseif(ROOT_cxx14_FOUND)
  set(CMAKE_CXX_STANDARD 14)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
elseif(ROOT_cxx11_FOUND)
  set(CMAKE_CXX_STANDARD 11)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
endif()
message(STATUS "Using C++${CMAKE_CXX_STANDARD}")

# warn about undefined symbols already for libraries
link_libraries("-Wl,--as-needed")
link_libraries("-Wl,--no-undefined")
add_compile_options("-Wall")
add_compile_options("-Werror")
add_compile_options("-Wpedantic")

# check for build in the source tree
if (CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
  message(STATUS "Building in source tree")
  set(BUILDINSOURCE true)
endif()

# add subdirectories for the actual work
add_subdirectory(analysis)
add_subdirectory(DataBaseSrc)
add_subdirectory(exe)
add_subdirectory(ImpedanceTest)
add_subdirectory(libalpide)
add_subdirectory(MosaicSrc/powerboard)
add_subdirectory(MosaicSrc/libmosaic)
add_subdirectory(MosaicSrc/libpowerboard)
add_subdirectory(ScopeControlSrc)
add_subdirectory(scripts)
add_subdirectory(GUI)

# for out-of-source builds create Data/ directory
if(NOT BUILDINSOURCE)
  file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/Data)
endif()

# add target for format checking
add_custom_target(format-check
  COMMAND ./.format-check.sh
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

# install pre-commit hook for format checking
configure_file(.pre-commit-clang-format ${CMAKE_CURRENT_SOURCE_DIR}/.git/hooks/pre-commit COPYONLY)

# setup rpm packaging
set(CPACK_PACKAGE_VERSION ${GIT_VERSION})
set(CPACK_GENERATOR "RPM")
set(CPACK_PACKAGE_NAME "alptest")
set(CPACK_PACKAGE_RELEASE 1)
set(CPACK_PACKAGE_CONTACT "someone")
#set(CPACK_PACKAGE_VENDOR "CERN")
set(CPACK_PACKAGING_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CPACK_PACKAGE_RELEASE}.${CMAKE_SYSTEM_PROCESSOR}")
include(CPack)
