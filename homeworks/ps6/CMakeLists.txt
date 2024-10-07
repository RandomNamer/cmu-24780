add_executable(ps6 MACOSX_BUNDLE ps6.cpp ${FSSIMPLE_SOURCES} )
target_link_libraries(ps6 ${COCOA_LIBRARY} ${OGL_LIBRARY})