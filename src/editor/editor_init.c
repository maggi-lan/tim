#include "editor.h"

#include <string.h>

#include "rope.h"
#include "terminal.h"


// Global editor state
EditorState E;


// Initialize global editor state
void init_editor(RopeNode *root, const char *filename) {
    // Set cursor position at top left
    E.cx = 0;
    E.cy = 0;
    E.snapx = 0;
    E.rowoff = 0;
    E.coloff = 0;
    E.filename = strdup(filename);

    if (get_window_size(&E.screenrows, &E.screencols) == -1)
        halt("get_window_size");
    E.screenrows -= 2;  // leave space at the bottom for status/message bar

    E.statusmsg[0] = '\0';
    E.statusmsg_time = 0;

    E.mode = MODE_NORMAL;

    E.rope = root;
    E.numlines = (root == NULL) ? 1 : root->newlines + 1;
}
