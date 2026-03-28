add_library(rope)

target_sources(rope
    PRIVATE
        rope_core.c
        rope_avl.c
        rope_helper.c
        rope_utility.c

    PUBLIC
        FILE_SET HEADERS
        FILES
            rope.h
)

target_link_libraries(rope
    PUBLIC
        terminal
)
