
set(CMAKE_INCLUDE_CURRENT_DIR ON)


add_executable(sdrserver sdrserver.c)
if(WIN32)
target_link_libraries(sdrserver sdrlib fftw3-3 PortAudio Opus-0 ogg  winmm pthread wsock32)
else()
target_link_libraries(sdrserver sdrlib fftw3 PortAudio Opus)
endif()

add_executable(sdrcmd sdrcmd.c)
if(WIN32)
target_link_libraries(sdrcmd sdrlib fftw3-3 PortAudio Opus-0 ogg winmm pthread)
else()
target_link_libraries(sdrcmd sdrlib fftw3 PortAudio Opus)
endif()

