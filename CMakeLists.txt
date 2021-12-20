# General info
cmake_minimum_required(VERSION 3.20)
project(ProjetC C) # TODO

# Create Executable
set(CMAKE_C_STANDARD 11)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/build/")
add_executable(ProjetC clientSrc.c)


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

# Use PKG config to find GTK
find_package(PkgConfig REQUIRED)

# Add GTK4 to project
pkg_check_modules(GTK4 REQUIRED gtk4)
include_directories(${GTK4_INCLUDE_DIRS})
link_directories(${GTK4_LIBRARY_DIRS})
add_definitions(${GTK4_CFLAGS_OTHER})
target_link_libraries(ProjetC ${GTK4_LIBRARIES})

# Add sqlite3 to project
find_package(SQLite3)
pkg_check_modules(SQlite3 REQUIRED Sqlite3)
include_directories(${SQLite3_INCLUDE_DIRS})
link_directories(${SQLite3_LIBRARY_DIRS})
add_definitions(${SQLite3_CFLAGS_OTHER})
