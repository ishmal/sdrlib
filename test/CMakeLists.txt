
set(CMAKE_INCLUDE_CURRENT_DIR ON)


add_executable(testme testme.c)
target_link_libraries(testme sdrlib fftw3 PortAudio)
