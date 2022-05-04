cmake_minimum_required(VERSION 3.16)

project("ExtraWordPuzzle")

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

find_package(Curses REQUIRED)
find_library(MENU_LIBRARY menu)
include_directories(${CURSES_INCLUDE_DIR})

#set(SOURCE_FILES main.cpp)
file(GLOB SOURCES "*.cpp")

add_executable(main ${SOURCES})
target_link_libraries(main ${MENU_LIBRARY} ${CURSES_LIBRARY})
