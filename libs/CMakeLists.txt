cmake_minimum_required (VERSION 2.6)

project (edifix)

set (CMAKE_CXX_STANDARD 11)

add_definitions(-DWITH_NCURSES)

add_executable(edifix EdifixMain.cpp
  EdifixEditor.cpp
  EdifixEditor.h
)            

install(TARGETS edifix DESTINATION bin)
