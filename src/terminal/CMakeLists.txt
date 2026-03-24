add_library(terminal)

target_sources(terminal
    PRIVATE
        terminal_core.c
        terminal_helper.c

    PUBLIC
        FILE_SET HEADERS
        FILES
            terminal.h
)

target_link_libraries(terminal
    PRIVATE
        editor
)
