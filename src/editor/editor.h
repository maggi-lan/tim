#ifndef EDITOR_H
#define EDITOR_H

#include <stdbool.h>
#include <stdio.h>
#include <termios.h>
#include <time.h>

#include "rope.h"

# define ABUF_INIT {NULL, 0, 0}
# define TIM_VERSION "0.0.1"
# define CTRL_PLUS(ch) ((ch) & 0x1f)  // 'Ctrl+Ch'
# define TAB_WIDTH 4


// EditorState maintains the editor’s runtime data and configuration
typedef struct EditorState {
    int cx, cy;               // cursor coordinate (zero-indexed) -> location of cursor in the file
    int rx;                   // rendered x-coordinate
    int snapx;                // rendered x-coordinate to snap back to if possible
    int screenrows;           // number of visible rows in the screen
    int screencols;           // number of visible columns in the screen
    int rowoff;               // row offset (zero-indexed)
    int coloff;               // column offset (zero-indexed)
    char *filename;           // name of the open file

    char statusmsg[80];       // status message to be displayed at the bottom of the screen
    time_t statusmsg_time;    // timestamp of status message

    RopeNode *rope;           // data structure containing the text buffer
    int numlines;             // number of lines in rope
} EditorState;

// AppendBuffer is a dynamic string type which supports appending
typedef struct AppendBuffer {
    char *buffer;  // buffer for the string (doesn't include null terminator)
    int bufflen;   // number of items occupied in the buffer
    int capacity;  // max capacity of the buffer
} AppendBuffer;


// Global editor state
extern EditorState E;


// Input operations
void move_cursor(int key);
int process_keypress(void);

// Output operations
void refresh_screen(void);
void draw_rows(AppendBuffer *ab);
void scroll(void);
void draw_status_bar(AppendBuffer *ab);
void set_status_message(const char *fmt, ...);
void draw_message_bar(AppendBuffer *ab);

// Append buffer operations
void ab_append(AppendBuffer *ab, const char *str, int len);
void ab_free(AppendBuffer *ab);

// Editor initialization
void init_editor(RopeNode *root, const char *filename);

// Helper functions
int cx_to_rx(int line, int cx);
int rx_to_cx(int line, int rx);

#endif
