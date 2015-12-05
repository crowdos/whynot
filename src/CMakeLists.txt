set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)

find_package(Boost REQUIRED system)

include_directories(${Boost_INCLUDE_DIRS})
add_executable(whynot main.cpp)
target_link_libraries(whynot ${Boost_LIBRARIES} dl pthread)
install(TARGETS whynot DESTINATION /usr/bin)
