
set(CMAKE_INCLUDE_CURRENT_DIR ON)


add_executable(testme testme.c)
if(WIN32)
target_link_libraries(testme sdrlib fftw3-3 PortAudio Opus-0 ogg winmm pthread)
else()
target_link_libraries(testme sdrlib fftw3 PortAudio Opus ogg)
endif()


