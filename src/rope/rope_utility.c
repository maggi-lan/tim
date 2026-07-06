#include "rope.h"

#include <stdlib.h>

#include "terminal.h"


/*
-> Returns the index (0-based) of a given newline (identified by 0-based newline_idx) from a rope
-> Returns -1 if newline_idx is out of range
*/
int find_newline_pos(RopeNode *node, int newline_idx, int offset) {
    if (node == NULL)
        return -1;

    // BASE CASE
    if (is_leaf(node)) {
        int newline_count = 0;
        for (int i = 0; node->str[i] != '\0'; i++) {
            if (node->str[i] == '\n') {
                if (newline_count == newline_idx)
                    return offset + i;  // offset = index of the first character of the text chunk
                newline_count++;
            }
        }

        return -1;
    }

    int left_newlines = node->left ? node->left->newlines : 0;

    // CASE-1: target newline is in the left subtree
    if (newline_idx < left_newlines)
        return find_newline_pos(node->left, newline_idx, offset);

    // CASE-2: target newline is in the right subtree
    else
        return find_newline_pos(node->right, newline_idx - left_newlines, offset + node->weight);  // adjust newline_idx and offset
}


/*
-> Returns the starting index (0-based) of a given line (0-based) from a rope
-> Returns 0 if the rope is empty or the line number is negative
-> Returns the index after the last character if the line does not exist
*/
int get_line_start(RopeNode *root, int line) {
    if (root == NULL || line < 0)
        return 0;

    if (root->total_len == 0)
        return 0;

    // Line 0 starts at index 0
    if (line == 0)
        return 0;

    // Line N starts at the position after (N-1)th newline
    int newline_pos = find_newline_pos(root, line - 1, 0);

    if (newline_pos == -1)
        return root->total_len;  // index after the last character

    return newline_pos + 1;
}


/*
-> Returns the length of a specific line (excluding newline) from a rope
-> Pass in root and line index (0-based)
-> Returns zero in case of error
*/
int get_line_length(RopeNode *root, int line) {
    if (root == NULL)
        return 0;

    int start = get_line_start(root, line);
    if (start >= root->total_len)
        return 0;

    int end;
    if (line >= root->newlines)  // last line
        end = root->total_len;
    else
        end = find_newline_pos(root, line, 0);

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


/*
-> Returns a segment of text at the Nth line (0-indexed) from a rope
-> 'start': starting index (0-indexed) of the segment in the line
-> 'maxlen': maximum length of the segment
-> Resultant string will exclude newlines
*/
char *get_line_segment_from_rope(RopeNode *root, int line, int start, int maxlen) {
    if (root == NULL)
        return NULL;

    int line_len = get_line_length(root, line);
    if (start >= line_len || maxlen == 0)
        return NULL;

    int len = MIN(maxlen, line_len - start);
    int lstart = get_line_start(root, line) + start;
    int offset;

    RopeNode *leaf = leaf_at(root, lstart, &offset);
    if (!leaf && len != 0)
        halt("get_line_segment_from_rope");

    char *result = calloc(len + 1, 1);
    if (!result)
        halt("get_line_segment_from_rope");

    // Walk through the leaves to fetch segment
    for (int idx = 0; idx < len; idx++) {
        if (offset >= leaf->weight) {
            leaf = next_leaf(leaf);
            if (!leaf)
                halt("get_line_segment_from_rope");

            offset = 0;
        }

        result[idx] = leaf->str[offset];
        offset++;
    }

    return result;
}


/*
-> Returns the leaf containing a character at a given index
-> Stores the character's offset within the leaf in 'offset'
-> Returns NULL if the index is out of range
*/
RopeNode *leaf_at(RopeNode *node, int idx, int *offset) {
    if (node == NULL)
        return NULL;

    // BASE CASE
    if (is_leaf(node)) {
        if (idx < 0 || idx >= node->weight)
            return NULL;

        *offset = idx;
        return node;
    }

    // CASE-1: character lies in the left subtree
    if (idx < node->weight)
        return leaf_at(node->left, idx, offset);

    // CASE-2: character lies in the right subtree
    else
        return leaf_at(node->right, idx - node->weight, offset);  // adjust index
}


/*
-> Returns the next leaf in inorder traversal given a leaf node in a rope
-> Returns NULL if no next leaf exists
*/
RopeNode *next_leaf(RopeNode *leaf) {
    if (!leaf || !is_leaf(leaf))
        return NULL;

    RopeNode *next = successor_node(leaf);
    while (next != NULL && !is_leaf(next))
        next = successor_node(next);

    return next;
}


/*
-> Returns the inorder successor of a node in a rope
-> Returns NULL if successor doesn't exist
*/
RopeNode *successor_node(RopeNode *node) {
    if (!node)
        return NULL;

    // CASE-A: 'node' has a right child -> successor is the leftmost node in the right subtree
    if (node->right) {
        node = node->right;
        while (node->left != NULL)
            node = node->left;

        return node;
    }

    // CASE-B: 'node' has no right child -> walk up the rope until you came from a left child
    else {
        RopeNode *parent = node->parent;
        while (parent != NULL && parent->left != node) {
            node = parent;
            parent = node->parent;
        }

        return parent;
    }
}
