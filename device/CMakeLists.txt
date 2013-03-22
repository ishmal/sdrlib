
set(CMAKE_INCLUDE_CURRENT_DIR ON)


add_library(device-rtl SHARED device-rtl.c)
target_link_libraries(device-rtl rtlsdr)
