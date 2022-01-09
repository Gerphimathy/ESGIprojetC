# General info
cmake_minimum_required(VERSION 3.20)
project(ProjetC C) # TODO

# Create Executable
set(CMAKE_C_STANDARD 11)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/build/")
add_executable(ProjetC clientSrc.c headers/macros.h
        sources/window.c headers/window.h
        sources/config.c headers/config.h
        sources/database.c headers/database.h
        sources/jsonParse.c headers/jsonParse.h
        sources/login.c headers/login.h
        sources/terminal.c headers/terminal.h
        sources/feed.c headers/feed.h sources/ytapi.c headers/ytapi.h)


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

# Use PKG config to find GTK
find_package(PkgConfig REQUIRED)

# Add GTK3 to project
pkg_check_modules(GTK3 REQUIRED gtk+-3.0)
include_directories(${GTK3_INCLUDE_DIRS})
link_directories(${GTK3_LIBRARY_DIRS})
add_definitions(${GTK3_CFLAGS_OTHER})
target_link_libraries(ProjetC ${GTK3_LIBRARIES})

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

# Add openssl libs to project
find_package(CURL)
include_directories(${CURL_INCLUDE_DIRS})
link_directories(${CURL_LIBRARY_DIRS})
add_definitions(${CURL_CFLAGS_OTHER})
target_link_libraries(ProjetC ${CURL_LIBRARIES})
