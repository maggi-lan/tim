#include "rope.h"

#include <stdio.h>
#include <stdlib.h>


// Allocates a new rope node, attaches text, sets metadata and returns it
RopeNode *create_leaf(char *text) {
	RopeNode *node = calloc(1, sizeof(RopeNode));	
	// If calloc fails
	if (node == NULL) {
		perror("calloc");
		exit(EXIT_FAILURE);
	}

	node->str = string_copy(text);
	update_metadata(node);

	return node;
}


/*
-> Combines two subtrees and returns the root of the concatenated subtree
-> NOTE: concat() rebalances just the new concatenated subtree, not the whole tree above it
-> NOTE: don't forget to rebalance the nodes above the concatenated subtree after using concat()
*/
RopeNode *concat(RopeNode *left_subtree, RopeNode *right_subtree) {
	// Edge cases
	if (left_subtree == NULL)
		return right_subtree;
	if (right_subtree == NULL)
		return left_subtree;

	int skew = node_height(right_subtree) - node_height(left_subtree);

	// CASE-1: There isn't much height difference between the left & right subtrees
	// Create a new parent node and attach the left & right subtrees as its children
	if (skew >= -1 && skew <= 1) {
		RopeNode *parent_node = calloc(1, sizeof(RopeNode));
		// If calloc fails
		if (parent_node == NULL) {
			perror("calloc");
			exit(EXIT_FAILURE);
		}

		parent_node->left = left_subtree;
		parent_node->right = right_subtree;
		update_metadata(parent_node);

		left_subtree->parent = parent_node;
		right_subtree->parent = parent_node;

		return parent_node;
	}

	// CASE-2: Right subtree is heavier
    // Attach left_subtree deep in the left spine of right_subtree
	if (skew >= 2) {
		// Recurse down the left spine of right_subtree to find the perfect spot for concatenating (|skew| <= 1)
		right_subtree->left = concat(left_subtree, right_subtree->left);

		if (right_subtree->left)
			right_subtree->left->parent = right_subtree;

		update_metadata(right_subtree);

		return rebalance(right_subtree);  // right_subtree = concatenated subtree's root
	}

	// CASE-3: Left subtree is heavier
    // Attach right_subtree deep in the right spine of left_subtree
	if (skew <= -2) {
		// Recurse down the right spine of left_subtree to find the perfect spot for concatenating (|skew| <= 1)
		left_subtree->right = concat(left_subtree->right, right_subtree);

		if (left_subtree->right)
			left_subtree->right->parent = left_subtree;

		update_metadata(left_subtree);
		return rebalance(left_subtree);  // left_subtree = concatenated subtree's root
	}

	// concat() should never reach here but this silences warnings
	return NULL;
}


/*
-> Splits a tree into two parts at a given index recursively and concatenates to rebuild the trees
-> 'left' and 'right' are the resulting subtrees
*/
void split(RopeNode *node, int idx, RopeNode **left, RopeNode **right) {
	// Edge case
	if (node == NULL) {
		*left = NULL;
		*right = NULL;
		return;
	}

	// BASE CASE: 'node' is a leaf node
	if (is_leaf(node)) {
		int len = string_length(node->str);

		// SUB-CASE-A: everything goes to the right
		if (idx <= 0) {
			*left = NULL;
			*right = node;
		}

		// SUB-CASE-B: everything goes to the left
		else if (idx >= len) {
			*left = node;
			*right = NULL;
		}

		// SUB-CASE-C: split the leaf into two leaves
		else {
			char *left_str = substr_copy(node->str, idx);
			char *right_str = substr_copy(node->str + idx, len - idx);

			*left = create_leaf(left_str);
			*right = create_leaf(right_str);

			free(left_str);
			free(right_str);
			free(node->str);
			free(node);
		}

		return;
	}

	// CASE-1: required index is in the left subtree
	if (idx < node->weight) {
		RopeNode *L;  // left split of the left subtree
		RopeNode *R;  // right split of the left subtree

		// Split the left subtree
		split(node->left, idx, &L, &R);

		// concatenate the right portion of the split of left subtree with right portion of current split
		*right = concat(R, node->right);

		// Update 'left' with the left portion of the split of left subtree
		*left = L;
	}

	// CASE-2: required index is in the right subtree
	else {
		RopeNode *L;  // left split of the right subtree
		RopeNode *R;  // right split of the right subtree

		// Split the right subtree
		split(node->right, idx - node->weight, &L, &R);  // NOTE: index changes when we recurse to right subtree

		// concatenate the left portion of the split of right subtree with left portion of current split
		*left = concat(node->left, L);

		// Update 'right' with the right portion of the split of right subtree
		*right = R;
	}

	// Free the old internal node
	free(node);
}


// Builds a rope structure from the string of text and returns the root of the rope
RopeNode *build_rope(char *text) {
	// Edge case
	if (text == NULL)
		return NULL;

	int len = string_length(text);
	RopeNode *root = NULL;

	// Iteratively create leaves and concatenate to the root
	for (int i = 0; i < len; i += CHUNK_SIZE) {
		char *buffer = substr_copy(text + i, CHUNK_SIZE);
		RopeNode *leaf = create_leaf(buffer);
		root = concat(root, leaf);
		free(buffer);
	}

	return root;
}

/*
-> Inserts a string at a given index
-> Splits the rope into two at the given index and concatenates a new rope in between
-> Returns the new root
*/
RopeNode *insert_at(RopeNode *root, int idx, char *text) {
	// Edge case
	if (root == NULL)
		return build_rope(text);
	if (text == NULL)
		return root;

	// Out of bounds handling
	if (idx < 0)
		idx = 0;
	else if (idx > root->total_len)
		idx = root->total_len;

	RopeNode *left, *right;
	split(root, idx, &left, &right);

	RopeNode *middle = build_rope(text);

	RopeNode *new_root;
	new_root = concat(left, middle);
	new_root = concat(new_root, right);

	return new_root;
}


/*
-> Deletes a string of certain length at a given index
-> Splits the rope at two points, concatenates the left most & right most ropes and deletes the middle rope
-> root = root node of the rope
-> start = starts deleting from this index
-> len = length of text to be deleted
-> Returns the new root
*/
RopeNode *delete_at(RopeNode *root, int start, int len) {
	// Edge case
	if (root == NULL || len <= 0)
		return root;

	// Out of bounds handling
	if (start < 0)
		start = 0;
	if (start >= root->total_len)
		return root;  // nothing to delete
	if (start + len > root->total_len)
		len = root->total_len - start;

	// Split at 'start' -> left + mid
	RopeNode *left = NULL;
	RopeNode *mid = NULL;
	split(root, start, &left, &mid);

	// Split at 'mid' at 'len' -> mid + right
	RopeNode *right = NULL;
	split(mid, len, &mid, &right);

	free_rope(mid);
	mid = NULL;

	RopeNode *new_root = concat(left, right);

	return new_root;
}


// Recursively frees all nodes and their text chunks in a rope
void free_rope(RopeNode *node) {
	// BASE-CASE
	if (node == NULL)
		return;

	// Free children first: Post Order
	free_rope(node->left);
	free_rope(node->right);

	if (node->str != NULL)
		free(node->str);

	node->left = NULL;
	node->right = NULL;
	node->parent = NULL;
	node->str = NULL;

	free(node);
}
