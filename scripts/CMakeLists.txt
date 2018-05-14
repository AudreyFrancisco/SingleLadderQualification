set(SRCS
packaging_script.sh
rate_dependence.sh
renew_kerberos.py
run_backbias_threshold.sh
run_irrad_chip_test.sh
run_pulseshape_scan_vcasn.sh
test_power_up.sh
common/functions.sh
common/hameg2030.py
common/hameg_module_test.py
common/huber.py
common/powerOFF.py
common/powerON.py
common/pulser.py
stave_test/HP_E3631A.py
stave_test/hameg.py
stave_test/huber.py
stave_test/powerOff.sh
stave_test/powerOn.sh
)

if(NOT "${PROJECT_SOURCE_DIR}" STREQUAL "${PROJECT_BINARY_DIR}")
foreach(src ${SRCS})
	add_custom_command(OUTPUT ${src}
		COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/${src} ${CMAKE_CURRENT_BINARY_DIR}/${src}
		DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${src})
endforeach()
add_custom_target(scripts ALL DEPENDS ${SRCS})
endif()

install(FILES ${SRCS}
	PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
	DESTINATION srcs)

