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

    // Fetch terminal screen dimensions and handle errors
    if (get_window_size(&E.screenrows, &E.screencols) == -1)
        halt("get_window_size");

    E.rope = root;
    if (root == NULL)
        E.numlines = 0;
    else
        E.numlines = root->newlines + 1;
}
