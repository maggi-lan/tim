#include "editor.h"

#include "../terminal/terminal.h"


EditorState E;


// Initialize global editor state
void init_editor(void) {
    // Set cursor position at top left
    E.cx = 0;
    E.cy = 0;

    // Fetch terminal screen dimensions and handle error
    if (get_window_size(&E.screenrows, &E.screencols) == -1)
        halt("get_window_size");
}
