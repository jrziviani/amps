#-----------------------------------------
# Sample code setup
#-----------------------------------------
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(../include)

add_executable(amps_sample
               main.cpp)

if (enable-static)
    target_link_libraries(amps_sample LINK_PUBLIC
                          ${CMAKE_DL_LIBS} amps-static)
else(enable-static)
    target_link_libraries(amps_sample LINK_PUBLIC
                          ${CMAKE_DL_LIBS} amps)
endif(enable-static)

if(UNIX)
	add_custom_command(TARGET amps_sample POST_BUILD
		COMMAND cp ${CMAKE_SOURCE_DIR}/sample/*.tpl ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
	)
else()
	add_custom_command(TARGET amps_sample POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/sample/template.tpl ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
	)
endif()
