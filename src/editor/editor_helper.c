#include "editor.h"

#include <stdlib.h>


// Converts a cursor column (cx) on the specified line (0-indexed) to its rendered column (rx)
int cx_to_rx(int line, int cx) {
    int rx = 0;
    char *buffer = get_line_segment_from_rope(E.rope, line, 0, cx);
    if (!buffer)
        return 0;

    for (int i = 0; i < cx; i++) {
        if (buffer[i] == '\t')
            rx += TAB_WIDTH - (rx % TAB_WIDTH);  // snaps to next tab stop
        else
            rx++;
    }

    free(buffer);
    return rx;
}

/*
-> Converts a rendered column (rx) on the specified line to its cursor column (cx)
-> Returns the 'cx' of the end of the line if 'rx' exceeds the rendered width
*/
int rx_to_cx(int line, int rx) {
    // Fetch entire line because we need to expand tabs
    int linelen = get_line_length(E.rope, line);
    char *buffer = get_line_segment_from_rope(E.rope, line, 0, linelen);
    if (!buffer)
        return 0;
    int cur_rx = 0;

    for (int cx = 0; cx < linelen; cx++) {
        if (buffer[cx] == '\t')
            cur_rx += TAB_WIDTH - (cur_rx % TAB_WIDTH);
        else
            cur_rx++;

        if (cur_rx > rx) {
            free(buffer);
            return cx;
        }
    }

    free(buffer);

    return linelen;  // clamp to the end of the line
}
