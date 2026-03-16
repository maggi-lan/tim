add_library(file_io)

target_sources(file_io
    PRIVATE
        file_io.c

    PUBLIC
        FILE_SET HEADERS
        FILES
            file_io.h
)

target_link_libraries(file_io
    PRIVATE
        rope
)
