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
    int total_len = E.rope ? E.rope->total_len : 0;
    if (total_len == 0)
        return;

    int idx = get_rope_idx_from_cursor();

    if (E.cx > 0)
        move_cursor(ARROW_LEFT);
    else if (E.cy > 0) {
        E.cy--;
        E.cx = get_line_length(E.rope, E.cy);
        E.rx = cx_to_rx(E.cy, E.cx);
        E.snapx = E.rx;
        E.numlines--;
    }

    E.rope = delete_at(E.rope, idx - 1, 1);
}


// Deletes the character at the current cursor position from the rope
void delete_char_at_cursor(void) {
    int total_len = E.rope ? E.rope->total_len : 0;
    if (total_len == 0)
        return;

    int idx = get_rope_idx_from_cursor();
    if (idx >= total_len)
        return;

    int len = get_line_length(E.rope, E.cy);

    if (E.cx == len) {
        // In normal mode, 'x' does not delete the newline character
        if (E.mode == MODE_NORMAL)
            return;

        // In insert mode, DEL deletes the newline character to merge with the next line
        E.rope = delete_at(E.rope, idx, 1);
        E.numlines = count_total_lines(E.rope);
    }
    else {
        E.rope = delete_at(E.rope, idx, 1);
        E.numlines = count_total_lines(E.rope);

        int new_len = get_line_length(E.rope, E.cy);
        if (E.mode == MODE_NORMAL && E.cx == new_len && E.cx > 0)
            move_cursor(ARROW_LEFT);
    }
}
