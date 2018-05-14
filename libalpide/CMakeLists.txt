set(BASE_OBJS 
  ../DataBaseSrc/DBHelpers.cpp
  ../src/AlpideConfig.cpp
  ../src/AlpideDebug.cpp
  ../src/AlpideDecoder.cpp
  ../src/BoardDecoder.cpp
  ../src/Common.cpp
  ../src/SetupHelpers.cpp
  ../src/TAlpide.cpp
  ../src/TApplyMask.cpp
  ../src/TBoardConfig.cpp
  ../src/TBoardConfigDAQ.cpp
  ../src/TBoardConfigMOSAIC.cpp
  ../src/TBoardConfigRU.cpp
  ../src/TChipConfig.cpp
  ../src/TConfig.cpp
  ../src/TCycleAnalysis.cpp
  ../src/TDACScan.cpp
  ../src/TDCTRLAnalysis.cpp
  ../src/TDCTRLMeasurement.cpp
  ../src/TDataTaking.cpp
  ../src/TDigitalAnalysis.cpp
  ../src/TDigitalScan.cpp
  ../src/TDigitalWFAnalysis.cpp
  ../src/TEnduranceCycle.cpp
  ../src/TEyeMeasurement.cpp
  ../src/TFastPowerAnalysis.cpp
  ../src/TFastPowerTest.cpp
  ../src/TFifoAnalysis.cpp
  ../src/TFifoTest.cpp
  ../src/THIC.cpp
  ../src/THicConfig.cpp
  ../src/THisto.cpp
  ../src/TLocalBusAnalysis.cpp
  ../src/TLocalBusTest.cpp
  ../src/TNoiseAnalysis.cpp
  ../src/TNoiseOccupancy.cpp
  ../src/TPowerAnalysis.cpp
  ../src/TPowerBoard.cpp
  ../src/TPowerBoardConfig.cpp
  ../src/TPowerTest.cpp
  ../src/TReadoutAnalysis.cpp
  ../src/TReadoutBoard.cpp
  ../src/TReadoutBoardDAQ.cpp
  ../src/TReadoutBoardMOSAIC.cpp
  ../src/TReadoutBoardRU.cpp
  ../src/TReadoutTest.cpp
  ../src/TSCurveScan.cpp
  ../src/TScan.cpp
  ../src/TScanAnalysis.cpp
  ../src/TScanConfig.cpp
  ../src/TestBeamTools.cpp
  ../src/USB.cpp
  ../src/USBHelpers.cpp
)

set(RU_OBJS
  ../ReadoutUnitSrc/TRuDctrlModule.cpp
  ../ReadoutUnitSrc/TRuTransceiverModule.cpp
  ../ReadoutUnitSrc/TRuWishboneModule.cpp
)

set(MOSAIC_OBJS
  ../MosaicSrc/TAlpideDataParser.cpp
  ../MosaicSrc/alpidercv.cpp
  ../MosaicSrc/controlinterface.cpp
  ../MosaicSrc/pexception.cpp
  ../MosaicSrc/trgrecorderparser.cpp
)

set(OBJS_ANA
  ../src/TSCurveAnalysis.cpp
  ../src/TApplyTuning.cpp
  ../src/TEyeAnalysis.cpp
)

add_library(alpide SHARED ${BASE_OBJS} ${RU_OBJS} ${MOSAIC_OBJS})
target_include_directories(alpide
	PUBLIC ../inc ../MosaicSrc .. # .. is for ReadoutUnitSrc
)
target_link_libraries(alpide alucms mosaic powerboard scopecontrol curl usb-1.0 tinyxml)
list(APPEND ALP_LIBRARIES alpide)

if (${ROOT_FOUND})
  add_library(alpide_analysis SHARED ${OBJS_ANA})
  target_compile_options(alpide_analysis PUBLIC ${ROOTOPTS})
  target_include_directories(alpide_analysis PUBLIC ${ROOT_INCLUDE_DIRS})
  target_link_libraries(alpide_analysis alpide ${ROOT_LIBRARIES} pthread)
  list(APPEND ALP_LIBRARIES alpide_analysis)
endif()

install(TARGETS
  ${ALP_LIBRARIES}
  LIBRARY DESTINATION lib
)

