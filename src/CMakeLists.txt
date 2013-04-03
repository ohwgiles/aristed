cmake_minimum_required(VERSION 2.8.9)
project(aristed)

# You may need to set these relative to your clang installation
set(ARCH_HEADERS "/usr/lib/clang/3.1/include" CACHE PATH "Path to architecture-specific headers")
set(CLANG_LIBRARY_PATH "/usr/lib/llvm" CACHE PATH "Path to clang shared library")
set(CLANG_INCLUDE_DIR "/usr/include/clang" CACHE PATH "Path to clang headers")

set(CMAKE_AUTOMOC TRUE)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt5Widgets REQUIRED)

include_directories( ${CLANG_INCLUDE_DIR})
link_directories(${CLANG_LIBRARY_PATH})

add_definitions(
 -g
 -std=c++11
 -fno-rtti
 -DARCH_HEADERS=\"${ARCH_HEADERS}\"
 -Wall
 -Wextra
 -Werror
)

add_executable(ae
 main.cpp
 window.cpp
 codemodel.cpp
 cxxmodel.cpp
 codedecoration.cpp
 editor.cpp
 filesearchdialog.cpp
 highlighter.cpp
 log.cpp
 linenumberpanel.cpp
 project.cpp
 searchpanel.cpp
)

qt5_use_modules(ae Widgets)
target_link_libraries(ae clang)
