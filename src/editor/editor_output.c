#include "editor.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>


// Re-draws entire editor screen
void refresh_screen(void) {
    AppendBuffer ab = ABUF_INIT;

    ab_append(&ab, "\x1b[?25l", 6);  // this escape sequence hides cursor
    ab_append(&ab, "\x1b[H", 3);     // this escape sequence moves cursor to top left

    draw_rows(&ab);

    // Move cursor to its original position
    // NOTE: cursor coordinates (E.cx, E.cy) are start from index 0
    // NOTE: cursor positions (used in escape sequences) start from index 1
    char buffer[32];
    snprintf(buffer, sizeof(buffer), "\x1b[%d;%dH", E.cy + 1, E.cx + 1);
    ab_append(&ab, buffer, strlen(buffer));

    ab_append(&ab, "\x1b[?25h", 6);  // this escape sequence shows cursor

    write(STDOUT_FILENO, ab.buffer, ab.len);
    ab_free(&ab);
}


/*
-> Renders all visible rows in the editor viewport
-> It doesn't actually write to STDOUT
-> It appends all content to the append buffer
*/
void draw_rows(AppendBuffer *ab) {
    for (int y = 0; y < E.screenrows; y++) {
        ab_append(ab, "\x1b[2K", 4);  // this escape sequence clears current line

        // Display welcome message
        if (y == (E.screenrows / 3)) {
            char welcome[80];
            int welcomelen = snprintf(welcome, sizeof(welcome),
                "TIM (Text vIM) -- version %s", TIM_VERSION);

            // Truncate welcome message if needed
            if (welcomelen > E.screencols)
                welcomelen = E.screencols;

            // Add padding to welcome message
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

        // Avoid adding newline in the last row
        if (y < E.screenrows - 1)
            ab_append(ab, "\r\n", 2);
    }
}
