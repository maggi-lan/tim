add_library(editor)

target_sources(editor
    PRIVATE
        editor_input.c
        editor_output.c
        editor_buffer.c
        editor_init.c

    PUBLIC
        FILE_SET HEADERS
        FILES
            editor.h
)

target_link_libraries(editor
    PUBLIC
        rope
        terminal
)
