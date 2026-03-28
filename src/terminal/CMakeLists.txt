add_library(terminal)

target_sources(terminal
    PRIVATE
        terminal_core.c
        terminal_helper.c
        terminal_buffer.c

    PUBLIC
        FILE_SET HEADERS
        FILES
            terminal.h
)
