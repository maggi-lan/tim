cmake_minimum_required(VERSION 3.23)
project(tim C)




add_executable(tim)

target_sources(tim
    PRIVATE
        src/main.c
)

target_link_libraries(tim
    PRIVATE
        rope
        file_io
)




add_subdirectory(src/rope)
add_subdirectory(src/file_io)
