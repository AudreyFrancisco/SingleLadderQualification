cmake_minimum_required (VERSION 3.1)
project (new_alpide_software)

set (CMAKE_CXX_STANDARD 11)
SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")

set(BASE_OBJS 
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

execute_process(COMMAND git describe --dirty --always OUTPUT_VARIABLE GIT_VERSION)
add_definitions(-DVERSION="${GIT_VERSION}")

execute_process(COMMAND root-config --cflags OUTPUT_VARIABLE ROOTFLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${ROOTFLAGS}")
execute_process(COMMAND root-config --libs OUTPUT_VARIABLE ROOTLIBS OUTPUT_STRIP_TRAILING_WHITESPACE)

include_directories(. inc DataBaseSrc MosaicSrc MosaicSrc/libpowerboard/include MosaicSrc/libmosaic/include /usr/include/libxml2 ScopeControlSrc ScopeControlSrc/serial/include)

add_library(alpide SHARED ${BASE_OBJS} ${RU_OBJS} ${MOSAIC_OBJS})
target_link_libraries(alpide alucms curl xml2 mosaic powerboard scopecontrol usb-1.0 tinyxml)

add_library(alpide_analysis SHARED ${OBJS_ANA})
target_link_libraries(alpide_analysis alucms curl xml2 mosaic powerboard usb-1.0 tinyxml ${ROOTLIBS})

foreach(exe ${EXE})
  add_executable(${exe} exe/main_${exe}.cpp)
  target_link_libraries(${exe} alpide alpide_analysis alucms curl xml2 mosaic powerboard)
endforeach()

foreach(exe ${TEST_EXE})
  string(REPLACE "test_" "" test ${exe})
  add_executable(${exe} exe/main_${test}.cpp)
  target_link_libraries(${exe} alpide alpide_analysis alucms curl xml2 mosaic powerboard)
endforeach()

foreach(exe ${TEST_EXE_ROOT})
  string(REGEX REPLACE "^test_" "" test ${exe})
  add_executable(${exe} exe/main_${test}.cpp)
  target_link_libraries(${exe} alpide alpide_analysis alucms curl xml2 mosaic powerboard ${ROOTLIBS})
endforeach()

install(TARGETS 
  ${EXE}
  ${TEST_EXE}
  ${TEST_EXE_ROOT}
  alpide
  alpide_analysis
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
)

subdirs(DataBaseSrc)
subdirs(GUI)
subdirs(MosaicSrc/libmosaic)
subdirs(MosaicSrc/libpowerboard)
subdirs(ScopeControlSrc)
