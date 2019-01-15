#-----------------------------------------
# Sample code setup
#-----------------------------------------
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include_directories(../include)

add_executable(volt_sample
               main.cpp)

target_link_libraries(volt_sample LINK_PUBLIC ${CMAKE_DL_LIBS} volt-static)

add_custom_command(TARGET volt_sample POST_BUILD
    COMMAND cp ${CMAKE_SOURCE_DIR}/sample/*.tpl ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
)
