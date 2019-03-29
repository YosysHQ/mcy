set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

aux_source_directory(. GUI_SOURCE_FILES)

qt5_add_resources(GUI_RESOURCE_FILES base.qrc)

add_executable(mcy-gui ${GUI_SOURCE_FILES} ${GUI_RESOURCE_FILES})
set_target_properties(mcy-gui PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})
target_include_directories(mcy-gui PRIVATE common ../3rdparty/scintilla/qt/ScintillaEdit ../3rdparty/scintilla/qt/ScintillaEditBase ../3rdparty/scintilla/include ../3rdparty/scintilla/lexlib ../3rdparty/QtPropertyBrowser/src)
target_compile_definitions(mcy-gui PRIVATE QT_NO_KEYWORDS EXPORT_IMPORT_API=)
target_link_libraries(mcy-gui LINK_PUBLIC Qt5::Widgets Qt5::Sql ScintillaEdit QtPropertyBrowser ${Boost_LIBRARIES})
install(TARGETS mcy-gui RUNTIME DESTINATION bin)