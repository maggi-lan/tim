#include "editor.h"

#include "rope.h"
#include "terminal.h"


// Global editor state
EditorState E;


// Initialize global editor state
void init_editor(RopeNode *root) {
    // Set cursor position at top left
    E.cx = 0;
    E.cy = 0;
    E.rowoff = 0;
    E.coloff = 0;

    // Fetch terminal screen dimensions and handle errors
    if (get_window_size(&E.screenrows, &E.screencols) == -1)
        halt("get_window_size");

    E.rope = root;
    E.numlines = (root == NULL) ? 0 : root->newlines + 1;
}
