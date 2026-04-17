#include "editor.h"

#include <stdlib.h>


// Convert cursor coordinate (cx) at Nth line (zero-indexed) to it's rendered column coordinate (rx)
int cx_to_rx(int line, int cx) {
    int rx = 0;
    char *buffer = get_line_segment_from_rope(E.rope, line, 0, cx);
    // Edge case
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


// Convert rendered column coordinate (rx) at Nth line (zero-indexed) to it's cursor coordinate (cx)
int rx_to_cx(int line, int rx) {
    // Fetch entire line
    int cur_rx = 0;
    int linelen = get_line_length(E.rope, line);
    char *buffer = get_line_segment_from_rope(E.rope, line, 0, linelen);
    // Edge case
    if (!buffer)
        return 0;

    // Walk through the line and compute rx
    for (int cx = 0; cx < linelen; cx++) {
        if (buffer[cx] == '\t')
            cur_rx += TAB_WIDTH - (cur_rx % TAB_WIDTH);
        else
            cur_rx++;

        // Return required 'cx' when input 'rx' is reached
        if (cur_rx > rx) {
            free(buffer);
            return cx;
        }
    }

    free(buffer);
    return linelen;
}
