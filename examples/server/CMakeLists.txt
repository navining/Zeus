# Include source files
aux_source_directory(. SRC_SERVER)
aux_source_directory(../utils SRC_UTILS)
aux_source_directory(../net SRC_NET)

# Generate executable file
add_executable(server ${SRC_SERVER} ${SRC_UTILS} ${SRC_NET})

# Link libraries
target_link_libraries(server pthread)
