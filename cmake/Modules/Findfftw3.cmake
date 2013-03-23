if(NOT FFTW3_FOUND)
  pkg_check_modules (FFTW3_PKG librtlsdr)
  find_path(FFTW3_INCLUDE_DIR NAMES fftw3.h
    PATHS
    ${FFTW3_PKG_INCLUDE_DIRS}
    /usr/include
    /usr/local/include
  )

  find_library(FFTW3_LIBRARIES NAMES rtlsdr
    PATHS
    ${FFTW3_PKG_LIBRARY_DIRS}
    /usr/lib
    /usr/local/lib
  )

if(FFTW3_INCLUDE_DIR AND FFTW3_LIBRARIES)
  set(FFTW3_FOUND TRUE CACHE INTERNAL "fftw3 found")
  message(STATUS "Found fftw3: ${FFTW3_INCLUDE_DIR}, ${FFTW3_LIBRARIES}")
else(FFTW3_INCLUDE_DIR AND FFTW3_LIBRARIES)
  set(FFTW3_FOUND FALSE CACHE INTERNAL "fftw3 found")
  message(STATUS "fftw3 not found.")
endif(FFTW3_INCLUDE_DIR AND FFTW3_LIBRARIES)

mark_as_advanced(FFTW3_INCLUDE_DIR FFTW3_LIBRARIES)

endif(NOT FFTW3_FOUND)
