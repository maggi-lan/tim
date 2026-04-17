#include "editor.h"

#include <stdlib.h>


// Convert cursor coordinate (cx) in a line to it's rendered column index (rx)
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
