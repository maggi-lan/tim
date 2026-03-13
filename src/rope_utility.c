#include "../include/rope.h"

#include <stdlib.h>

/*
-> Returns the character at a given index recursively
-> Usage: pass in the root node and zero-based index
-> Returns null character if index goes out of bounds
*/
char char_at(RopeNode *node, int idx) {
    // Edge case
    if (node == NULL)
        return '\0';

    // BASE CASE: leaf nodes -> return the character at 'idx'
    if (is_leaf(node)) {
        // Out of bounds handling
        if (idx < 0 || idx >= node->weight)
            return '\0';

        return node->str[idx];
    }

    // CASE-1: 'idx' lies in the left subtree -> recurse to the left subtree
    if (idx < node->weight)
        return char_at(node->left, idx);

    // CASE-2: 'idx' lies in the right subtree -> recurse to the right subtree
    else
        return char_at(node->right, idx - node->weight);  // adjust index
}


/*
-> Returns the text index (zero-based) of the Nth newline character (zero-based) recursively
-> Returns -1 if newline_idx is invalid
*/
int find_newline_pos(RopeNode *node, int newline_idx, int offset) {
    // Edge case
    if (node == NULL)
        return -1;

    // BASE CASE: leaf nodes -> iteratively check for the newline in the text chunk
    if (is_leaf(node)) {
        int newline_count = 0;
        for (int i = 0; node->str[i] != '\0'; i++) {
            if (node->str[i] == '\n') {
                if (newline_count == newline_idx)
                    return offset + i;  // offset = index of the first character of the text chunk
                newline_count++;
            }
        }

        return -1;  // execution reaches here if there is no newline in the text chunk
    }

    int left_newlines = node->left ? node->left->newlines : 0;

    // CASE-1: target newline is in left subtree -> recurse to the left subtree
    if (newline_idx < left_newlines)
        return find_newline_pos(node->left, newline_idx, offset);

    // CASE-2: target newline is in right subtree -> recurse to the right subtree
    else
        // Adjust newline_idx and offset
        return find_newline_pos(node->right, newline_idx - left_newlines, offset + node->weight);
}


/*
-> Returns the starting position (zero-based character index) of Nth line (zero-based)
-> Returns 0 in case of error or negative line
-> Returns the index after the last character in the text if 'line' exceeds the number of available lines
*/
int get_line_start(RopeNode *root, int line) {
    // Edge case
    if (root == NULL || line < 0)
        return 0;

    if (root->total_len == 0)
        return 0;

    // Line 0 starts at index 0
    if (line == 0)
        return 0;

    // Line N starts at position after (N-1)th newline
    int newline_pos = find_newline_pos(root, line - 1, 0);

    if (newline_pos == -1)
        return root->total_len;  // Line doesn't exist, return (last_index + 1)

    return newline_pos + 1;  // Position after the newline
}


/*
-> Returns the length of a specific line (excluding newline)
-> Pass in root and line index (zero-based)
-> Returns zero in case of error
*/
int get_line_length(RopeNode *root, int line) {
    // Edge case
    if (root == NULL)
        return 0;

    int start = get_line_start(root, line);  // starting position of the line

    // If 'line' exceeds the total number of available lines
    if (start >= root->total_len)
        return 0;

    int end;  // position after the last character of the line

    // If the 'line' is the last line and it doesn't have a newline character at the end
    if (line >= root->newlines) {
        end = root->total_len;
    }
    else {
        int newline_pos = find_newline_pos(root, line, 0);
        end = newline_pos;
    }

    return end - start;
}

// Returns the total number of lines in a rope
int count_total_lines(RopeNode *root) {
    if (root == NULL || root->total_len == 0)
        return 1;  // treat empty buffers as one empty line

    // Ex: "Hello" -> 0 newlines -> 1 line
    // Ex: "Hello\nThere" -> 1 newline -> 2 lines
    // Ex: "Hello\n" -> 1 newline -> 2 lines (last line is an empty line)
    return root->newlines + 1;
}
