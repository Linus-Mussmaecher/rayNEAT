cmake_minimum_required(VERSION 3.21)
project(rayNEAT)

set(CMAKE_CXX_STANDARD 20)

#raylib
find_package(raylib REQUIRED)
include_directories(${RAYLIB_INCLUDE_DIRS})


add_executable(rayNEAT code/main.cpp code/NEAT/rayNEAT.h code/NEAT/connection_gene.cpp code/NEAT/network.cpp code/NEAT/neat.cpp code/scenarios/XOR/XOR.cpp code/scenarios/XOR/XOR.h code/NEAT/node_gene.cpp code/NEAT/utility.cpp code/NEAT/activation_functions.cpp code/scenarios/snake/snake.cpp code/scenarios/snake/snake.h code/scenarios/snake/snake_agents.cpp)

#raylib link
target_link_libraries(rayNEAT ${RAYLIB_LIBRARIES})

#copy resources folder into build folder
add_custom_command(TARGET rayNEAT POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/resources/ $<TARGET_FILE_DIR:rayNEAT>/resources)

#copy dlls
add_custom_command(TARGET rayNEAT POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/dll/ $<TARGET_FILE_DIR:rayNEAT>)