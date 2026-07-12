#include "editor.h"

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "terminal.h"
#include "rope.h"


// Rebuilds and redraws the entire editor screen in a single buffered write.
void refresh_screen(void) {
    scroll();

    // Accumulate all screen output to 'ab' before writing it to STDOUT in one go
    AppendBuffer ab = ABUF_INIT;

    ab_append(&ab, "\x1b[?25l", 6);  // hides cursor to prevent flickering while redrawing the screen
    ab_append(&ab, "\x1b[H", 3);     // moves cursor to the top left of the screen before redrawing the screen

    draw_rows(&ab);
    draw_status_bar(&ab);
    draw_message_bar(&ab);

    // NOTE: render/cursor coordinates (E.rx, E.cy) are 0-indexed
    // NOTE: use E.rx for horizontal cursor position
    // NOTE: cursor positions (used in escape sequences) are 1-indexed
    // NOTE: "\x1b[X;YH" moves cursor to position (X, Y)

    // Restore cursor to the editor's logical position
    char buffer[32];
    snprintf(buffer, sizeof(buffer), "\x1b[%d;%dH", (E.cy - E.rowoff) + 1, (E.rx - E.coloff) + 1);
    ab_append(&ab, buffer, strlen(buffer));

    ab_append(&ab, "\x1b[?25h", 6);  // displays the cursor after redrawing the screen

    // Flush the append buffer to STDOUT in one go
    write(STDOUT_FILENO, ab.buffer, ab.bufflen);
    ab_free(&ab);
}


/*
-> Renders all visible rows in the editor's viewport
-> It doesn't actually write to STDOUT
-> It appends all content to the append buffer
*/
void draw_rows(AppendBuffer *ab) {
    for (int line = 0; line < E.screenrows; line++) {
        ab_append(ab, "\x1b[2K", 4);    // clears current line to prevent artifacts from previous content
        int filerow = line + E.rowoff;  // 0-indexed

        if (filerow < E.numlines)
            draw_line(ab, filerow);
        else {
            // Display welcome message
            if (E.numlines == 0 && line == (E.screenrows / 3)) {
                char welcome[80];
                int welcomelen = snprintf(welcome, sizeof(welcome),
                    "TIM (Text vIM) -- version %s", TIM_VERSION);

                if (welcomelen > E.screencols)
                    welcomelen = E.screencols;

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
            else
                ab_append(ab, "~", 1);
        }

        // Add newline after every row
        ab_append(ab, "\r\n", 2);
    }
}


/*
-> Renders a single row of text (identified by 0-indexed 'filerow') to the screen
-> Expands tabs to spaces and appends visible portions of the line to an append buffer
*/
void draw_line(AppendBuffer *ab, int filerow) {
    // Entire line is fetched because we need to expand tabs and calculate the rendered length of the line
    int rawlen = get_line_length(E.rope, filerow);
    char *raw = get_line_segment_from_rope(E.rope, filerow, 0, rawlen);

    if (raw) {
        int maxlen = MIN((rawlen * TAB_WIDTH) + 1, E.screencols + 1);
        char *render = calloc(1, maxlen);  // render will only hold characters that will be displayed in screen
        if (!render)
            halt("draw_line");

        int renlen = 0;
        int rx = 0;

        for (int i = 0; i < rawlen && rx < E.coloff + E.screencols; i++) {
            if (raw[i] == '\n')
                break;

            // Expand tabs
            if (raw[i] == '\t') {
                int spaces = TAB_WIDTH - (rx % TAB_WIDTH);  // spaces required to reach next tab stop
                while (spaces--) {
                    if (rx >= E.coloff && rx < E.coloff + E.screencols)
                        render[renlen++] = ' ';
                    rx++;
                }
            }
            else {
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


// Update E.rowoff/E.coloff to scroll vertically/horizontally
void scroll(void) {
    // Scroll vertically up/down
    if (E.cy < E.rowoff)
        E.rowoff = E.cy;
    else if (E.cy >= E.rowoff + E.screenrows)
        E.rowoff = E.cy - E.screenrows + 1;

    // Scroll horizontally left/right
    if (E.rx < E.coloff)
        E.coloff = E.rx;
    else if (E.rx >= E.coloff + E.screencols)
        E.coloff = E.rx - E.screencols + 1;
}


/*
-> Renders status bar at the bottom of the viewport
-> It doesn't actually write to STDOUT
-> It appends all content to the append buffer
*/
void draw_status_bar(AppendBuffer *ab) {
    ab_append(ab, "\x1b[7m", 4);  // white background, black foreground

    char *mode;
    if (E.mode == MODE_NORMAL)
        mode = "NORMAL";
    else if (E.mode == MODE_INSERT)
        mode = "INSERT";
    else if (E.mode == MODE_COMMAND)
        mode = "COMMAND";

    char status[80];
    int len = snprintf(status, sizeof(status), "  %s  |  %.20s  |  %d lines", mode, E.filename, E.numlines);

    if (len > E.screencols)
        len = E.screencols;

    ab_append(ab, status, len);

    char rstatus[80];
    int rlen = snprintf(rstatus, sizeof(rstatus), "%d/%d", E.cy + 1, E.numlines);

    while (len < E.screencols) {
        // Right side of the status bar
        if (E.screencols - len == rlen) {
            ab_append(ab, rstatus, rlen);
            break;
        }

        // Whitespaces
        else {
            ab_append(ab, " ", 1);
            len++;
        }
    }

    ab_append(ab, "\x1b[m", 3);  // resets colors to default
    ab_append(ab, "\r\n", 2);
}


/*
-> Sets custom status message by updating the editor state
-> Behaves like printf()
-> Accepts variable number of arguements
*/
void set_status_message(const char *fmt, ...) {
    // Create custom printf() like facility
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(E.statusmsg, sizeof(E.statusmsg), fmt, ap);
    va_end(ap);
    E.statusmsg_time = time(NULL);
}

/*
-> Renders message bar at the bottom of the viewport
-> It doesn't actually write to STDOUT
-> It appends all content to the append buffer
*/
void draw_message_bar(AppendBuffer *ab) {
    ab_append(ab, "\x1b[K", 3);  // clears current line to prevent artifacts from previous content

    int msglen = strlen(E.statusmsg);
    if (msglen > E.screencols)
        msglen = E.screencols;

    // Display message only if it's less than 5 seconds old
    if (msglen && time(NULL) - E.statusmsg_time < 5)
        ab_append(ab, E.statusmsg, msglen);
}
