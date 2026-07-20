#include "editor.h"

#include "rope.h"

void insert_at_cursor(char ch) {
    char str[2];
    str[0] = ch;
    str[1] = '\0';

    E.rope = insert_at(E.rope, get_rope_idx_from_cursor(), str);
}

void insert_char_at_cursor(char ch) {
    insert_at_cursor(ch);

    if (ch == '\t')
        E.rx += TAB_WIDTH - (E.rx % TAB_WIDTH);
    else
        E.rx++;
    E.cx++;
    E.snapx = E.rx;
}
