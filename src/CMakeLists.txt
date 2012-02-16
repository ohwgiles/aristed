cmake_minimum_required(VERSION 2.6)
project(aristed)

# Grab required libraries
find_package(Qt4 COMPONENTS QtCore QtGui REQUIRED)
link_directories(${QT_LIBRARY_DIR} /usr/lib/llvm)

# Generate C++ from Qt objects
qt4_wrap_cpp(UI_MOCS mainwindow.hpp editor.hpp linenumberbar.hpp cxx/cxxeditor.hpp cxx/clangfile.hpp cxx/semanticworker.hpp)
qt4_wrap_ui(UI_OUTPUT mainwindow.ui)

include_directories(${QT_INCLUDES} ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

execute_process(COMMAND ls -r /usr/include/c++ COMMAND head -1 OUTPUT_VARIABLE CXX_STDLIB_VER OUTPUT_STRIP_TRAILING_WHITESPACE)
add_definitions("-g -fno-rtti -D__STDC_LIMIT_MACROS -D__STDC_CONSTANT_MACROS -DCXX_STDLIB_VER=\"\\\"${CXX_STDLIB_VER}\\\"\" -Wall -Wextra -Werror")

add_executable(ae main.cpp mainwindow.cpp cxx/cxxeditor.cpp cxx/clangfile.cpp editor.cpp linenumberbar.hpp log.cpp ${UI_MOCS} ${UI_OUTPUT})
target_link_libraries(ae
	${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY}
	clangCodeGen clangRewrite clangFrontend clangSerialization clangFrontend clangSema
	clangAnalysis clangDriver clangAST clangParse clangLex clangBasic
	LLVMMC LLVMCore LLVMSupport LLVMBitWriter LLVMBitReader LLVMCodeGen
	LLVMAnalysis LLVMTarget pthread dl)
