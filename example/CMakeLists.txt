cmake_minimum_required(VERSION 3.15)
project(c_data_structure_examples C)

set(CMAKE_C_STANDARD 17)

add_executable(c_data_structure_examples 
    src/main.c
)

target_link_libraries(c_data_structure_examples
    misc_c_data_structures
)

install(TARGETS c_data_structure_examples DESTINATION "."
    RUNTIME DESTINATION bin
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
)
