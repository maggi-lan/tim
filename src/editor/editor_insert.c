#include "editor.h"

#include "rope.h"
#include "terminal.h"


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
    move_cursor(ARROW_RIGHT);
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


// Deletes the character before the current cursor position from the rope
void delete_char_before_cursor(void) {
    if (E.cx == 0 && E.cy == 0)
        return;

    E.rope = delete_at(E.rope, get_rope_idx_from_cursor() - 1, 1);

    if (E.cx > 0)
        move_cursor(ARROW_LEFT);
    else if (E.cy > 0) {
        E.cy--;
        E.cx = get_line_length(E.rope, E.cy);
        E.rx = cx_to_rx(E.cy, E.cx);
        E.snapx = E.rx;
        E.numlines--;
    }
}


void delete_char_at_cursor(void) {
    if (E.cx == 0 && E.cy == 0)
        return;

    if (get_line_length(E.rope, E.cy) == 0 && E.cy == E.numlines - 1)
        return;

    E.rope = delete_at(E.rope, get_rope_idx_from_cursor(), 1);
}
