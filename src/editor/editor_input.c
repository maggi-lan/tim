#include "editor.h"

#include <unistd.h>

#include "terminal.h"
#include "rope.h"
#include "file_io.h"


// Moves cursor position by updating cursor coordinates
void move_cursor(int key) {
    int rowsize =  get_line_length(E.rope, E.cy);

    // NOTE: cursor coordinates stored in 'E' are 0-indexed
    switch (key) {
        case ARROW_LEFT:
            if (E.cx != 0) {
                E.cx--;
                E.rx = cx_to_rx(E.cy, E.cx);
                E.snapx = E.rx;
            }
            break;

        case ARROW_DOWN:
            if (E.cy != E.numlines - 1 && E.numlines > 0) {
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
            int limit = (E.mode == MODE_INSERT) ? rowsize : rowsize - 1;

            if (E.cx < limit) {
                E.cx++;
                E.rx = cx_to_rx(E.cy, E.cx);
                E.snapx = E.rx;
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
            // TODO: remove 'return' after moving exit command to command mode
            return handle_normal_keypress(ch);
        case MODE_INSERT:
            handle_insert_keypress(ch);
            break;
        case MODE_COMMAND:
            // TODO: add 'return' after moving exit command to command mode
            handle_command_keypress(ch);
            break;
    }

    return 0;
}


/*
-> Handles keypresses in normal mode
-> Returns -1 when quit command is called
-> Returns 0 if everything works properly
-> TODO: return 'void' after moving exit command to command mode
*/
int handle_normal_keypress(int ch) {
    ch = map_vim_nav_key(ch);

    switch (ch) {
        // TODO: remove this after moving exit command to command mode
        // Save/Quit command
        case CTRL_PLUS('s'):
            if (save_file(E.rope, E.filename)) {
                E.is_dirty = false;
                E.is_insert_mode_dirty = false;
            }
            break;
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
            move_cursor(ARROW_LEFT);
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

        // Delete character at cursor
        case 'x':
        case DEL_KEY:
            if (delete_char_at_cursor())
                E.is_dirty = true;
            break;

        // Switch modes
        case 'i':
        case 'a':
        case INS_KEY:
            E.mode = MODE_INSERT;
            if (ch == 'a')
                move_cursor(ARROW_RIGHT);
            break;
        case ':':
            E.mode = MODE_COMMAND;
            break;
    }

    return 0;
}


// Handles keypresses in insert mode
void handle_insert_keypress(int ch) {
    switch (ch) {
        case '\x1b':  // Escape key
            E.mode = MODE_NORMAL;
            move_cursor(ARROW_LEFT);
            if (E.is_insert_mode_dirty) {
                E.is_dirty = true;
                E.is_insert_mode_dirty = false;
            }
            break;

        // Cursor movement
        case ARROW_LEFT:
        case ARROW_DOWN:
        case ARROW_UP:
        case ARROW_RIGHT:
            move_cursor(ch);
            break;


        case BACKSPACE:
        case CTRL_PLUS('h'):
            delete_char_before_cursor();
            break;

        case DEL_KEY:
            delete_char_at_cursor();
            break;

        case '\r':  // Enter key
            insert_newline_at_cursor();
            break;

        default:
            // Printable characters have ASCII range from 32 to 126
            // Allow tab character (ASCII 9) to be inserted as well
            if ((ch >= 32 && ch <= 126) || ch == '\t')
                insert_char_at_cursor(ch);
            break;
    }
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
