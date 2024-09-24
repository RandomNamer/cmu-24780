add_executable(ps3-1 ps3-1.cpp)

#include_directories(${PROJECT_SOURCE_DIR}/${FSSIMPLE_DIR})
add_executable(ps3-2 MACOSX_BUNDLE ps3-2.cpp ${FSSIMPLE_SOURCES} ${FSSIMPLE_HEADER})
target_link_libraries(ps3-2 ${COCOA_LIBRARY} ${OGL_LIBRARY})