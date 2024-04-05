cmake_minimum_required(VERSION 3.21)
project(rayNEAT)

set(CMAKE_CXX_STANDARD 20)

# Allow fetching content from repos
include(FetchContent)
set(FETCHCONTENT_QUIET FALSE)

# Fetch raylib
FetchContent_Declare(
        raylib
        GIT_REPOSITORY "https://github.com/raysan5/raylib.git"
        GIT_TAG "master"
        GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(raylib)

# Add source files
file(GLOB_RECURSE PROJECT_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_LIST_DIR}/src/*")
set(PROJECT_INCLUDE "${CMAKE_CURRENT_LIST_DIR}/src/")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(${PROJECT_NAME})
target_sources(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_INCLUDE})

# link raylib
target_link_libraries(${PROJECT_NAME} PRIVATE raylib)

# copy DLLs
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/dll/ $<TARGET_FILE_DIR:rayNEAT>)
