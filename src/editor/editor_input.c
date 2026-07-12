#include "editor.h"

#include <unistd.h>
#include <stdlib.h>

#include "terminal.h"
#include "rope.h"


// Moves cursor position by updating cursor coordinates
void move_cursor(int key) {
    int rowsize =  get_line_length(E.rope, E.cy);

    // NOTE: cursor coordinates stored in 'E' are 0-indexed
    switch (key) {
        case ARROW_LEFT:
            if (E.cx != 0) {
                // Check the character being crossed (character before the current cursor position) to adjust rx
                char *seg = get_line_segment_from_rope(E.rope, E.cy, E.cx - 1, 1);
                if (seg) {
                    if (seg[0] == '\t')
                        E.rx -= TAB_WIDTH - (E.rx % TAB_WIDTH);
                    else
                        E.rx--;

                    E.cx--;
                    E.snapx = E.rx;

                    free(seg);
                }
            }
            break;

        case ARROW_DOWN:
            if (E.cy != E.numlines - 1) {
                E.cy++;

                E.cx = rx_to_cx(E.cy, E.snapx);  // snaps cursor horizontally
                E.rx = cx_to_rx(E.cy, E.cx);
            }
            break;

        case ARROW_UP:
            if (E.cy != 0) {
                E.cy--;

                E.cx = rx_to_cx(E.cy, E.snapx);  // snaps cursor horizontally
                E.rx = cx_to_rx(E.cy, E.cx);
            }
            break;

        case ARROW_RIGHT:
            if (E.cx < rowsize) {
                // Check the character being crossed (character at the current cursor position) to adjust rx
                char *seg = get_line_segment_from_rope(E.rope, E.cy, E.cx, 1);
                if (seg) {
                    if (seg[0] == '\t')
                        E.rx += TAB_WIDTH - (E.rx % TAB_WIDTH);
                    else
                        E.rx++;

                    E.cx++;
                    E.snapx = E.rx;

                    free(seg);
                }
            }
            break;
    }
}


/*
-> Captures input keypress and executes editor commands
-> Returns -1 when quit command is called
-> Returns 0 if everything works properly
*/
int process_keypress(void) {
    int ch = read_key();

    switch (E.mode) {
        case MODE_NORMAL:
            return handle_normal_keypress(ch);
        case MODE_INSERT:
            return handle_insert_keypress(ch);
        case MODE_COMMAND:
            return handle_command_keypress(ch);
    }

    return 0;
}

/*
-> Handles keypresses in normal mode
-> Returns -1 when quit command is called
-> Returns 0 if everything works properly
*/
int handle_normal_keypress(int ch) {

    switch (ch) {
        // Quit command
        case CTRL_PLUS('q'):
            write(STDOUT_FILENO, "\x1b[2J", 4);  // clear terminal screen
            write(STDOUT_FILENO, "\x1b[H", 3);   // move cursor to top left
            return -1;

        // Cursor movement
        case ARROW_LEFT:
        case ARROW_DOWN:
        case ARROW_UP:
        case ARROW_RIGHT:
            move_cursor(ch);
            break;

        // Move cursor to start/end of line
        case HOME_KEY:
            E.cx = 0;
            E.rx = 0;
            E.snapx = E.rx;
            break;
        case END_KEY:
            E.cx =  get_line_length(E.rope, E.cy);
            E.rx = cx_to_rx(E.cy, E.cx);
            E.snapx = E.rx;
            break;

        // Scroll up/down
        case PAGE_UP:
        case PAGE_DOWN:
            {
                if (ch == PAGE_UP) {
                    E.cy = E.rowoff;
                }
                else if (ch == PAGE_DOWN) {
                    E.cy = E.rowoff + E.screenrows - 1;
                    if (E.cy > E.numlines)
                        E.cy = E.numlines;
                }

                for (int i = 0; i < E.screenrows; i++) {
                    if (ch == PAGE_UP)
                        move_cursor(ARROW_UP);
                    else if (ch == PAGE_DOWN)
                        move_cursor(ARROW_DOWN);
                }
            }
            break;

        // Switch modes
        case 'i':
            E.mode = MODE_INSERT;
            break;
        case ':':
            E.mode = MODE_COMMAND;
            break;
    }

    return 0;
}


// TODO
int handle_insert_keypress(int ch) {
    switch (ch) {
        case '\x1b':  // Escape key
            E.mode = MODE_NORMAL;
            break;
    }

    return 0;
}


// TODO
int handle_command_keypress(int ch) {
    switch (ch) {
        case '\x1b':  // Escape key
            E.mode = MODE_NORMAL;
            break;
    }

    return 0;
}
