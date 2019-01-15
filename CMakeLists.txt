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
# Not sure if I'll keep this
#-----------------------------------------
configure_file (
    "${PROJECT_SOURCE_DIR}/config.h.in"
    "${PROJECT_SOURCE_DIR}/include/config.h"
)

#-----------------------------------------
# Setup main directories
#-----------------------------------------
add_subdirectory(sample)
add_subdirectory(src)

#-----------------------------------------
# Setup main directories
#-----------------------------------------
option(enable-test "enable-test" OFF)
if (enable-test)
    enable_testing()
    add_subdirectory(test)
endif(enable-test)
