cmake_minimum_required (VERSION 3.1)
project (new_alpide_software)

find_package(ROOT)

if (${ROOT_cxx17_FOUND})
  MESSAGE("using C++17")
  set (CMAKE_CXX_STANDARD 17)
elseif(${ROOT_cxx14_FOUND})
  MESSAGE("using C++14")
  set (CMAKE_CXX_STANDARD 17)
else()
  MESSAGE("using C++11")
endif()

#SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
#SET(CMAKE_SHARED_LINKER_FLAGS "-Wl,--as-needed")

set(BASE_OBJS 
  DataBaseSrc/DBHelpers.cpp
  src/AlpideConfig.cpp
  src/AlpideDebug.cpp
  src/AlpideDecoder.cpp
  src/BoardDecoder.cpp
  src/Common.cpp
  src/SetupHelpers.cpp
  src/TAlpide.cpp
  src/TApplyMask.cpp
  src/TBoardConfig.cpp
  src/TBoardConfigDAQ.cpp
  src/TBoardConfigMOSAIC.cpp
  src/TBoardConfigRU.cpp
  src/TChipConfig.cpp
  src/TConfig.cpp
  src/TCycleAnalysis.cpp
  src/TDACScan.cpp
  src/TDCTRLAnalysis.cpp
  src/TDCTRLMeasurement.cpp
  src/TDataTaking.cpp
  src/TDigitalAnalysis.cpp
  src/TDigitalScan.cpp
  src/TDigitalWFAnalysis.cpp
  src/TEnduranceCycle.cpp
  src/TEyeMeasurement.cpp
  src/TFastPowerAnalysis.cpp
  src/TFastPowerTest.cpp
  src/TFifoAnalysis.cpp
  src/TFifoTest.cpp
  src/THIC.cpp
  src/THicConfig.cpp
  src/THisto.cpp
  src/TLocalBusAnalysis.cpp
  src/TLocalBusTest.cpp
  src/TNoiseAnalysis.cpp
  src/TNoiseOccupancy.cpp
  src/TPowerAnalysis.cpp
  src/TPowerBoard.cpp
  src/TPowerBoardConfig.cpp
  src/TPowerTest.cpp
  src/TReadoutAnalysis.cpp
  src/TReadoutBoard.cpp
  src/TReadoutBoardDAQ.cpp
  src/TReadoutBoardMOSAIC.cpp
  src/TReadoutBoardRU.cpp
  src/TReadoutTest.cpp
  src/TSCurveScan.cpp
  src/TScan.cpp
  src/TScanAnalysis.cpp
  src/TScanConfig.cpp
  src/TestBeamTools.cpp
  src/USB.cpp
  src/USBHelpers.cpp
)

set(RU_OBJS
  ReadoutUnitSrc/TRuDctrlModule.cpp
  ReadoutUnitSrc/TRuTransceiverModule.cpp
  ReadoutUnitSrc/TRuWishboneModule.cpp
)

set(MOSAIC_OBJS
  MosaicSrc/TAlpideDataParser.cpp
  MosaicSrc/alpidercv.cpp
  MosaicSrc/controlinterface.cpp
  MosaicSrc/pexception.cpp
  MosaicSrc/trgrecorderparser.cpp
)

set(OBJS_ANA
  src/TSCurveAnalysis.cpp
  src/TApplyTuning.cpp
  src/TEyeAnalysis.cpp
)

set(EXE
  startclk
  stopclk
)

set(TEST_EXE
  test_GRST
  test_alucms
  test_chip_count
  test_dacscan
  test_dacscan_voltage
  test_digitalscan
  test_fifo
  test_localbus
  test_mosaic
  test_noiseocc
  test_noiseocc_ext
  test_poweron
  test_pulselength
  test_readoutunit
  test_scope
  test_source
  test_supply_voltage
  test_temperature
  test_threshold
)

set(TEST_EXE_ROOT
  test_ITHRthreshold
  test_VCASNthreshold
  test_roottest
  test_scantest
  test_scantest_digital
  test_threshold_v1
  test_tuneITHR
  test_tuneVCASN
)

execute_process(COMMAND git describe --dirty --always WORKING_DIRECTORY ${CMAKE_SOURCE_DIR} OUTPUT_STRIP_TRAILING_WHITESPACE OUTPUT_VARIABLE GIT_VERSION)
MESSAGE("GIT_VERSION: ${GIT_VERSION}")
add_definitions(-DVERSION="${GIT_VERSION}")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${ROOT_CXX_FLAGS}")

include_directories(. inc DataBaseSrc MosaicSrc MosaicSrc/libpowerboard/include MosaicSrc/libmosaic/include /usr/include/libxml2 ScopeControlSrc ScopeControlSrc/serial/include ${ROOT_INCLUDE_DIRS})

add_library(alpide SHARED ${BASE_OBJS} ${RU_OBJS} ${MOSAIC_OBJS})
target_link_libraries(alpide alucms mosaic powerboard scopecontrol curl xml2 usb-1.0 tinyxml)
list(APPEND ALP_LIBRARIES alpide)

if (${ROOT_FOUND})
  add_library(alpide_analysis SHARED ${OBJS_ANA})
  target_link_libraries(alpide_analysis alucms mosaic powerboard scopecontrol curl xml2 usb-1.0 tinyxml ${ROOT_LIBRARIES})
  list(APPEND ALP_LIBRARIES alpide_analysis)
endif()

foreach(exe ${EXE})
  add_executable(${exe} exe/main_${exe}.cpp)
  target_link_libraries(${exe} alpide alucms mosaic powerboard curl xml2 usb-1.0 tinyxml)
endforeach()

foreach(exe ${TEST_EXE})
  string(REPLACE "test_" "" test ${exe})
  add_executable(${exe} exe/main_${test}.cpp)
  target_link_libraries(${exe} alpide alucms mosaic powerboard curl xml2 usb-1.0 tinyxml)
endforeach()

foreach(exe ${TEST_EXE_ROOT})
  string(REGEX REPLACE "^test_" "" test ${exe})
  add_executable(${exe} exe/main_${test}.cpp)
  target_link_libraries(${exe} alpide alpide_analysis alucms mosaic powerboard curl xml2 usb-1.0 tinyxml ${ROOT_LIBRARIES})
endforeach()

install(TARGETS 
  ${EXE}
  ${TEST_EXE}
  ${TEST_EXE_ROOT}
  ${ALP_LIBRARIES}
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
)

add_custom_target(format-check
  COMMAND ./.format-check.sh
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

subdirs(DataBaseSrc)
subdirs(GUI)
subdirs(MosaicSrc/powerboard)
subdirs(MosaicSrc/libmosaic)
subdirs(MosaicSrc/libpowerboard)
subdirs(ScopeControlSrc)

set(CPACK_PACKAGE_VERSION ${GIT_VERSION})
set(CPACK_GENERATOR "RPM")
set(CPACK_PACKAGE_NAME "alptest")
set(CPACK_PACKAGE_RELEASE 1)
set(CPACK_PACKAGE_CONTACT "someone")
#set(CPACK_PACKAGE_VENDOR "CERN")
set(CPACK_PACKAGING_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CPACK_PACKAGE_RELEASE}.${CMAKE_SYSTEM_PROCESSOR}")
include(CPack)
