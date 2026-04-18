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
    // NOTE: render/cursor coordinates (E.rx, E.cy) are zero-indexed
    // NOTE: use E.rx for horizontal cursor position
    // NOTE: cursor positions (used in escape sequences) are one-indexed
    // NOTE: "\x1b[X;YH" moves cursor to position (X, Y)
    char buffer[32];
    snprintf(buffer, sizeof(buffer), "\x1b[%d;%dH", (E.cy - E.rowoff) + 1, (E.rx - E.coloff) + 1);
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
            // Fetch entire line
            int rawlen = get_line_length(E.rope, filerow);
            char *raw = get_line_segment_from_rope(E.rope, filerow, 0, rawlen);

            // Render into expanded buffer (if tabs are present in the line)
            if (raw) {
                int maxlen = MIN((rawlen * TAB_WIDTH) + 1, E.screencols + 1);
                char *render = calloc(1, maxlen);  // render will only hold characters that will be displayed in screen
                // Error handling
                if (!render)
                    halt("draw_rows");

                int renlen = 0;
                int rx = 0;

                // Walk through raw line and expand the tabs
                for (int i = 0; i < rawlen && rx < E.coloff + E.screencols; i++) {
                    if (raw[i] == '\n')
                        break;

                    if (raw[i] == '\t') {
                        int spaces = TAB_WIDTH - (rx % TAB_WIDTH);  // spaces required to reach next tab stop
                        while (spaces--) {
                            // Add spaces to 'render' only if it is going to be displayed in the screen
                            if (rx >= E.coloff && rx < E.coloff + E.screencols)
                                render[renlen++] = ' ';
                            rx++;
                        }
                    }
                    else {
                        // Add characters to 'render' only if it is going to be displayed in the screen
                        if (rx >= E.coloff && rx < E.coloff + E.screencols)
                            render[renlen++] = raw[i];
                        rx++;
                    }
                }

                ab_append(ab, render, renlen);
                free(render);
                free(raw);
            }
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


// Update E.rowoff/E.coloff to scroll vertically/horizontally
void scroll(void) {
    // Scroll vertically
    if (E.cy < E.rowoff)
        E.rowoff = E.cy;
    else if (E.cy >= E.rowoff + E.screenrows)
        E.rowoff = E.cy - E.screenrows + 1;

    // Scroll horizontally
    if (E.rx < E.coloff)
        E.coloff = E.rx;
    else if (E.rx >= E.coloff + E.screencols)
        E.coloff = E.rx - E.screencols + 1;
}
