#-----------------------------------------
# Build test
#-----------------------------------------
add_executable(volt_test
               main.cpp
               ../src/scan.cpp
               ../src/token.cpp)
target_link_libraries(volt_test LINK_PUBLIC ${CMAKE_DL_LIBS} gmock_main)

configure_file (
    "${PROJECT_SOURCE_DIR}/test/block.1"
    "${CMAKE_BINARY_DIR}/test/block.1"
)


#-----------------------------------------
# Google Test Framework
#-----------------------------------------
configure_file(CMakeLists.txt.in googletest-download/CMakeLists.txt)
execute_process(COMMAND ${CMAKE_COMMAND} -F "${CMAKE_GENERATOR}" .
                RESULT_VARIABLE result
                WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/test/googletest-download )
if(result)
    message(FATAL_ERROR "CMake step for googletest failed: ${result}")
endif()

execute_process(COMMAND ${CMAKE_COMMAND} --build .
                RESULT_VARIABLE result
                WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/test/googletest-download )
if(result)
    message(FATAL_ERROR "Build step for googletest failed: ${result}")
endif()

# Prevent overriding the parent project's compiler/linker
# settings on Windows
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# Add googletest directly to our build. This defines
# the gtest and gtest_main targets.
add_subdirectory(${CMAKE_BINARY_DIR}/googletest-src
                 ${CMAKE_BINARY_DIR}/googletest-build
                 EXCLUDE_FROM_ALL)

#-----------------------------------------
# American Fuzzy Library (not done yet)
#-----------------------------------------
# set (CMAKE_C_COMPILER "build/afl-2.52b/afl-clang")
# set (CMAKE_CXX_COMPILER "build/afl-2.52b/afl-clang++")
# set (CMAKE_C_COMPILER "/usr/bin/clang")
# set (CMAKE_CXX_COMPILER "/usr/bin/clang++")
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=memory -fsanitize=undefined -fsanitize=integer -fsanitize=nullability -fvisibility=hidden -flto -fsanitize=cfi -fsanitize=safe-stack")
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=safe-stack")
