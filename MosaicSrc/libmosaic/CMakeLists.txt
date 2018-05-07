set(SRCS
src/genconsumer.cpp
src/i2cbus.cpp
src/i2cslave.cpp
src/i2csyspll.cpp
src/ipbus.cpp
src/ipbusudp.cpp
src/mboard.cpp
src/mdatagenerator.cpp
src/mdatareceiver.cpp
src/mdatasave.cpp
src/mexception.cpp
src/mruncontrol.cpp
src/mservice.cpp
src/mtriggercontrol.cpp
src/mwbbslave.cpp
src/pulser.cpp
src/trgrecorder.cpp
src/mcoordinator.cpp
)

add_library(mosaic SHARED ${SRCS})
install(TARGETS mosaic
  LIBRARY DESTINATION lib)