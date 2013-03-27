
set(CMAKE_INCLUDE_CURRENT_DIR ON)


add_executable(testme testme.c)
if(WIN32)
target_link_libraries(testme sdrlib fftw3-3 PortAudio winmm pthread)
else()
target_link_libraries(testme sdrlib fftw3 PortAudio)
endif()

add_executable(sdrcmd sdrcmd.c)
if(WIN32)
target_link_libraries(sdrcmd sdrlib fftw3-3 PortAudio winmm pthread)
else()
target_link_libraries(sdrcmd sdrlib fftw3 PortAudio)
endif()

