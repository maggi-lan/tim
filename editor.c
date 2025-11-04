#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MAX(a, b) ((a) > (b) ? (a) : (b))


// NOTE: There are two types of nodes here: internal nodes & leaf nodes
typedef struct RopeNode {
	int weight;     // length of text in left subtree (in internal nodes) or length of string in node (in leaf nodes)
	int total_len;  // total number of characters under the whole subtree
	char *str;      // only for leaf nodes
	int height;     // for AVL rotations
	int newlines;   // count of '\n's in subtree (used by the text cursor)

	struct RopeNode *left;
	struct RopeNode *right;
	struct RopeNode *parent;
} RopeNode;

// NOTE: weight is used for indexing while total_len helps us to calculate weights

/*
# LEAF NODES
> leaf nodes store the text chunks
> left = right = NULL
> weight = strlen(str)
> str = chunk of text (array of characters)

# INTERNAL NODES
> internal nodes don't store the text chunks
> left != NULL and right != NULL
> weight = total length of text in all the leaf nodes from the left subtree
> str = NULL
*/


// Helper functions
bool is_leaf(RopeNode *node);
int node_height(RopeNode *node);
int string_length(char *str);
void update_metadata(RopeNode *node);
void print_text(RopeNode *node);




// Returns true if a given node is a leaf node, else false
bool is_leaf(RopeNode *node) {
	// Edge case when node is NULL
	if (node == NULL)
		return false;

	// Returns true when no children is available, else false
	if ((node->left == NULL) && (node->right == NULL))
		return true;
	else
		return false;
}


// Returns height of a given node (returns 0 if NULL pointer is passed)
int node_height(RopeNode *node) {
	if (node != NULL)
		return node->height;
	else
		return 0;  // returns 0 if node is NULL
}


int string_length(char *str) {
	if (str != NULL)
		return strlen(str);
	else
		return 0;  // returns 0 if node is NULL
}


// Recomputes total_len, weight, height and newlines
void update_metadata(RopeNode *node) {
	// Edge case when node is NULL
	if (node == NULL)
		return;

	// CASE 1: node = leaf node
	if (is_leaf(node)) {
		node->total_len = string_length(node->str);
		node->weight = node->total_len;  // weight of a leaf node = strlen(node->str)

		node->height = 1;  // height of a leaf node is 1

		// Iteratively search through the string to count newlines
		node->newlines = 0;
		if (node->str != NULL) {
			for (int i = 0; (node->str)[i] != '\0'; i++)
				if ((node->str)[i] == '\n')
					(node->newlines)++;
		}
	}

	// CASE 2: node = internal node
	else {
		// total_len of internal node = sum of total_len of left & right nodes
		int left_len = node->left ? node->left->total_len : 0;
		int right_len = node->right ? node->right->total_len : 0;
		node->total_len = left_len + right_len;

		node->weight = left_len;  // Weight of internal node = total length of characters in the left subtree

		// Calculates the height based on the heights of the node's children
		node->height = 1 + MAX(node_height(node->left), node_height(node->right));

		// newline = sum of number of newlines in left & right nodes
		node->newlines = 0;
		if (node->left)
			node->newlines += node->left->newlines;
		if (node->right)
			node->newlines += node->right->newlines;
	}
}


// Prints all the text in a rope using recursion
// Useful for debugging
void print_text(RopeNode *node) {
	// Return void if node is NULL
	if (node == NULL)
		return;
	
	// CASE 1: node = leaf node
	if (is_leaf(node))
		printf("%s", node->str);

	// CASE 2: node = internal node
	else {
		print_text(node->left);   // recurse to the left subtree
		print_text(node->right);  // recurse to the right subtree
	}
}




int main(int argc, char **argv) {
	return 0;
}
