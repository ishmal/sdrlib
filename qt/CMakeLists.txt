
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Declarative REQUIRED)

#get_cmake_property(_variableNames VARIABLES)
#foreach (_variableName ${_variableNames})
#    message(STATUS "${_variableName}=${${_variableName}}")
#endforeach()

add_executable(simple simple.cpp)
message(STATUS, ${Qt5Widgets_INCLUDE_DIRS})
include_directories(${Qt5Widgets_INCLUDE_DIRS},${Qt5Declarative_INCLUDE_DIRS})
add_definitions(${QT_DEFINITIONS})
set(CMAKE_CXX_FLAGS "${Qt5Widgets_EXECUTABLE_COMPILE_FLAGS}")

if(WIN32)
target_link_libraries(simple ${Qt5Widgets_LIBRARIES} ${Qt5Declarative_LIBRARIES} sdrlib fftw3-3 PortAudio winmm pthread)
else()
target_link_libraries(simple ${Qt5Widgets_LIBRARIES} ${Qt5Declarative_LIBRARIES} sdrlib fftw3 PortAudio)
endif()
