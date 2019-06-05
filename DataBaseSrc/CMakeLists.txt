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
if (USE_PYTHON_SSO)
  target_compile_definitions(alucms PRIVATE USE_PYTHON_SSO)
endif()
install(TARGETS alucms
  LIBRARY DESTINATION lib)
