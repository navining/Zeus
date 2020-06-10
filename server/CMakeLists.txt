# Include source files
aux_source_directory(. SRC_LIST)

# Generate executable file
add_executable(server ${SRC_LIST})

# Link libraries
target_link_libraries(server pthread)

