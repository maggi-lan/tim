#include <stdio.h>
#include <string.h>

#include "../include/rope.h"
#include "../include/file_io.h"

// Debug helpers
void print_text(RopeNode *node);
void print_tree(RopeNode *root);
void print_tree_rec(RopeNode *node, int depth, char branch);

// Prints all the text in a rope using recursion (useful for debugging)
void print_text(RopeNode *node) {
	// BASE CASE
	if (node == NULL)
		return;

	// CASE-1: node = leaf node
	if (is_leaf(node))
		printf("%s", node->str);

	// CASE-2: node = internal node
	else {
		print_text(node->left);
		print_text(node->right);
	}
}


// Prints the tree structure (useful for debugging)
void print_tree(RopeNode *root) {
	printf("\n========== ROPE TREE DUMP ==========\n");
	if (root == NULL)
		printf("(empty tree)\n");
	else
		print_tree_rec(root, 0, '*');
	printf("====================================\n\n");
}


// Recursive helper function for print_tree()
void print_tree_rec(RopeNode *node, int depth, char branch) {
	if (node == NULL)
		return;

	// Indentation based on depth
	for (int i = 0; i < depth; i++)
		printf("    ");

	// Print branch direction (root = '*')
	if (depth == 0)
		printf("* ");
	else if (branch == 'L')
		printf("L── ");
	else if (branch == 'R')
		printf("R── ");

	printf("[%p] h=%d w=%d len=%d nl=%d ",
		   (void *)node, node->height, node->weight, node->total_len, node->newlines);

	// Leaf preview
	if (node->str != NULL) {
		printf("leaf=\"");
		for (int i = 0; i < 20 && node->str[i] != '\0'; i++) {
			if (node->str[i] == '\n')
				printf("\\n");
			else
				putchar(node->str[i]);
		}
		if (strlen(node->str) > 20)
			printf("...");
		printf("\" ");
	}

	printf(" parent=%p\n", (void *)node->parent);

	print_tree_rec(node->left,  depth + 1, 'L');
	print_tree_rec(node->right, depth + 1, 'R');
}





int main(int argc, char **argv) {
	if (argc != 2) {
		printf("Incorrect usage\nTry: ./tim_debug <file>\n");
		return 1;
	}
 
	RopeNode *root = load_file(argv[1]);
	if (!root)
        return 1;
 
	print_tree(root);
	print_text(root);
 
	free_rope(root);
	return 0;
}
