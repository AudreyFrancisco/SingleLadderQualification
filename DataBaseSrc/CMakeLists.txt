set(SRCS
  AlpideDB.cpp
  AlpideDBEndPoints.cpp
  AlpideDBManager.cpp
  CernSsoCookiesJar.cpp
  utilities.cpp
)

add_library(alucms SHARED ${SRCS})
install(TARGETS alucms
  LIBRARY DESTINATION lib)