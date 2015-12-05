set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)

find_package(Boost REQUIRED system)

include_directories(${Boost_INCLUDE_DIRS})
add_executable(whynot main.cpp)
target_link_libraries(whynot ${Boost_LIBRARIES})
target_compile_features(whynot PRIVATE cxx_range_for)
