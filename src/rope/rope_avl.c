#include "rope.h"

#include <stdlib.h>


// Returns the height difference between the left child and the right child of a node
int get_skew(RopeNode *node) {
	// Edge case
	if (node == NULL)
		return 0;

	return node_height(node->right) - node_height(node->left);
}


// Performs a right rotation and returns the new subtree root
RopeNode *rotate_right(RopeNode *node) {
	/*
	# initially:
         y
        / \
       x  [C]
      / \
    [A] [B]

	# after right-rotation:
         x
        / \
      [A]  y
          / \
        [B] [C]
	*/

	// Edge case: y is NULL or x is NULL
	if (node == NULL || node->left == NULL)
		return NULL;

	RopeNode *y = node;
	RopeNode *x = y->left;
	RopeNode *B = x->right;

	// Shift the parent of y to become the parent of x
	RopeNode *parent = node->parent;
	y->parent = NULL;
	x->parent = parent;
	if (parent != NULL) {
		if (parent->left == y)
			parent->left = x;
		else
			parent->right = x;
	}

	// Shift y to be the right child of x
	x->right = y;
	y->parent = x;

	// Move B
	y->left = B;
	if (B != NULL)
		B->parent = y;

	update_metadata(y);
	update_metadata(x);

	return x;
}


// Performs a left rotation and returns the new subtree root
RopeNode *rotate_left(RopeNode *node) {
	/*
	# initially:
         x
        / \
      [A]  y
          / \
        [B] [C]

	# after left-rotation:
         y
        / \
       x  [C]
      / \
    [A] [B]
	*/

	// Edge case: x is NULL or y is NULL
	if (node == NULL || node->right == NULL)
		return NULL;

	RopeNode *x = node;
	RopeNode *y = x->right;
	RopeNode *B = y->left;

	// Shift the parent of x to become the parent of y
	RopeNode *parent = node->parent;
	x->parent = NULL;
	y->parent = parent;
	if (parent != NULL) {
		if (parent->left == x)
			parent->left = y;
		else
			parent->right = y;
	}

	// Shift x to be the left child of y
	y->left = x;
	x->parent = y;

	// Move B
	x->right = B;
	if (B != NULL) {
		B->parent = x;
	}

	update_metadata(x);
	update_metadata(y);

	return y;
}


// Performs AVL balancing and returns the root of the subtree on which the balancing is done on
RopeNode *rebalance(RopeNode *node) {
	// Edge case
	if (node == NULL)
		return NULL;

	update_metadata(node);
	int skew = get_skew(node);  // no need to rebalance if skew is either -1, 0 or 1

	// CASE-1: if right side is heavier
	if (skew == 2) {
		int right_skew = get_skew(node->right);

		// SUB-CASE-A: right_skew > -1: one left rotation on the root node
		if (right_skew == 1 || right_skew == 0) {
			RopeNode *result = rotate_left(node);
			update_metadata(result);
			return result;
		}

		// SUB-CASE-B: right_skew = -1: one right rotation on the right node + one left rotation on the root node
		else if (right_skew == -1) {
			update_metadata(rotate_right(node->right));
			RopeNode *result = rotate_left(node);
			update_metadata(result);
			return result;
		}
	}

	// CASE-2: f left side is heavier
	else if (skew == -2) {
		int left_skew = get_skew(node->left);

		// SUB-CASE-A: left_skew < 1: one right rotation on the root node
		if (left_skew == -1 || left_skew == 0) {
			RopeNode *result = rotate_right(node);
			update_metadata(result);
			return result;
		}

		// SUB-CASE-B: left_skew = 1: one left rotation on the left node + one right rotation on the root node
		else if (left_skew == 1) {
			update_metadata(rotate_left(node->left));
			RopeNode *result = rotate_right(node);
			update_metadata(result);
			return result;
		}
	}

	// If skew = -1, 0, 1 or anything else
	return node;
}
