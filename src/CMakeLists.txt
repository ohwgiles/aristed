cmake_minimum_required(VERSION 2.8.6) # 2.8.6 required for automoc
project(aristed)

# You may need to set these relative to your clang installation
set(ARCH_HEADERS "/usr/lib/clang/3.1/include" CACHE PATH "Path to architecture-specific headers")
set(CLANG_LIBRARY_PATH "/usr/lib/llvm" CACHE PATH "Path to clang shared library")
set(CLANG_INCLUDE_DIR "/usr/include/clang" CACHE PATH "Path to clang headers")

set(CMAKE_AUTOMOC TRUE)

# Grab required libraries
find_package(Qt4 COMPONENTS QtCore QtGui REQUIRED)
link_directories(${QT_LIBRARY_DIR} ${CLANG_LIBRARY_PATH})

qt4_wrap_ui(UI_OUTPUT mainwindow.ui)
qt4_add_resources(RCC resources.qrc)

include_directories(
 ${QT_INCLUDES}
 ${CLANG_INCLUDE_DIR}
 ${CMAKE_CURRENT_BINARY_DIR}
 ${CMAKE_CURRENT_SOURCE_DIR}
)


add_definitions(
 -g
 -fno-rtti
 -DARCH_HEADERS=\"${ARCH_HEADERS}\"
 -Wall
 -Wextra
 -Werror
)

add_executable(ae
 main.cpp
 mainwindow.cpp
 codemodel.cpp
 cxxmodel.cpp
 textstyle.cpp
 editor.cpp
 linenumberbar.hpp
 highlighter.cpp
 log.cpp
qlinenumberpanel.cpp
qpanel.cpp
qpanellayout.cpp
 ${UI_MOCS}
 ${UI_OUTPUT}
)

target_link_libraries(ae
 ${QT_QTCORE_LIBRARY}
 ${QT_QTGUI_LIBRARY}
 clang
)
