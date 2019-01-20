#-----------------------------------------
# Sample code setup
#-----------------------------------------
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

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

add_custom_command(TARGET amps_sample POST_BUILD
    COMMAND cp ${CMAKE_SOURCE_DIR}/sample/*.tpl ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
)
