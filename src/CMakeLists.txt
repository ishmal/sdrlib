add_definitions(-std=c99)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

file(GLOB sdrlib_SOURCES *.c)

if(WIN32)
endif()

# All files in this directory are included in the lib
add_library(sdrlib STATIC ${sdrlib_SOURCES})

