set(SRCS
 src/powerboard.cpp
 src/ltc2635.cpp
 src/pcf8574.cpp
 src/ad5254.cpp
 src/ad7997.cpp
 src/sc18is602.cpp
 src/max31865.cpp
 src/max5419.cpp
 src/ADC128D818.cpp
)

add_library(powerboard SHARED ${SRCS})
target_include_directories(powerboard PUBLIC include ../libmosaic/include)
target_link_libraries(powerboard mosaic)
install(TARGETS powerboard
  LIBRARY DESTINATION lib)
