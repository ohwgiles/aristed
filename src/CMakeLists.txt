cmake_minimum_required(VERSION 2.6)
project(aristed)

# Grab required libraries
find_package(Qt4 COMPONENTS QtCore QtGui REQUIRED)
link_directories(${QT_LIBRARY_DIR} /usr/lib/llvm)

# Generate C++ from Qt objects
qt4_wrap_cpp(UI_MOCS mainwindow.hpp editor.hpp linenumberbar.hpp cxxeditor.hpp semanticthread.hpp highlighter.hpp)
qt4_wrap_ui(UI_OUTPUT mainwindow.ui)
qt4_add_resources(RCC resources.qrc)

include_directories(${QT_INCLUDES} ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

execute_process(COMMAND ls -r /usr/lib/clang/ COMMAND head -1 OUTPUT_VARIABLE CLANG_VER OUTPUT_STRIP_TRAILING_WHITESPACE)
add_definitions("-g -fno-rtti -D__STDC_LIMIT_MACROS -D__STDC_CONSTANT_MACROS -DCLANG_VER=\"\\\"${CLANG_VER}\\\"\" -Wall -Wextra -Werror")

add_executable(ae main.cpp mainwindow.cpp cxxeditor.cpp semanticthread.cpp textstyle.cpp editor.cpp linenumberbar.hpp highlighter.cpp log.cpp ${UI_MOCS} ${UI_OUTPUT})
target_link_libraries(ae
	${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} clang pthread dl)
