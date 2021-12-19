cmake_minimum_required(VERSION 3.20)
project(ProjetC C) # TODO

set(CMAKE_C_STANDARD 11)

add_executable(ProjetC # TODO
        clientSrc.c)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES main.cpp)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK3 REQUIRED gtk+-3.0)

include_directories(${GTK3_INCLUDE_DIRS})
link_directories(${GTK3_LIBRARY_DIRS})

add_definitions(${GTK3_CFLAGS_OTHER})

target_link_libraries(ProjetC ${GTK3_LIBRARIES})