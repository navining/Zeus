# Include source files
aux_source_directory(. SRC_SERVER)
aux_source_directory(../common SRC_COMMON)

# Generate executable file
add_executable(server ${SRC_SERVER} ${SRC_COMMON})

# Link libraries
target_link_libraries(server pthread)

