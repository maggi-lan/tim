#include "editor.h"

#include <unistd.h>
#include <stdlib.h>

#include "terminal.h"
#include "rope.h"


// Moves cursor position by updating cursor coordinates
void move_cursor(int key) {
    int rowsize =  get_line_length(E.rope, E.cy);

    switch (key) {
        // NOTE: cursor coordinates stored in 'E' use zero-indexed
        case ARROW_LEFT:
            if (E.cx != 0) {
                // Check char being crossed (char before current cursor position) to adjust rx
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
                // Check char being crossed (char at current cursor position) to adjust rx
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
    }

    return 0;
}
