#ifndef EDITOR_H
#define EDITOR_H

#include <stdbool.h>
#include <stdio.h>
#include <termios.h>
#include <time.h>

#include "rope.h"

# define ABUF_INIT {NULL, 0, 0}
# define CTRL_PLUS(ch) ((ch) & 0x1f)  // 'Ctrl + <ch>'
# define TAB_WIDTH 4


// Modes of the editor
typedef enum EditorMode {
    MODE_NORMAL,
    MODE_INSERT,
    MODE_COMMAND
} EditorMode;

// Maintains the editor’s runtime data and configuration
typedef struct EditorState {
    int cx, cy;                 // cursor coordinate (0-indexed) -> location of cursor in the file
    int rx;                     // rendered x-coordinate
    int snapx;                  // rendered x-coordinate to snap back to if possible
    int screenrows;             // number of visible rows in the screen
    int screencols;             // number of visible columns in the screen
    int rowoff;                 // row offset (0-indexed)
    int coloff;                 // column offset (0-indexed)
    char *filename;             // name of the open file

    char statusmsg[80];         // status message to be displayed at the bottom of the screen
    time_t statusmsg_time;      // timestamp of status message
    bool is_dirty;              // 'true' if the file has unsaved changes
    bool is_insert_mode_dirty;  // 'true' if insert mode made changes

    EditorMode mode;            // current mode of the editor

    RopeNode *rope;             // data structure containing the text buffer
    int numlines;               // number of lines in the rope
} EditorState;

// A dynamic string type which supports appending
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
int handle_normal_keypress(int ch);  // TODO: return 'void' after moving exit command to command mode
void handle_insert_keypress(int ch);
int handle_command_keypress(int ch);

// Output operations
void refresh_screen(void);
void draw_rows(AppendBuffer *ab);
void draw_line(AppendBuffer *ab, int filerow);
void scroll(void);
void draw_status_bar(AppendBuffer *ab);
void set_status_message(const char *fmt, ...);
void draw_message_bar(AppendBuffer *ab);

// Insert mode operations
void insert_at_cursor(char ch);
void insert_char_at_cursor(char ch);
void insert_newline_at_cursor(void);
void delete_char_before_cursor(void);
bool delete_char_at_cursor(void);

// Append buffer operations
void ab_append(AppendBuffer *ab, const char *str, int len);
void ab_free(AppendBuffer *ab);

// Editor initialization
void init_editor(RopeNode *root, const char *filename);

// Helper functions
int cx_to_rx(int line, int cx);
int rx_to_cx(int line, int rx);
int map_vim_nav_key(int ch);
int get_rope_idx_from_cursor(void);

#endif
