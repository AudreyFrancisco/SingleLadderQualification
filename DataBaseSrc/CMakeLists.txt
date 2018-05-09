set(SRCS
AlpideDB.cpp AlpideDBEndPoints.cpp AlpideDBManager.cpp CernSsoCookiesJar.cpp utilities.cpp DBHelpers.cpp
)

add_library(alucms SHARED ${SRCS})
target_link_libraries(alucms alpide)
install(TARGETS alucms
  LIBRARY DESTINATION lib)