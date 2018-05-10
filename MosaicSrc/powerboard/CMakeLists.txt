set(SRCS
  pbif.cpp
  pbctrl.cpp
)

add_executable(pbctrl ${SRCS})
target_link_libraries(pbctrl mosaic powerboard)
install(TARGETS pbctrl
  RUNTIME DESTINATION bin)

subdirs(GUI)
