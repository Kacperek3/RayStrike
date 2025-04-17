cmake_minimum_required(VERSION 3.16)
project(RayStrike)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(SFML 2.6 COMPONENTS graphics window system REQUIRED)

file(GLOB SOURCES src/*.cpp)
add_executable(RayStrike ${SOURCES})
file(COPY assets DESTINATION ${CMAKE_BINARY_DIR})

target_link_libraries(RayStrike PRIVATE sfml-graphics sfml-window sfml-system sfml-audio sfml-network)
