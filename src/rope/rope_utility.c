#include "rope.h"

#include <stdlib.h>

#include "terminal.h"


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


/*
-> Returns the string of text at line 'line' (zero-indexed)
-> Resultant string will exclude newlines
*/
char *get_line_from_rope(RopeNode *root, int line) {
    // Edge case
    if (root == NULL)
        return NULL;

    int len = get_line_length(root, line);
    int start = get_line_start(root, line);
    int offset;

    RopeNode *leaf = leaf_at(root, start, &offset);
    // Error handling
    if (!leaf && len != 0)
        halt("get_line_from_rope");

    char *result = calloc(len + 1, 1);
    // Error handling
    if (!result)
        halt("get_line_from_rope");

    for (int idx = 0; idx < len; idx++) {
        if (offset >= leaf->weight) {
            leaf = next_leaf(leaf);
            // Error handling
            if (!leaf)
                halt("get_line_from_rope");

            offset = 0;
        }

        result[idx] = leaf->str[offset];
        offset++;
    }

    return result;
}


/*
-> Returns the leaf node containing the character at a given index using recursion
-> Also updates 'offset' with the local offset of the character within the node
-> Usage: pass in the root node, zero-based index and a pointer to the variable storing the offset
-> Returns NULL in case of error
*/
RopeNode *leaf_at(RopeNode *node, int idx, int *offset) {
    // Edge case
    if (node == NULL)
        return NULL;

    // BASE CASE: leaf nodes -> return the character at 'idx'
    if (is_leaf(node)) {
        // Out of bounds handling
        if (idx < 0 || idx >= node->weight)
            return NULL;

        *offset = idx;

        return node;
    }

    // CASE-1: 'idx' lies in the left subtree -> recurse to the left subtree
    if (idx < node->weight)
        return leaf_at(node->left, idx, offset);

    // CASE-2: 'idx' lies in the right subtree -> recurse to the right subtree
    else
        return leaf_at(node->right, idx - node->weight, offset);  // adjust index
}


/*
-> Returns the next leaf in a tree
-> Returns NULL in case of error
*/
RopeNode *next_leaf(RopeNode *leaf) {
    // Edge case
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
    // Edge case
    if (!node)
        return NULL;

    // CASE-A: 'node' has a right child
    if (node->right) {
        // Go to the right child and go left as much as possible
        node = node->right;
        while (node->left != NULL)
            node = node->left;

        return node;
    }

    // CASE-B: 'node' has no right child
    else {
        // Walk up the rope until you came from a left child
        RopeNode *parent = node->parent;
        while (parent != NULL && parent->left != node) {
            node = parent;
            parent = node->parent;
        }

        return parent;
    }
}
