#ifndef TERMINAL_H
#define TERMINAL_H


// Special values for some keys
enum SpecialKeys {
    ARROW_UP = 1000,
    ARROW_DOWN,
    ARROW_RIGHT,
    ARROW_LEFT,
    DEL_KEY,
    HOME_KEY,
    END_KEY,
    PAGE_UP,
    PAGE_DOWN,
};


// Terminal core operations
void enable_raw(void);
void disable_raw(void);
int read_key(void);
int get_cursor_pos(int *row, int *col);
int get_window_size(int *rows, int *cols);

// Terminal helpers
void halt(char *str);
int escape_parser(void);


#endif
