if(NOT OPUS_FOUND)
  pkg_check_modules (OPUS_PKG opus)
  find_path(OPUS_INCLUDE_DIR NAMES opus/opus.h
    PATHS
    ${OPUS_PKG_INCLUDE_DIRS}
    /usr/include
    /usr/local/include
  )

  find_library(OPUS_LIBRARIES NAMES opus
    PATHS
    ${OPUS_PKG_LIBRARY_DIRS}
    /usr/lib
    /usr/local/lib
  )

if(OPUS_INCLUDE_DIR AND OPUS_LIBRARIES)
  set(OPUS_FOUND TRUE CACHE INTERNAL "Opus found")
  message(STATUS "Found Opus: ${OPUS_INCLUDE_DIR}, ${OPUS_LIBRARIES}")
else(OPUS_INCLUDE_DIR AND OPUS_LIBRARIES)
  set(OPUS_FOUND FALSE CACHE INTERNAL "Opus found")
  message(STATUS "Opus not found.")
endif(OPUS_INCLUDE_DIR AND OPUS_LIBRARIES)

mark_as_advanced(OPUS_INCLUDE_DIR OPUS_LIBRARIES)

endif(NOT OPUS_FOUND)
