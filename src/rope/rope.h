#ifndef ROPE_H
#define ROPE_H

#include <stdbool.h>

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define CHUNK_SIZE 64


/*
-> RopeNode represents a node in a binary tree based data structure called rope
-> Ropes are used to implement text buffers
-> Ropes have two types of nodes: internal nodes & leaf nodes
-> Only leaf nodes contain text chunks
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
} RopeNode;

// NOTE: 'weight' helps in indexing while 'total_len' helps to calculate weights

/*
# LEAF NODES
- leaf nodes store the text chunks
- left = right = NULL
- str = chunk of text
- weight = strlen(str)

# INTERNAL NODES
- internal nodes don't store the text chunks
- left != NULL and right != NULL
- str = NULL
- weight = total length of text in all the leaf nodes from the left subtree
*/


// Rope functions
RopeNode *create_leaf(char *text);
RopeNode *concat(RopeNode *left, RopeNode *right);
void split(RopeNode *root, int idx, RopeNode **left, RopeNode **right);
RopeNode *build_rope(char *text);
RopeNode *insert_at(RopeNode *root, int idx, char *text);
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
int string_length(char *str);
int count_newlines(char *str);
void update_metadata(RopeNode *node);
char *string_copy(char *src);
char *substr_copy(char *start, int n);

// Utility functions
char char_at(RopeNode *root, int idx);
int find_newline_pos(RopeNode *node, int newline_idx, int offset);
int get_line_start(RopeNode *root, int line);
int get_line_length(RopeNode *root, int line);
int count_total_lines(RopeNode *root);
char *get_line_from_rope(RopeNode *root, int line);


#endif
