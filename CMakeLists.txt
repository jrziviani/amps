#-----------------------------------------
# CMake setup
#-----------------------------------------
cmake_minimum_required (VERSION 3.10)
project (volt VERSION 0.0
              DESCRIPTION "Text Template Engine"
              LANGUAGES CXX)

#-----------------------------------------
# C++ compiler setup
#-----------------------------------------
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic -Wextra")

#-----------------------------------------
# It's not ready even for Linux...
#-----------------------------------------
if(NOT UNIX)
    message(FATAL_ERROR "Not ready...")
endif()

#-----------------------------------------
# Binary directories
#-----------------------------------------
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/lib)

#-----------------------------------------
# Not sure if I'll keep this
#-----------------------------------------
configure_file (
    "${PROJECT_SOURCE_DIR}/config.h.in"
    "${PROJECT_SOURCE_DIR}/src/config.h"
)

#-----------------------------------------
# Setup main directories
#-----------------------------------------
add_subdirectory(lib)
add_subdirectory(src)

#-----------------------------------------
# Setup main directories
#-----------------------------------------
enable_testing()
add_subdirectory(test)
