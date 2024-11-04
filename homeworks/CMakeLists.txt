cmake_minimum_required(VERSION 3.20)
project(homeworks)

set(CMAKE_CXX_STANDARD 20)
SET(CMAKE_C_FLAGS_DEBUG "-D_DEBUG")

# OGL setup
find_package(OpenGL REQUIRED)
#find_library(OGL_LIBRARY OpenGL)
set(OGL_LIBRARY OpenGL::GL)

# macOS support for Cocoa
if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    enable_language(OBJC OBJCXX)
    find_library(COCOA_LIBRARY Cocoa)

    find_library(AVFOUNDATION_FRAMEWORK AVFoundation)
    if(AVFOUNDATION_FRAMEWORK)
        message(STATUS "AVFoundation framework found: ${AVFOUNDATION_FRAMEWORK}")
    else()
        message(FATAL_ERROR "AVFoundation framework not found")
    endif()
endif()

# FSSimple setup
set(FSSIMPLE_HEADER)
set(FSSIMPLE_SOURCES)
set(SIMPLESOUND_HEADER)
set(SIMPLESOUND_SOURCES)
set(FSSIMPLE_DIR_PLATFORM_SPECIFIC)

if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    set(FSSIMPLE_DIR_PLATFORM_SPECIFIC ${PROJECT_SOURCE_DIR}/fssimple/windows)
    list(APPEND FSSIMPLE_HEADER ${FSSIMPLE_DIR_PLATFORM_SPECIFIC}/fssimplewindow.h)
    list(APPEND FSSIMPLE_SOURCES ${FSSIMPLE_DIR_PLATFORM_SPECIFIC}/fssimplewindow.cpp)
    list(APPEND SIMPLESOUND_HEADER ${FSSIMPLE_DIR_PLATFORM_SPECIFIC}/yssimplesound.h)
    list(APPEND SIMPLESOUND_SOURCES ${FSSIMPLE_DIR_PLATFORM_SPECIFIC}/yssimplesound.cpp)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(FSSIMPLE_DIR_PLATFORM_SPECIFIC ${PROJECT_SOURCE_DIR}/fssimple/linux)
    list(APPEND FSSIMPLE_HEADER ${FSSIMPLE_DIR_PLATFORM_SPECIFIC}/fssimplewindow.h)
    list(APPEND FSSIMPLE_SOURCES ${FSSIMPLE_DIR_PLATFORM_SPECIFIC}/fssimplewindow.cpp)
    list(APPEND SIMPLESOUND_HEADER ${FSSIMPLE_DIR_PLATFORM_SPECIFIC}/yssimplesound.h)
    list(APPEND SIMPLESOUND_SOURCES ${FSSIMPLE_DIR_PLATFORM_SPECIFIC}/yssimplesound.cpp)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(FSSIMPLE_DIR_PLATFORM_SPECIFIC ${PROJECT_SOURCE_DIR}/fssimple/macosx)
    list(APPEND FSSIMPLE_HEADER ${FSSIMPLE_DIR_PLATFORM_SPECIFIC}/fssimplewindow.h)
    list(APPEND FSSIMPLE_SOURCES ${FSSIMPLE_DIR_PLATFORM_SPECIFIC}/fssimplewindowobjc.m ${FSSIMPLE_DIR_PLATFORM_SPECIFIC}/fssimplewindowcpp.cpp)
    list(APPEND SIMPLESOUND_HEADER ${FSSIMPLE_DIR_PLATFORM_SPECIFIC}/yssimplesound.h)
    list(APPEND SIMPLESOUND_SOURCES ${FSSIMPLE_DIR_PLATFORM_SPECIFIC}/yssimplesound.cpp ${FSSIMPLE_DIR_PLATFORM_SPECIFIC}/yssimplesound_macosx_objc.m)
endif()

set(BITMAP_FONT_HEADER ${PROJECT_SOURCE_DIR}/fssimple/ysglfontdata.h)
set(BITMAP_FONT_SOURCES ${PROJECT_SOURCE_DIR}/fssimple/ysglfontdata.c)

file(GLOB YSPNG ${PROJECT_SOURCE_DIR}/fssimple/yspng/*)

message(STATUS "FSSIMPLE_DIR_PLATFORM_SPECIFIC: ${FSSIMPLE_DIR_PLATFORM_SPECIFIC}")
message(STATUS "FSSIMPLE_HEADER: ${FSSIMPLE_HEADER}")
message(STATUS "FSSIMPLE_SOURCES: ${FSSIMPLE_SOURCES}")
message(STATUS "SIMPLESOUND_HEADER: ${SIMPLESOUND_HEADER}")
message(STATUS "SIMPLESOUND_SOURCES: ${SIMPLESOUND_SOURCES}")
message(STATUS "YSPNG SOURCE+HEADER: " ${YSPNG})

include_directories(${FSSIMPLE_DIR_PLATFORM_SPECIFIC})
include_directories(${PROJECT_SOURCE_DIR}/fssimple)
include_directories(${PROJECT_SOURCE_DIR}/fssimple/yspng)


#util functions
function(copy_resources target_name)
    file(GLOB RESOURCES res/*)
    list(FILTER RESOURCES EXCLUDE REGEX "original|DS_Store")
    message(STATUS "${target_name} > calling copy resources for: ${RESOURCES}")
    get_target_property(IS_OSX_BUNDLE ${target_name} MACOSX_BUNDLE)
    if(APPLE AND IS_OSX_BUNDLE)
        message(STATUS "${target_name} is a macOS bundle")
        set(DESTINATION_DIR "$<TARGET_BUNDLE_CONTENT_DIR:${target_name}>/MacOS")
    else()
        set(DESTINATION_DIR "$<TARGET_FILE_DIR:${target_name}>")
    endif()

    # Copy each resource file to the target's output directory
    foreach(RES ${RESOURCES})
        add_custom_command(
                TARGET ${target_name}
                POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy
                ${RES}
                ${DESTINATION_DIR}
        )
    endforeach()
endfunction()

# demo targets (course material, close-source)
add_subdirectory(demos)

# Homework targets
add_executable(homeworks main.cpp)
add_executable(ps1 ps1/ps1-3.cpp)

add_subdirectory(ps2)
add_subdirectory(ps3)
add_subdirectory(ps4)
add_subdirectory(ps5)
add_subdirectory(ps6)
add_subdirectory(ps7)
#add_subdirectory(indiedemo)
add_subdirectory(ps8)
add_subdirectory(ps9)

