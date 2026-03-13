#include "../include/rope.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


// Returns true if the node has no children
bool is_leaf(RopeNode *node) {
	// Edge case
	if (node == NULL)
		return false;

	if ((node->left == NULL) && (node->right == NULL))
		return true;
	else
		return false;
}


/*
-> Returns the height of the subtree rooted at the node
-> Returns zero if NULL pointer is passed
*/
int node_height(RopeNode *node) {
	if (node != NULL)
		return node->height;
	else
		return 0;
}


/*
-> Returns the length of the string
-> Returns zero if NULL string is passed
*/
int string_length(char *str) {
	if (str != NULL)
		return strlen(str);
	else
		return 0;
}


/*
-> Returns the number '\n's in the string
-> Returns zero if NULL string is passed
*/
int count_newlines(char *str) {
	if (str == NULL)
		return 0;

	// Iteratively count the '\n's
	int count = 0;
	for (int i = 0; str[i] != '\0'; i++)
		if (str[i] == '\n')
			count++;

	return count;
}


// Recomputes total_len, weight, height and newlines of a node
void update_metadata(RopeNode *node) {
	// Edge case
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
		// total_len of internal node = sum of total_len of the left child and the right child
		int left_len = node->left ? node->left->total_len : 0;
		int right_len = node->right ? node->right->total_len : 0;
		node->total_len = left_len + right_len;

		// Weight of internal node = total length of characters in the left subtree
		node->weight = left_len;

		// Calculate the height based on the heights of the node's children
		node->height = 1 + MAX(node_height(node->left), node_height(node->right));

		// newline = sum of number of newlines in the left child and the right child
		node->newlines = 0;
		if (node->left)
			node->newlines += node->left->newlines;
		if (node->right)
			node->newlines += node->right->newlines;
	}
}


// Allocates memory for a string, copies the input to it and returns the new string
char *string_copy(char *src) {
	char *dst = malloc(string_length(src) + 1);  // plus one for null character
	// If malloc fails
	if (dst == NULL) {
		perror("malloc");
		exit(EXIT_FAILURE);
	}

	strcpy(dst, src);
	return dst;
}


// Allocates memory for a string, copies a substring (of length 'n') of the input and returns the new string
char *substr_copy(char *start, int n) {
	// Edge case
	if (start == NULL)
		return NULL;
	if (n > string_length(start)) {
		n = string_length(start);
	}

    char *dst = malloc(n + 1);  // plus one for null character
	// If malloc fails
    if (dst == NULL) {
		perror("malloc");
		exit(EXIT_FAILURE);
	}

	strncpy(dst, start, n);
	dst[n] = '\0';           // Add null terminator
	return dst;
}
