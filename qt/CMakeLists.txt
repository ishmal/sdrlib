
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
find_package(Qt5Widgets REQUIRED)

#get_cmake_property(_variableNames VARIABLES)
#foreach (_variableName ${_variableNames})
#    message(STATUS "${_variableName}=${${_variableName}}")
#endforeach()

qt5_wrap_ui(ui_simple.h simple.ui)
   
add_executable(simple simple.cpp sdr.cpp ui_simple.h sdr.h freqdial.h waterfall.h)
qt5_use_modules(simple Widgets)

include_directories(${Qt5Widgets_INCLUDE_DIRS})
add_definitions(${QT_DEFINITIONS})
set(CMAKE_CXX_FLAGS "${Qt5Widgets_EXECUTABLE_COMPILE_FLAGS}")

if(WIN32)
target_link_libraries(simple ${Qt5Widgets_LIBRARIES} sdrlib fftw3-3 PortAudio Opus winmm pthread)
else()
target_link_libraries(simple ${Qt5Widgets_LIBRARIES} sdrlib fftw3 PortAudio Opus)
endif()
