#include "editor.h"

#include <unistd.h>

#include "terminal.h"
#include "rope.h"


// Moves cursor position by updating cursor coordinates
void move_cursor(int key) {
    int rowsize =  get_line_length(E.rope, E.cy);

    switch (key) {
        // NOTE: cursor coordinates stored in 'E' use zero-indexed
        case ARROW_LEFT:
            if (E.cx != 0)
                E.cx--;
            break;
        case ARROW_DOWN:
            if (E.cy != E.numlines - 1)
                E.cy++;
            break;
        case ARROW_UP:
            if (E.cy != 0)
                E.cy--;
            break;
        case ARROW_RIGHT:
            if (E.cx < rowsize)
                E.cx++;
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
            break;
        case END_KEY:
            E.cx = E.screencols - 1;
            break;

        // Scroll up/down
        case PAGE_UP:
        case PAGE_DOWN:
            {
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
