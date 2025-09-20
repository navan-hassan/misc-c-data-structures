cmake_minimum_required(VERSION 3.15)
project(misc_c_data_structures C)

set(CMAKE_C_STANDARD 17)

find_package(xxHash REQUIRED)

add_subdirectory(example)

add_library(misc_c_data_structures 
    src/hash_table.c
    src/list_utilities.c
    src/quad_tree.c
)

target_include_directories(misc_c_data_structures
PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(misc_c_data_structures
    xxHash::xxhash
)

install(TARGETS misc_c_data_structures DESTINATION "."
    RUNTIME DESTINATION bin
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
)
