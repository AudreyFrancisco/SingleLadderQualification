set(SRCS
  scope_control.cpp
  serial/src/serial.cc
  serial/src/impl/unix.cc
  serial/src/impl/list_ports/list_ports_linux.cc
)

add_library(scopecontrol SHARED ${SRCS})
install(TARGETS scopecontrol
  LIBRARY DESTINATION lib)