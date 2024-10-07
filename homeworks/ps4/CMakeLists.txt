
add_executable(ps4-1 MACOSX_BUNDLE ps4-1.cpp ${FSSIMPLE_SOURCES} ${FSSIMPLE_HEADER} compatibility.h osx_compatibility.m)
target_link_libraries(ps4-1 ${COCOA_LIBRARY} ${OGL_LIBRARY})

add_executable(ps4-2 MACOSX_BUNDLE ps4-2.cpp ${FSSIMPLE_SOURCES} )
target_link_libraries(ps4-2 ${COCOA_LIBRARY} ${OGL_LIBRARY})

add_executable(ps4-2-d MACOSX_BUNDLE ps4-2-d.cpp ${FSSIMPLE_SOURCES} ${FSSIMPLE_HEADER})
target_link_libraries(ps4-2-d ${COCOA_LIBRARY} ${OGL_LIBRARY})