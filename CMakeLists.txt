cmake_minimum_required(VERSION 3.0)
project(Zeus)

# Compile arguments
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -std=c++11)

# Executable files output path
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

# Header file include path
include_directories(${PROJECT_SOURCE_DIR}/Zeus-Client)
include_directories(${PROJECT_SOURCE_DIR}/Zeus-Server)

# Add subdirectory
add_subdirectory(Zeus-Client)
add_subdirectory(Zeus-Server)

