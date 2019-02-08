#-----------------------------------------
# Main source code setup
#-----------------------------------------
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include_directories(../include)

option(enable-static "enable-static" OFF)
if (enable-static)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
    add_library(amps-static STATIC
                scan.cpp
                engine.cpp
                token.cpp
                compiler.cpp
                context.cpp)
else(enable-static)
    add_library(amps SHARED
                scan.cpp
                engine.cpp
                token.cpp
                compiler.cpp
                context.cpp)
endif(enable-static)
