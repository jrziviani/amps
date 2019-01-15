#-----------------------------------------
# Main source code setup
#-----------------------------------------
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include_directories(../include)

add_library(volt SHARED
            scan.cpp
            engine.cpp
            token.cpp
            compiler.cpp
            context.cpp)

add_library(volt-static STATIC
            scan.cpp
            engine.cpp
            token.cpp
            compiler.cpp
            context.cpp)
