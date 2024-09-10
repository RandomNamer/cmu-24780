cmake_minimum_required(VERSION 3.20)
project(homeworks)

set(CMAKE_CXX_STANDARD 20)
SET(CMAKE_C_FLAGS_DEBUG "-D_DEBUG")

add_executable(homeworks main.cpp)
add_executable(ps1 ps1/ps1-3.cpp)

add_subdirectory(ps2)

