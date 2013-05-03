########################################################################
# Project setup
########################################################################
cmake_minimum_required(VERSION 2.8)
project(sdrlib)

#select the debug build type by default to get optimization flags
if(NOT CMAKE_BUILD_TYPE)
   set(CMAKE_BUILD_TYPE "Debug")
   message(STATUS "Build type not specified: defaulting to debug.")
endif(NOT CMAKE_BUILD_TYPE)
set(CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE} CACHE STRING "")

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/Modules)


########################################################################
# Compiler specific setup
########################################################################
#set(CMAKE_C_FLAGS CMAKE_C_FLAGS_DEBUG)

########################################################################
# Find build dependencies
########################################################################
if(WIN32)

message(STATUS "Using provided libraries for win32")
include_directories("${PROJECT_SOURCE_DIR}/win32")
link_directories("${PROJECT_SOURCE_DIR}/win32")

else(WIN32)


find_package(PkgConfig)
find_package(fftw3)
find_package(PortAudio)
find_package(Opus)
find_package(LibRtlSdr)
set(THREADS_USE_PTHREADS_WIN32 true)
find_package(Threads)

if(NOT FFTW3_FOUND)
    message(FATAL_ERROR "fftw required to compile SdrLib")
endif(NOT FFTW3_FOUND)
if(NOT LIBRTLSDR_FOUND)
    message(FATAL_ERROR "librtlsdr required to compile SdrLib")
endif(NOT LIBRTLSDR_FOUND)
if(NOT THREADS_FOUND)
    message(FATAL_ERROR "pthreads(-win32) required to compile SdrLib")
endif(NOT THREADS_FOUND)

endif(WIN32)
########################################################################
# Setup the include and linker paths
########################################################################
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
    ${LIBRTLSDR_INCLUDE_DIR}
    ${THREADS_PTHREADS_INCLUDE_DIR}
)

#link_directories(
#    ...
#)

# Set component parameters
set(INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include CACHE INTERNAL "" FORCE)


########################################################################
# Setup the include and linker paths
########################################################################
add_subdirectory(src)
add_subdirectory(device)
add_subdirectory(test)
add_subdirectory(app)
add_subdirectory(qt)
