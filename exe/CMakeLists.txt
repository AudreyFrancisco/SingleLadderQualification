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

foreach(exe ${EXE})
  add_executable(${exe} main_${exe}.cpp)
  target_link_libraries(${exe} alpide alucms mosaic powerboard curl xml2 usb-1.0 tinyxml)
endforeach()

foreach(exe ${TEST_EXE})
  string(REPLACE "test_" "" test ${exe})
  add_executable(${exe} main_${test}.cpp)
  target_link_libraries(${exe} alpide alucms mosaic powerboard curl xml2 usb-1.0 tinyxml)
endforeach()

if (${ROOT_FOUND})
  foreach(exe ${TEST_EXE_ROOT})
    string(REGEX REPLACE "^test_" "" test ${exe})
    add_executable(${exe} main_${test}.cpp)
    target_link_libraries(${exe} alpide alpide_analysis alucms mosaic powerboard curl xml2 usb-1.0 tinyxml ${ROOT_LIBRARIES})
  endforeach()
endif()

install(TARGETS 
  ${EXE}
  ${TEST_EXE}
  RUNTIME DESTINATION bin
)

if (${ROOT_FOUND})
  install(TARGETS 
    ${TEST_EXE_ROOT}
    RUNTIME DESTINATION bin
  )
endif()
