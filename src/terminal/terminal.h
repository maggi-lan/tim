#ifndef TERMINAL_H
#define TERMINAL_H

#include <termios.h>

# define ABUF_INIT {NULL, 0}


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


/*
-> Global terminal state
-> It's the state before enabling raw mode
*/
extern struct termios old_term;

// AppendBuffer is a dynamic string type which supports appending
typedef struct AppendBuffer {
    char *buffer;  // buffer for the string
    int len;       // length of the buffer
} AppendBuffer;


// Core operations
void enable_raw(void);
void disable_raw(void);
int read_key(void);
int get_cursor_pos(int *row, int *col);
int get_window_size(int *rows, int *cols);

// Helper functions
void halt(char *str);
int escape_parser(void);

// Append buffer operations
void ab_append(AppendBuffer *ab, char *str, int len);
void ab_free(AppendBuffer *ab);


#endif
