add_executable(ps8 ps8.cpp ${FSSIMPLE_SOURCES} ${FSSIMPLE_HEADER} ${YSPNG})
target_link_libraries(ps8 ${COCOA_LIBRARY} ${OGL_LIBRARY})

copy_resources(ps8)