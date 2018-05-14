cmake_minimum_required(VERSION 3.2)
project(new_alpide_software)

find_package(ROOT)

if(NOT ROOT_FOUND)
  message(WARNING "Compilation without ROOT results in limited functionality!")
endif()

if(${ROOT_cxx17_FOUND})
  message(STATUS "using C++17")
  set (CMAKE_CXX_STANDARD 17)
elseif(${ROOT_cxx14_FOUND})
  message(STATUS "using C++14")
  set (CMAKE_CXX_STANDARD 14)
else()
  message(STATUS "using C++11")
  set (CMAKE_CXX_STANDARD 11)
endif()

set(ROOTOPTS ${ROOT_CXX_FLAGS})
separate_arguments(ROOTOPTS)

link_libraries("-Wl,--no-undefined")

add_custom_target(format-check
  COMMAND ./.format-check.sh
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

add_subdirectory(DataBaseSrc)
add_subdirectory(exe)
add_subdirectory(libalpide)
add_subdirectory(MosaicSrc/powerboard)
add_subdirectory(MosaicSrc/libmosaic)
add_subdirectory(MosaicSrc/libpowerboard)
add_subdirectory(ScopeControlSrc)
add_subdirectory(scripts)
add_subdirectory(GUI)

set(CPACK_PACKAGE_VERSION ${GIT_VERSION})
set(CPACK_GENERATOR "RPM")
set(CPACK_PACKAGE_NAME "alptest")
set(CPACK_PACKAGE_RELEASE 1)
set(CPACK_PACKAGE_CONTACT "someone")
#set(CPACK_PACKAGE_VENDOR "CERN")
set(CPACK_PACKAGING_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CPACK_PACKAGE_RELEASE}.${CMAKE_SYSTEM_PROCESSOR}")
include(CPack)
