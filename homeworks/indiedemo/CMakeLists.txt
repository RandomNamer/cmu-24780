
add_executable(indiedemo demo.cpp ${FSSIMPLE_SOURCES} ${FSSIMPLE_HEADER} ${SIMPLESOUND_SOURCES} ${SIMPLESOUND_HEADER} ${BITMAP_FONT_SOURCES} ${BITMAP_FONT_HEADER})
target_link_libraries(indiedemo ${COCOA_LIBRARY} ${OGL_LIBRARY} ${AVFOUNDATION_FRAMEWORK})
target_include_directories(indiedemo PRIVATE ${OPENGL_INCLUDE_DIR})

file(GLOB RESOURCES res/*)
list(FILTER RESOURCES EXCLUDE REGEX "original|DS_Store")
message(STATUS "indiedemo > RESOURCES list: ${RESOURCES}")
#message(STATUS "indiedemo > TARGET_BIN_DIR:  $<TARGET_FILE_DIR:indiedemo>")
foreach(RES ${RESOURCES})
    add_custom_command(TARGET indiedemo POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${RES} $<TARGET_FILE_DIR:indiedemo>)
endforeach()