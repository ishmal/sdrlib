
set(CMAKE_INCLUDE_CURRENT_DIR ON)

file(GLOB sdrlib_SOURCES *.c)

# Create an executable file called helloworld from sources:
add_library(sdrlib STATIC ${sdrlib_SOURCES})
