add_executable(ps9 MACOSX_BUNDLE ps9.cpp ${FSSIMPLE_SOURCES} ${FSSIMPLE_HEADER} ${YSPNG})
target_link_libraries(ps9 ${COCOA_LIBRARY} ${OGL_LIBRARY})

copy_resources(ps9)