#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define CHUNK_SIZE 64


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
RopeNode *load_file(char *filename);




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


// Loads the file into a rope
RopeNode *load_file(char *filename) {
	FILE *fp = fopen(filename, "r");  // open the file in read mode
	// Error Handling
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
		root = concat(root, leaf);                        // concatenate the new leaf with root
		// NOTE: This is not balanced yet
    }

    fclose(fp);
    return root;
}




int main(int argc, char **argv) {
	if (argc != 2) {
		printf("Incorrect usage\nTry: ./tim <file>\n");
		return 1;
	}

	RopeNode *root = load_file(argv[1]);
	print_text(root);
	return 0;
}
