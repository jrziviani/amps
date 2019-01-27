#-----------------------------------------
# Build test
#-----------------------------------------
if (UNIX)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic -Wextra -Werror -fprofile-arcs -ftest-coverage")
else()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
endif()
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/test)

include_directories(../include)

add_executable(amps_test
               main.cpp
               ../src/compiler.cpp
               ../src/context.cpp
               ../src/scan.cpp
               ../src/token.cpp)

target_link_libraries(amps_test LINK_PUBLIC ${CMAKE_DL_LIBS} gmock_main)

if (UNIX)
	add_custom_target(coverage
		COMMAND ${CMAKE_MAKE_PROGRAM} test
		WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
	)

	add_custom_command(TARGET coverage
		COMMAND gcovr --root ${CMAKE_SOURCE_DIR}/src
			          --object-directory ${CMAKE_BINARY_DIR}/test
				      --html --html-details -o coverage.html
		COMMAND ${CMAKE_COMMAND} -E echo '---------------------------------------------------------'
		COMMAND ${CMAKE_COMMAND} -E echo ' Coverage test generated, results at '
		COMMAND ${CMAKE_COMMAND} -E echo ' file://${CMAKE_BINARY_DIR}/test/coverage.html '
		COMMAND ${CMAKE_COMMAND} -E echo '---------------------------------------------------------'
	)

	add_dependencies(coverage amps_test)
	set_property(DIRECTORY APPEND PROPERTY ADDITIONAL_MAKE_CLEAN_FILES coverage)
endif()

add_test(NAME test COMMAND amps_test WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin/test)

if (UNIX)
	add_custom_command(TARGET amps_test POST_BUILD
		COMMAND cp ${CMAKE_SOURCE_DIR}/test/block.* ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
	    COMMAND cp ${CMAKE_SOURCE_DIR}/test/code.* ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
	)
else()
	add_custom_command(TARGET amps_test POST_BUILD
		COMMAND cmd /c ${CMAKE_SOURCE_DIR}/copy.bat ${CMAKE_SOURCE_DIR}/test ${CMAKE_RUNTIME_OUTPUT_DIRECTORY} "block.*"
		COMMAND cmd /c ${CMAKE_SOURCE_DIR}/copy.bat ${CMAKE_SOURCE_DIR}/test ${CMAKE_RUNTIME_OUTPUT_DIRECTORY} "code.*"
	)
endif()

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
