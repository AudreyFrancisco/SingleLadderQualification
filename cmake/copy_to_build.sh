message(STATUS "Copying to build directory: ${SRC} -> ${DST}")
file(COPY ${SRC}/
  DESTINATION ${DST}
  FILES_MATCHING
  REGEX "${REGEX}"
)
