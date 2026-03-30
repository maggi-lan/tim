#include "editor.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "terminal.h"
#include "rope.h"


// Re-draws entire editor screen
void refresh_screen(void) {
    scroll();

    AppendBuffer ab = ABUF_INIT;

    ab_append(&ab, "\x1b[?25l", 6);  // this escape sequence hides cursor
    ab_append(&ab, "\x1b[H", 3);     // this escape sequence moves cursor to top left

    draw_rows(&ab);

    // Move cursor to its original position
    // NOTE: cursor coordinates (E.cx, E.cy) are zero-indexed
    // NOTE: cursor positions (used in escape sequences) are one-indexed
    // NOTE: "\x1b[X;YH" moves cursor to position (X, Y)
    char buffer[32];
    snprintf(buffer, sizeof(buffer), "\x1b[%d;%dH", (E.cy - E.rowoff) + 1, (E.cx - E.coloff) + 1);
    ab_append(&ab, buffer, strlen(buffer));

    ab_append(&ab, "\x1b[?25h", 6);  // this escape sequence shows cursor

    // NOTE: buffer has a null terminator in it and don't write it
    write(STDOUT_FILENO, ab.buffer, ab.bufflen);
    ab_free(&ab);
}


/*
-> Renders all visible rows in the editor viewport
-> It doesn't actually write to STDOUT
-> It appends all content to the append buffer
*/
void draw_rows(AppendBuffer *ab) {
    for (int line = 0; line < E.screenrows; line++) {
        ab_append(ab, "\x1b[2K", 4);  // this escape sequence clears current line

        // NOTE: 'line' is zero-indexed and it is the index of a line in the screen
        // NOTE: 'filerow' is also zero-indexed and it is the index of a line in the file
        int filerow = line + E.rowoff;

        // Display content
        if (filerow < E.numlines) {
            char *buffer = get_line_from_rope(E.rope, filerow);

            // 'bufflen': length of buffer to be displayed on screen
            int bufflen = string_length(buffer) - E.coloff;

            // Clamp bufflen
            if (bufflen < 0)
                bufflen = 0;
            if (bufflen > E.screencols)
                bufflen = E.screencols;

            ab_append(ab, &buffer[E.coloff], bufflen);
            free(buffer);
        }

        else {
            // Display welcome message
            if (E.numlines == 0 && line == (E.screenrows / 3)) {
                char welcome[80];
                int welcomelen = snprintf(welcome, sizeof(welcome),
                    "TIM (Text vIM) -- version %s", TIM_VERSION);

                // Truncate message if needed
                if (welcomelen > E.screencols)
                    welcomelen = E.screencols;

                // Add padding
                int padding = (E.screencols - welcomelen) / 2;
                if (padding) {
                    ab_append(ab, "~", 1);
                    padding--;
                }
                while (padding--)
                    ab_append(ab, " ", 1);

                ab_append(ab, welcome, welcomelen);
            }

            // Display empty lines
            else {
                ab_append(ab, "~", 1);
            }
        }

        // Avoid adding newline in the last row
        if (line < E.screenrows - 1)
            ab_append(ab, "\r\n", 2);
    }
}


// Update E.rowoff to scroll up/down
void scroll(void) {
    // Scroll up/down
    if (E.cy < E.rowoff)
        E.rowoff = E.cy;
    else if (E.cy >= E.rowoff + E.screenrows)
        E.rowoff = E.cy - E.screenrows + 1;

    // Scroll left/right
    if (E.cx < E.coloff)
        E.coloff = E.cx;
    else if (E.cx >= E.coloff + E.screencols)
        E.coloff = E.cx - E.screencols + 1;
}
