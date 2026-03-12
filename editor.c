#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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


// Helper functions
bool is_leaf(RopeNode *node);
int node_height(RopeNode *node);
int string_length(char *str);
int count_newlines(char *str);
void update_metadata(RopeNode *node);
char *string_copy(char *src);
char *substr_copy(char *start, int n);

// Rope functions
RopeNode *create_leaf(char *text);
RopeNode *concat(RopeNode *left, RopeNode *right);
void split(RopeNode *root, int idx, RopeNode **left, RopeNode **right);
RopeNode *build_rope(char *text);
RopeNode *insert_at(RopeNode *root, int idx, char *text);
RopeNode *delete_at(RopeNode *root, int start, int len);
void free_rope(RopeNode *root);

// File operations
RopeNode *load_file(char *filename);
bool save_file(RopeNode *root, char *filename);
void write_rope_to_file(RopeNode *node, FILE *fp);

// AVL balancing
int get_skew(RopeNode *node);
RopeNode *rotate_right(RopeNode *node);
RopeNode *rotate_left(RopeNode *node);
RopeNode *rebalance(RopeNode *node);

// Editor utility functions
char char_at(RopeNode *root, int idx);
int find_newline_pos(RopeNode *node, int newline_idx, int offset);
int get_line_start(RopeNode *root, int line);
int get_line_length(RopeNode *root, int line);
int count_total_lines(RopeNode *root);

// Debug helpers
void print_text(RopeNode *node);
void print_tree(RopeNode *root);
void print_tree_rec(RopeNode *node, int depth, char branch);




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
	new_root = rebalance(new_root);

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


// Loads the file into a rope and returns the root of the rope
RopeNode *load_file(char *filename) {
	FILE *fp = fopen(filename, "rb");  // open the file in read mode
	// Error handling
	if (!fp) {
        perror("Error opening file");
        return NULL;
    }

    RopeNode *root = NULL;
    char buffer[CHUNK_SIZE + 1] = {0};

	// Read the file in chunks [fread() loads the chunk of text into buffer]
	int n;
	while ((n = fread(buffer, 1, CHUNK_SIZE, fp)) > 0) {  // fread() returns the number of characters that were read
		buffer[n] = '\0';                                 // terminate buffer with null character
		RopeNode *leaf = create_leaf(buffer);
		root = concat(root, leaf);
    }

    fclose(fp);
    return root;
}


// Writes the rope content to a file recursively
void write_rope_to_file(RopeNode *node, FILE *fp) {
	// Base condition-1: NULL is reached
	if (node == NULL)
		return;

	// Base condition-2: leaf is reached
	if (is_leaf(node)) {
		if (node->str != NULL)
			fprintf(fp, "%s", node->str);     // appends the string to the file
		return;
	}

	write_rope_to_file(node->left, fp);
	write_rope_to_file(node->right, fp);
}


// Saves the rope to a file and returns 'true' if successful
bool save_file(RopeNode *root, char *filename) {
	// Edge case
	if (filename == NULL)
		return false;

	FILE *fp = fopen(filename, "w");
    // Error handling
	if (!fp) {
		perror("Error saving file");
		return false;
	}

	write_rope_to_file(root, fp);
	fclose(fp);

	return true;
}


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


/*
-> Returns the character at a given index recursively
-> Usage: pass in the root node and zero-based index
-> Returns null character if index goes out of bounds
*/
char char_at(RopeNode *node, int idx) {
    // Edge case
    if (node == NULL)
        return '\0';

    // BASE CASE: leaf nodes -> return the character at 'idx'
    if (is_leaf(node)) {
        // Out of bounds handling
        if (idx < 0 || idx >= node->weight)
            return '\0';

        return node->str[idx];
    }

    // CASE-1: 'idx' lies in the left subtree -> recurse to the left subtree
    if (idx < node->weight)
        return char_at(node->left, idx);

    // CASE-2: 'idx' lies in the right subtree -> recurse to the right subtree
    else
        return char_at(node->right, idx - node->weight);  // adjust index
}


/*
-> Returns the text index (zero-based) of the Nth newline character (zero-based) recursively
-> Returns -1 if newline_idx is invalid
*/
int find_newline_pos(RopeNode *node, int newline_idx, int offset) {
    // Edge case
    if (node == NULL)
        return -1;

    // BASE CASE: leaf nodes -> iteratively check for the newline in the text chunk
    if (is_leaf(node)) {
        int newline_count = 0;
        for (int i = 0; node->str[i] != '\0'; i++) {
            if (node->str[i] == '\n') {
                if (newline_count == newline_idx)
                    return offset + i;  // offset = index of the first character of the text chunk
                newline_count++;
            }
        }

        return -1;  // execution reaches here if there is no newline in the text chunk
    }

    int left_newlines = node->left ? node->left->newlines : 0;

    // CASE-1: target newline is in left subtree -> recurse to the left subtree
    if (newline_idx < left_newlines)
        return find_newline_pos(node->left, newline_idx, offset);

    // CASE-2: target newline is in right subtree -> recurse to the right subtree
    else
        // Adjust newline_idx and offset
        return find_newline_pos(node->right, newline_idx - left_newlines, offset + node->weight);
}


/*
-> Returns the starting position (zero-based character index) of Nth line (zero-based)
-> Returns 0 in case of error or negative line
-> Returns the index after the last character in the text if 'line' exceeds the number of available lines
*/
int get_line_start(RopeNode *root, int line) {
    // Edge case
    if (root == NULL || line < 0)
        return 0;

    if (root->total_len == 0)
        return 0;

    // Line 0 starts at index 0
    if (line == 0)
        return 0;

    // Line N starts at position after (N-1)th newline
    int newline_pos = find_newline_pos(root, line - 1, 0);

    if (newline_pos == -1)
        return root->total_len;  // Line doesn't exist, return (last_index + 1)

    return newline_pos + 1;  // Position after the newline
}


/*
-> Returns the length of a specific line (excluding newline)
-> Pass in root and line index (zero-based)
-> Returns zero in case of error
*/
int get_line_length(RopeNode *root, int line) {
    // Edge case
    if (root == NULL)
        return 0;

    int start = get_line_start(root, line);  // starting position of the line

    // If 'line' exceeds the total number of available lines
    if (start >= root->total_len)
        return 0;

    int end;  // position after the last character of the line

    // If the 'line' is the last line and it doesn't have a newline character at the end
    if (line >= root->newlines) {
        end = root->total_len;
    }
    else {
        int newline_pos = find_newline_pos(root, line, 0);
        end = newline_pos;
    }

    return end - start;
}

// Returns the total number of lines in a rope
int count_total_lines(RopeNode *root) {
    if (root == NULL || root->total_len == 0)
        return 1;  // treat empty buffers as one empty line

    // Ex: "Hello" -> 0 newlines -> 1 line
    // Ex: "Hello\nThere" -> 1 newline -> 2 lines
    // Ex: "Hello\n" -> 1 newline -> 2 lines (last line is an empty line)
    return root->newlines + 1;
}


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
		printf("Incorrect usage\nTry: ./tim <file>\n");
		return 1;
	}

	RopeNode *root = load_file(argv[1]);

	root = delete_at(root, 7, 6);
	print_text(root);

	char *filename = argv[1];
	if (save_file(root, filename))
		printf("File saved successfully to: %s\n", filename);

	free_rope(root);

	return 0;
}
