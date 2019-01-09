#-----------------------------------------
# Main source code setup
#-----------------------------------------
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_executable(volt
               scan.cpp
               engine.cpp
               token.cpp
               compiler.cpp
               context.cpp)
