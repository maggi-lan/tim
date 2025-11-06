#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define CHUNK_SIZE 2


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
int count_newlines(char *str);
void update_metadata(RopeNode *node);
char *string_copy(const char *src);
void print_text(RopeNode *node);

// Rope functions
RopeNode *create_leaf(char *text);
RopeNode *concat(RopeNode *left, RopeNode *right);
RopeNode *append_node(RopeNode *root, RopeNode *leaf);
RopeNode *load_file(char *filename);

// AVL balancing
int get_skew(RopeNode *node);
RopeNode *rotate_right(RopeNode *node);
RopeNode *rotate_left(RopeNode *node);
RopeNode *rebalance(RopeNode *node);

// Debug helpers
void print_tree(RopeNode *root);
void print_tree_rec(RopeNode *node, int depth, char branch);



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

// Returns the number '\n's in a string
int count_newlines(char *str) {
	// Edge case when str is NULL
	if (str == NULL)
		return 0;

	// Iteratively count the '\n's
	int count = 0;
	for (int i = 0; str[i] != '\0'; i++)
		if (str[i] == '\n')
			count++;

	return count;
}


// Recomputes total_len, weight, height and newlines
void update_metadata(RopeNode *node) {
	// Edge case when node is NULL
	if (node == NULL)
		return;

	// CASE 1: node = leaf node
	if (is_leaf(node)) {
		node->total_len = string_length(node->str);
		node->weight = node->total_len;              // weight of a leaf node = strlen(node->str)

		node->height = 1;                            // height of a leaf node is 1

		node->newlines = count_newlines(node->str);  // calculates the count of newlines in node->str
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


// Allocates memory for a string and copies the input to it
char *string_copy(const char *src) {
    char *dst = malloc(strlen (src) + 1);  // Space for length plus nul
	// If malloc fails
    if (dst == NULL) {
		perror("malloc");
		exit(EXIT_FAILURE);
	}

    strcpy(dst, src);                      // Copy the characters
    return dst;                            // Return the new string
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


// Allocates a new rope node, sets metadata and returns it
RopeNode *create_leaf(char *text) {
	RopeNode *node = calloc(1, sizeof(RopeNode));	
	// If calloc fails
	if (node == NULL) {
		perror("calloc");
		exit(EXIT_FAILURE);
	}

	// Set metadata
	node->str = string_copy(text);  // allocates & copies text into node->str
	update_metadata(node);          // update the metadata of the node

	return node;
}


// Combines two nodes
RopeNode *concat(RopeNode *left, RopeNode *right) {
	// Edge cases
	if (left == NULL)
		return right;
	if (right == NULL)
		return left;

	// Create an internal node whose children would be left & right
	RopeNode *node = calloc(1, sizeof(RopeNode));
	// If calloc fails
	if (node == NULL) {
		perror("calloc");
		exit(EXIT_FAILURE);
	}

	// Update the parent node
	node->left = left;
	node->right = right;
	update_metadata(node);

	// Set the parent pointers of left & right
	left->parent = node;
	right->parent = node;

	return node;
}


// Inserts leaves at the right most internal node (basically appending the leaf to the tree)
RopeNode *append_node(RopeNode *root, RopeNode *leaf) {
	// Appending to empty tree
	if (root == NULL)
		root = leaf;

	else {
		// If tree has only one leaf
		if (is_leaf(root))
			root = concat(root, leaf);  // root concatenates two leaves

		// If tree has internal nodes
		else {
			// Trying to set right_most = right most internal node
			RopeNode *right_most = root;
			while (!is_leaf(right_most->right))
				right_most = right_most->right;

			// new_node concatenates two leaves and becomes the right child of right_most
			RopeNode *new_node = concat(right_most->right, leaf);
			right_most->right = new_node;
			new_node->parent = right_most;
			update_metadata(right_most);

			// Rebalance iteratively from bottom to up
			for (RopeNode *node = new_node; node != NULL; node = node->parent) {
				if (node->parent == NULL)
					root = rebalance(node);
				else
					rebalance(node);
			}
		}
	}

	return root;
}


// Loads the file into a rope
RopeNode *load_file(char *filename) {
	FILE *fp = fopen(filename, "r");  // open the file in read mode
	// Error handling
	if (!fp) {
        perror("Error opening file");
        return NULL;
    }

    RopeNode *root = NULL;              // root of the whole rope
    char buffer[CHUNK_SIZE + 1] = {0};  // buffer to read chunks (of fixed size) from the file

	// Read the file in chunks [fread() loads the chunk of text into buffer]
	int n;
	while ((n = fread(buffer, 1, CHUNK_SIZE, fp)) > 0) {  // fread() returns the number of characters that were read
		buffer[n] = '\0';                                 // terminate buffer with null character
		RopeNode *leaf = create_leaf(buffer);             // create a leaf with the buffer
		root = append_node(root, leaf);                   // append the leaf to the tree
    }

    fclose(fp);
    return root;
}


// Returns the height difference between left and right children of a node
int get_skew(RopeNode *node) {
	// Edge case: node is NULL
	if (node == NULL)
		return 0;

	return node_height(node->right) - node_height(node->left);
}


// Performs a right rotation and returns the root of the rotation
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
	RopeNode *A = x->left;
	RopeNode *B = x->right;
	RopeNode *C = y->right;

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
	if (B != NULL) {
		y->left = B;
		B->parent = y;
	}

	// Update height, weight, total_len and newlines for x & y
	update_metadata(y);
	update_metadata(x);

	return x;
}


// Performs a left rotation and returns the root of the rotation
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
	RopeNode *A = x->left;
	RopeNode *B = y->left;
	RopeNode *C = y->right;

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
	if (B != NULL) {
		x->right = B;
		B->parent = x;
	}

	// Update height, weight, total_len and newlines for x & y
	update_metadata(x);
	update_metadata(y);

	return y;
}


// Performs AVL balancing and returns the root of the subtree on which the balancing is done on
RopeNode *rebalance(RopeNode *node) {
	// Edge case: node is NULL
	if (node == NULL)
		return NULL;

	update_metadata(node);      // update node meta data before proceeding
	int skew = get_skew(node);  // no need to rebalance if skew is either -1, 0 or 1

	// If right side is heavier
	if (skew == 2) {
		int right_skew = get_skew(node->right);  // right_skew = skew of the right node

		// CASE 1: right_skew > -1: one left rotation on the root node
		if (right_skew == 1 || right_skew == 0) {
			RopeNode *result = rotate_left(node);
			update_metadata(result);
			return result;
		}

		// CASE 2: right_skew = -1: one right rotation on the right node + one left rotation on the root node
		else if (right_skew == -1) {
			update_metadata(rotate_right(node->right));
			RopeNode *result = rotate_left(node);
			update_metadata(result);
			return result;
		}
	}

	// If left side is heavier
	else if (skew == -2) {
		int left_skew = get_skew(node->left);

		// CASE 1: left_skew < 1: one right rotation on the root node
		if (left_skew == -1 || left_skew == 0) {
			RopeNode *result = rotate_right(node);
			update_metadata(result);
			return result;
		}

		// CASE 2: left_skew = 1: one left rotation on the left node + one right rotation on the root node
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


// Prints the tree structure
// Useful for debugging
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

	// Print node metadata
	printf("[%p] h=%d w=%d len=%d nl=%d ",
		   (void *)node, node->height, node->weight, node->total_len, node->newlines);

	// Leaf preview
	if (node->str != NULL) {
		printf("leaf=\"");
		for (int i = 0; i < 20 && node->str[i] != '\0'; i++)
			putchar(node->str[i]);
		if (node->str[20] != '\0')
			printf("...");
		printf("\" ");
	}

	// Parent pointer
	printf(" parent=%p\n", (void *)node->parent);

	// Recursive printing
	print_tree_rec(node->left,  depth + 1, 'L');
	print_tree_rec(node->right, depth + 1, 'R');
}




int main(int argc, char **argv) {
	if (argc != 2) {
		printf("Incorrect usage\nTry: ./tim <file>\n");
		return 1;
	}

	RopeNode *root = load_file(argv[1]);

	print_text(root);

	print_tree(root);
	return 0;
}
