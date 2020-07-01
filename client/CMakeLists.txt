# Include source files
aux_source_directory(. SRC_CLIENT)
aux_source_directory(../utils SRC_COMMON)

# Generate executable file
add_executable(client ${SRC_CLIENT} ${SRC_COMMON})

# Link libraries
target_link_libraries(client pthread)

