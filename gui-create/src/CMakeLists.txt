set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

aux_source_directory(. GUI_SOURCE_FILES)

add_executable(mcy-create ${GUI_SOURCE_FILES} ${GUI_RESOURCE_FILES})
set_target_properties(mcy-create PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})
target_compile_definitions(mcy-create PRIVATE QT_NO_KEYWORDS EXPORT_IMPORT_API=)
target_link_libraries(mcy-create LINK_PUBLIC Qt5::Widgets)
install(TARGETS mcy-create RUNTIME DESTINATION bin)