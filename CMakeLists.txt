# General info
cmake_minimum_required(VERSION 3.20)
project(ProjetC C) # TODO

# Create Executable
set(CMAKE_C_STANDARD 11)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/build/")
add_executable(ProjetC clientSrc.c
        sources/window.c headers/window.h
        sources/config.c headers/config.h sources/configTypes.c
        sources/database.c headers/database.h sources/databaseTypes.c
        sources/jsonParse.c headers/jsonParse.h
        sources/login.c headers/login.h
        headers/macros.h)


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

# Use PKG config to find GTK
find_package(PkgConfig REQUIRED)

# Add GTK4 to project
pkg_check_modules(GTK4 REQUIRED gtk4)
include_directories(${GTK4_INCLUDE_DIRS})
link_directories(${GTK4_LIBRARY_DIRS})
add_definitions(${GTK4_CFLAGS_OTHER})
target_link_libraries(ProjetC ${GTK4_LIBRARIES})

# Add SQLite3 to project
find_package(SQLite3)
pkg_check_modules(SQLite3 REQUIRED SQLite3)
include_directories(${SQLite3_INCLUDE_DIRS})
link_directories(${SQLite3_LIBRARY_DIRS})
add_definitions(${SQLite3_CFLAGS_OTHER})
target_link_libraries(ProjetC ${SQLite3_LIBRARIES})

# Add json-c to project
find_package(json-c)
pkg_check_modules(json-c REQUIRED json-c)
include_directories(${json-c_INCLUDE_DIRS})
link_directories(${json-c_LIBRARY_DIRS})
add_definitions(${json-c_CFLAGS_OTHER})
target_link_libraries(ProjetC ${json-c_LIBRARIES})

# Add openssl libs to project
find_package(OpenSSL)
pkg_check_modules(OpenSSL REQUIRED OpenSSL)
include_directories(${OpenSSL_INCLUDE_DIRS})
link_directories(${OpenSSL_LIBRARY_DIRS})
add_definitions(${OpenSSL_CFLAGS_OTHER})
target_link_libraries(ProjetC ${OpenSSL_LIBRARIES})
