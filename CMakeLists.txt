cmake_minimum_required(VERSION 3.20)
project(ProjetC C) # TODO

set(CMAKE_C_STANDARD 11)
add_executable(ProjetC clientSrc.c)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK4 REQUIRED gtk4)

include_directories(${GTK4_INCLUDE_DIRS})
link_directories(${GTK4_LIBRARY_DIRS})

add_definitions(${GTK4_CFLAGS_OTHER})

target_link_libraries(ProjetC ${GTK4_LIBRARIES})

