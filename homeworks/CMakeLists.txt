cmake_minimum_required(VERSION 3.20)
project(homeworks)

set(CMAKE_CXX_STANDARD 20)
SET(CMAKE_C_FLAGS_DEBUG "-D_DEBUG")

# OGL setup
find_package(OpenGL REQUIRED)
find_library(OGL_LIBRARY OpenGL)
# macOS support for Cocoa
if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    enable_language(OBJC OBJCXX)
    find_library(COCOA_LIBRARY Cocoa)
endif()

# FSSimple setup
set(FSSIMPLE_HEADER)
set(FSSIMPLE_SOURCES)
set(FSSIMPLE_DIR)

if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    set(FSSIMPLE_DIR "fssimple/windows")
    file(GLOB FSSIMPLE_HEADER ${FSSIMPLE_DIR}/*.h)
    file(GLOB FSSIMPLE_SOURCES ${FSSIMPLE_DIR}/*.cpp)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(FSSIMPLE_DIR "fssimple/linux")
    file(GLOB FSSIMPLE_HEADER ${FSSIMPLE_DIR}/*.h)
    file(GLOB FSSIMPLE_SOURCES ${FSSIMPLE_DIR}/*.cpp)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(FSSIMPLE_DIR "fssimple/macosx")
    file(GLOB FSSIMPLE_HEADER ${FSSIMPLE_DIR}/*.h)
    file(GLOB FSSIMPLE_SOURCES ${FSSIMPLE_DIR}/*.cpp ${FSSIMPLE_DIR}/*.m)
endif()

# demo targets (course material, close-source)
add_subdirectory(demos)

# Homework targets
add_executable(homeworks main.cpp)
add_executable(ps1 ps1/ps1-3.cpp)

add_subdirectory(ps2)
add_subdirectory(ps3)

