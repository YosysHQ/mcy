CMAKE_MINIMUM_REQUIRED(VERSION 2.8.11)
PROJECT(ScintillaEdit)

FIND_PACKAGE(Qt5Widgets REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(SCINTILLA_EDIT_SRC 
    qt/ScintillaEdit/ScintillaEdit.cpp
    qt/ScintillaEdit/ScintillaDocument.cpp
    qt/ScintillaEdit/ScintillaEdit.h
    qt/ScintillaEdit/ScintillaDocument.h
    qt/ScintillaEditBase/PlatQt.cpp
    qt/ScintillaEditBase/ScintillaQt.cpp
    qt/ScintillaEditBase/ScintillaEditBase.cpp
    qt/ScintillaEditBase/ScintillaQt.h
    qt/ScintillaEditBase/ScintillaEditBase.h
    src/XPM.cxx
    src/ViewStyle.cxx
    src/UniConversion.cxx
    src/Style.cxx
    src/Selection.cxx
    src/ScintillaBase.cxx
    src/RunStyles.cxx
    src/RESearch.cxx
    src/PositionCache.cxx
    src/PerLine.cxx
    src/MarginView.cxx
    src/LineMarker.cxx
    src/KeyMap.cxx
    src/Indicator.cxx
    src/ExternalLexer.cxx
    src/EditView.cxx
    src/Editor.cxx
    src/EditModel.cxx
    src/Document.cxx
    src/Decoration.cxx
    src/DBCS.cxx
    src/ContractionState.cxx
    src/CharClassify.cxx
    src/CellBuffer.cxx
    src/Catalogue.cxx
    src/CaseFolder.cxx
    src/CaseConvert.cxx
    src/CallTip.cxx
    src/AutoComplete.cxx
    lexlib/WordList.cxx
    lexlib/StyleContext.cxx
    lexlib/PropSetSimple.cxx
    lexlib/LexerSimple.cxx
    lexlib/LexerNoExceptions.cxx
    lexlib/LexerModule.cxx
    lexlib/LexerBase.cxx
    lexlib/DefaultLexer.cxx
    lexlib/CharacterSet.cxx
    lexlib/CharacterCategory.cxx
    lexlib/Accessor.cxx
)

aux_source_directory(lexers SCINTILLA_LEXERS_SRC)

add_library(ScintillaEdit STATIC ${SCINTILLA_EDIT_SRC} ${SCINTILLA_LEXERS_SRC})
target_compile_definitions(ScintillaEdit PRIVATE SCINTILLA_QT=1 SCI_LEXER=1 EXPORT_IMPORT_API=)
target_compile_options(ScintillaEdit PRIVATE -Wno-strict-overflow)
target_include_directories(ScintillaEdit PRIVATE qt/ScintillaEditBase include src lexlib)
target_link_libraries(ScintillaEdit Qt5::Widgets)