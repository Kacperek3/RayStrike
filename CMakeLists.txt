cmake_minimum_required(VERSION 3.16)
project(RayStrike)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(SFML 2.6 COMPONENTS graphics window system REQUIRED)

file(GLOB_RECURSE SOURCES src/*.cpp)
add_executable(RayStrike ${SOURCES})

target_include_directories(RayStrike PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_include_directories(RayStrike PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/engine)
target_include_directories(RayStrike PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/network)
target_include_directories(RayStrike PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/states)
target_include_directories(RayStrike PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/utills)

file(COPY assets DESTINATION ${CMAKE_BINARY_DIR})

target_link_libraries(RayStrike PRIVATE sfml-graphics sfml-window sfml-system sfml-audio sfml-network)
