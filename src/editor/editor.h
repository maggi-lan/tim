#ifndef EDITOR_H
#define EDITOR_H

#include <stdbool.h>
#include <stdio.h>
#include <termios.h>

#include "rope.h"
#include "terminal.h"

# define TIM_VERSION "0.0.1"
# define CTRL_PLUS(ch) ((ch) & 0x1f)  // 'Ctrl+Ch'


// EditorState maintains the editor’s runtime data and configuration
typedef struct EditorState {
    int cx, cy;               // cursor coordinate (zero-indexed)
    int screenrows;           // number of visible rows in the screen
    int screencols;           // number of visible columns in the screen

    RopeNode *rope;           // data structure containing the text buffer
    int numlines;             // number of lines in rope
} EditorState;


// Global editor state
extern EditorState E;


// Input operations
void move_cursor(int key);
int process_keypress(void);

// Output operations
void refresh_screen(void);
void draw_rows(AppendBuffer *ab);

// Editor initialization
void init_editor(RopeNode *root);


#endif
