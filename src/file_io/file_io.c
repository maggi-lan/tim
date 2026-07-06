#include "file_io.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include "rope.h"
#include "terminal.h"


/*
-> Loads a file into a rope
-> Returns the root of the rope
-> Returns an empty rope if file doesn't exist
*/
RopeNode *load_file(const char *filename) {
	FILE *fp = fopen(filename, "rb");

	if (!fp) {
        // CASE-A: file doesn't exist -> return empty rope
        if (errno == ENOENT)
            return NULL;
        // CASE-B: can't load existing file -> exit program
        else
            halt("load_file");
    }

    RopeNode *root = NULL;
    char buffer[CHUNK_SIZE + 1] = {0};  // +1 for null terminator

	// Read the file in chunks
	int n;
	while ((n = fread(buffer, 1, CHUNK_SIZE, fp)) > 0) {
		buffer[n] = '\0';                      // terminate buffer
		RopeNode *leaf = create_leaf(buffer);
		root = concat(root, leaf);
    }

    fclose(fp);
    return root;
}


// Writes the rope contents to a file recursively
void write_rope_to_file(RopeNode *node, FILE *fp) {
	if (node == NULL)
		return;

	if (is_leaf(node)) {
		if (node->str != NULL)
			fprintf(fp, "%s", node->str);
		return;
	}

    // Inorder traversal to preserve the original text order
	write_rope_to_file(node->left, fp);
	write_rope_to_file(node->right, fp);
}


/*
-> Saves the text in a rope to a file
-> Returns 'true' on success, 'false' on failure
*/
bool save_file(RopeNode *root, const char *filename) {
	if (filename == NULL)
		return false;

	FILE *fp = fopen(filename, "w");
	if (!fp) {
		halt("save_file");
	}

	write_rope_to_file(root, fp);
	fclose(fp);

	return true;
}
