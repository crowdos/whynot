include_directories(${CMAKE_SOURCE_DIR}/src/)

add_library(watchdog MODULE watchdog.cpp)
install(TARGETS watchdog DESTINATION /usr/lib/whynot/)
