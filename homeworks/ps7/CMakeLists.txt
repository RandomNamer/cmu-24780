add_executable(ps7 MACOSX_BUNDLE ps7.cpp ${FSSIMPLE_SOURCES} )
target_link_libraries(ps7 ${COCOA_LIBRARY} ${OGL_LIBRARY})