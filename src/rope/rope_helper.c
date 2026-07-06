#include "rope.h"

#include <stdlib.h>
#include <string.h>

#include "terminal.h"


// Returns true if the node has no children
bool is_leaf(RopeNode *node) {
	if (node == NULL)
		return false;

	if ((node->left == NULL) && (node->right == NULL))
		return true;
	else
		return false;
}


/*
-> Returns the height of the subtree rooted at 'node'
-> Returns zero if subtree doesn't exist
*/
int node_height(RopeNode *node) {
	if (node != NULL)
		return node->height;
	else
		return 0;
}


/*
-> Returns the length of a string
-> Returns zero if NULL string is passed
*/
int string_length(const char *str) {
	if (str != NULL)
		return strlen(str);
	else
		return 0;
}


/*
-> Returns the number '\n's in a string
-> Returns zero if NULL string is passed
*/
int count_newlines(const char *str) {
	if (str == NULL)
		return 0;

	int count = 0;
	for (int i = 0; str[i] != '\0'; i++)
		if (str[i] == '\n')
			count++;

	return count;
}


// Recomputes total_len, weight, height and newlines of a node
void update_metadata(RopeNode *node) {
	if (node == NULL)
		return;

	// CASE 1: node = leaf node
	if (is_leaf(node)) {
		node->total_len = string_length(node->str);
		node->weight = node->total_len;              // weight of a leaf node = strlen(node->str)
		node->height = 1;
		node->newlines = count_newlines(node->str);
	}

	// CASE 2: node = internal node
	else {
		int left_len = node->left ? node->left->total_len : 0;
		int right_len = node->right ? node->right->total_len : 0;
		node->total_len = left_len + right_len;

		node->weight = left_len; // weight of internal node = total length of characters in the left subtree

		node->height = 1 + MAX(node_height(node->left), node_height(node->right));

		node->newlines = 0;
		if (node->left)
			node->newlines += node->left->newlines;
		if (node->right)
			node->newlines += node->right->newlines;
	}
}


// Returns a newly allocated copy of a given string
char *string_copy(const char *src) {
	char *dst = malloc(string_length(src) + 1);  // +1 for null character
	if (dst == NULL)
		halt("string_copy");

	strcpy(dst, src);
	return dst;
}


// Returns a newly allocated copy of the first 'n' characters of a given string
char *substr_copy(const char *start, int n) {
	if (start == NULL)
		return NULL;
	if (n > string_length(start)) {
		n = string_length(start);
	}

    char *dst = malloc(n + 1);  // +1 for null character
    if (dst == NULL) {
		halt("substr_cpy");
		exit(EXIT_FAILURE);
	}

	strncpy(dst, start, n);
	dst[n] = '\0';           // Add null terminator
	return dst;
}
