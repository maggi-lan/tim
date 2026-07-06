#ifndef ROPE_H
#define ROPE_H

#include <stdbool.h>

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define CHUNK_SIZE 256


/*
-> RopeNode represents a node in a rope
-> A rope is a binary tree used as a text buffer
-> Only leaf nodes store text chunks
-> Refer https://en.wikipedia.org/wiki/Rope_(data_structure) for more details about ropes
*/
typedef struct RopeNode {
	int weight;     // length of text in the left subtree (in internal nodes) or length of text chunk in the node (in leaf nodes)
	int total_len;  // total number of characters under the subtree rooted at this node
	char *str;      // contains a text chunk (only in leaf nodes)
	int height;     // height of the subtree rooted at this node (used in AVL rotations)
	int newlines;   // count of '\n's in the subtree rooted at this node (used by the text cursor)

	struct RopeNode *left;
	struct RopeNode *right;
	struct RopeNode *parent;

    // NOTE: 'weight' helps in O(log n) indexing while 'total_len' helps to calculate weights
} RopeNode;

/*
# LEAF NODES
- left = right = NULL
- str = chunk of text
- weight = strlen(str)

# INTERNAL NODES
- at least one child is not NULL
- str = NULL
- weight = total length of text in all the leaf nodes from the left subtree
*/


// Core functions
RopeNode *create_leaf(const char *text);
RopeNode *concat(RopeNode *left, RopeNode *right);
void split(RopeNode *root, int idx, RopeNode **left, RopeNode **right);
RopeNode *build_rope(const char *text);
RopeNode *insert_at(RopeNode *root, int idx, const char *text);
RopeNode *delete_at(RopeNode *root, int start, int len);
void free_rope(RopeNode *root);

// AVL balancing
int get_skew(RopeNode *node);
RopeNode *rotate_right(RopeNode *node);
RopeNode *rotate_left(RopeNode *node);
RopeNode *rebalance(RopeNode *node);

// Helper functions
bool is_leaf(RopeNode *node);
int node_height(RopeNode *node);
int string_length(const char *str);
int count_newlines(const char *str);
void update_metadata(RopeNode *node);
char *string_copy(const char *src);
char *substr_copy(const char *start, int n);

// Utility functions
int find_newline_pos(RopeNode *node, int newline_idx, int offset);
int get_line_start(RopeNode *root, int line);
int get_line_length(RopeNode *root, int line);
int count_total_lines(RopeNode *root);
char *get_line_segment_from_rope(RopeNode *root, int line, int start, int maxlen);
RopeNode *leaf_at(RopeNode *node, int idx, int *offset);
RopeNode *next_leaf(RopeNode *node);
RopeNode *successor_node(RopeNode *node);


#endif
