#include "rope.h"

#include <stdlib.h>

#include "terminal.h"


// Creates a leaf node from the given text chunk
RopeNode *create_leaf(const char *text) {
	RopeNode *node = calloc(1, sizeof(RopeNode));	
	if (node == NULL)
		halt("create_leaf");

	node->str = string_copy(text);
	update_metadata(node);

	return node;
}


/*
-> Concatenates two subtrees and returns the root of the new subtree
-> Rebalances the new concatenated subtree too
*/
RopeNode *concat(RopeNode *left_subtree, RopeNode *right_subtree) {
	if (left_subtree == NULL)
		return right_subtree;
	if (right_subtree == NULL)
		return left_subtree;

	RopeNode *concatenated_root = NULL;
	int skew = node_height(right_subtree) - node_height(left_subtree);


	// CASE-1: There isn't much height difference between left_subtree and right_subtree
	if (skew >= -1 && skew <= 1) {
		concatenated_root = calloc(1, sizeof(RopeNode));
		if (concatenated_root == NULL)
			halt("concat");

		concatenated_root->left = left_subtree;
		concatenated_root->right = right_subtree;

		left_subtree->parent = concatenated_root;
		right_subtree->parent = concatenated_root;

		update_metadata(concatenated_root);
		return concatenated_root;
	}

	// CASE-2: Right subtree is heavier -> attach left_subtree deep in the left spine of right_subtree
	else if (skew >= 2) {
		// Recurse down the left spine of right_subtree to find the perfect spot for concatenating (|skew| <= 1)
		right_subtree->left = concat(left_subtree, right_subtree->left);
		if (right_subtree->left)
			right_subtree->left->parent = right_subtree;

		concatenated_root = right_subtree;
	}

	// CASE-3: Left subtree is heavier -> attach right_subtree deep in the right spine of left_subtree
	else if (skew <= -2) {
		// Recurse down the right spine of left_subtree to find the perfect spot for concatenating (|skew| <= 1)
		left_subtree->right = concat(left_subtree->right, right_subtree);
		if (left_subtree->right)
			left_subtree->right->parent = left_subtree;

		concatenated_root = left_subtree;
	}

	update_metadata(concatenated_root);
	return rebalance(concatenated_root);
}


/*
-> Splits a rope into two subtrees at the given index
-> Stores the resulting left and right subtrees in 'left' and 'right'
*/
void split(RopeNode *node, int idx, RopeNode **left, RopeNode **right) {
	if (node == NULL) {
		*left = NULL;
		*right = NULL;
		return;
	}

	// BASE CASE
	if (is_leaf(node)) {
		int len = string_length(node->str);

		// SUB-CASE-A: split before the leaf
		if (idx <= 0) {
			*left = NULL;
			*right = node;
		}

		// SUB-CASE-B: split after the leaf
		else if (idx >= len) {
			*left = node;
			*right = NULL;
		}

		// SUB-CASE-C: split the leaf into two
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

	// CASE-1: required index is in the left subtree -> split the left subtree
	if (idx < node->weight) {
		RopeNode *left_split;   // of the left subtree
		RopeNode *right_split;  // of the left subtree

		split(node->left, idx, &left_split, &right_split);

        // 'node' will be freed later -> detach its children to avoid dangling pointers
        if (node->right)
            node->right->parent = NULL;
        if (left_split)
            left_split->parent = NULL;

		*right = concat(right_split, node->right);
		*left = left_split;
	}

	// CASE-2: required index is in the right subtree -> split the right subtree
	else {
		RopeNode *left_split;   // of the right subtree
		RopeNode *right_split;  // of the right subtree

		split(node->right, idx - node->weight, &left_split, &right_split);  // adjust idx relative to the right subtree

        // 'node' will be freed later -> detach its children to avoid dangling pointers
        if (node->left)
            node->left->parent = NULL;
        if (right_split)
            right_split->parent = NULL;

		*left = concat(node->left, left_split);
		*right = right_split;
	}

	free(node);
}


/*
-> Builds a rope from the given string of text
-> Returns the root of the rope
*/
RopeNode *build_rope(const char *text) {
	if (text == NULL)
		return NULL;

	int len = string_length(text);
	RopeNode *root = NULL;

	for (int i = 0; i < len; i += CHUNK_SIZE) {
		char *buffer = substr_copy(text + i, CHUNK_SIZE);
		RopeNode *leaf = create_leaf(buffer);
		root = concat(root, leaf);
		free(buffer);
	}

	return root;
}


/*
-> Inserts a string of text to a rope at a given index
-> Returns the new root of the rope after insertion
*/
RopeNode *insert_at(RopeNode *root, int idx, const char *text) {
	if (root == NULL)
		return build_rope(text);
	if (text == NULL)
		return root;

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
-> Deletes 'len' characters from the rope starting at index 'start'
-> Returns the new root of the rope after deletion
*/
RopeNode *delete_at(RopeNode *root, int start, int len) {
	if (root == NULL || len <= 0)
		return root;

	if (start < 0)
		start = 0;
	if (start >= root->total_len)
		return root;  // nothing to delete
	if (start + len > root->total_len)
		len = root->total_len - start;

	// root -> left + mid
	RopeNode *left = NULL;
	RopeNode *mid = NULL;
	split(root, start, &left, &mid);

	// mid -> mid + right
	RopeNode *right = NULL;
	split(mid, len, &mid, &right);

	free_rope(mid);
	mid = NULL;

	RopeNode *new_root = concat(left, right);

	return new_root;
}


// Recursively frees all nodes and their text chunks in a rope
void free_rope(RopeNode *node) {
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
