set(EXE
  startclk
  stopclk
)

set(EXE_ROOT
  plot_eyes
)
if (OLD_CXX)
  list(REMOVE_ITEM EXE_ROOT plot_eyes)
endif()

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
  test_powerdown_analogue
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

# for now put the executables in the build root
# for compatibility with existing scripts
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

foreach(exe ${EXE})
  add_executable(${exe} main_${exe}.cpp)
  target_link_libraries(${exe} alpide)
endforeach()

foreach(exe ${TEST_EXE})
  string(REPLACE "test_" "" test ${exe})
  add_executable(${exe} main_${test}.cpp)
  target_link_libraries(${exe} alpide)
endforeach()

if (${ROOT_FOUND})
  foreach(exe ${EXE_ROOT})
    add_executable(${exe} main_${exe}.cpp)
    target_link_libraries(${exe} alpide)
  endforeach()

  foreach(exe ${TEST_EXE_ROOT})
    string(REGEX REPLACE "^test_" "" test ${exe})
    add_executable(${exe} main_${test}.cpp)
    target_include_directories(${exe} PRIVATE ${ROOT_INCLUDE_DIRS})
    target_link_libraries(${exe} alpide)
  endforeach()
endif()

set(ALL_EXE ${EXE} ${TEST_EXE})
if (${ROOT_FOUND})
  list(APPEND ALL_EXE ${TEST_EXE_ROOT})
endif()

install(TARGETS
  ${ALL_EXE}
  RUNTIME DESTINATION bin
)
