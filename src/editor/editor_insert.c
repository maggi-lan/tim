#include "editor.h"

#include "rope.h"

// Inserts any character at the current cursor position to the rope
void insert_at_cursor(char ch) {
    char str[2];
    str[0] = ch;
    str[1] = '\0';

    E.rope = insert_at(E.rope, get_rope_idx_from_cursor(), str);
}

// Inserts a printable character at the current cursor position
void insert_char_at_cursor(char ch) {
    insert_at_cursor(ch);

    if (ch == '\t')
        E.rx += TAB_WIDTH - (E.rx % TAB_WIDTH);
    else
        E.rx++;
    E.cx++;
    E.snapx = E.rx;
}

// Inserts a newline character at the current cursor position
void insert_newline_at_cursor(void) {
    insert_at_cursor('\n');

    E.cy++;
    E.cx = 0;
    E.rx = 0;
    E.snapx = E.rx;
    E.numlines++;
}
