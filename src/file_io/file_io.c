#include <stdio.h>
#include <stdlib.h>

#include "file_io.h"
#include "rope.h"


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
