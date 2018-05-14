set(SRCS
  AlpideDB.cpp
  AlpideDBEndPoints.cpp
  AlpideDBManager.cpp
  CernSsoCookiesJar.cpp
  utilities.cpp
)

add_library(alucms SHARED ${SRCS})
target_include_directories(alucms PUBLIC . /usr/include/libxml2)
target_link_libraries(alucms curl xml2)
install(TARGETS alucms
  LIBRARY DESTINATION lib)
