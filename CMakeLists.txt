cmake_minimum_required(VERSION 3.0)
project(Zeus)

# Compile arguments
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -O3")

# Executable files output path
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

# Header file include path
include_directories(${PROJECT_SOURCE_DIR}/client)
include_directories(${PROJECT_SOURCE_DIR}/server)
include_directories(${PROJECT_SOURCE_DIR}/common)

# Add subdirectory
add_subdirectory(client)
add_subdirectory(server)

