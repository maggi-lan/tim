#include <stdio.h>

#include "rope.h"
#include "file_io.h"

int main(int argc, char **argv) {
	if (argc != 2) {
		printf("Incorrect usage\nTry: ./tim <file>\n");
		return 1;
	}

	RopeNode *root = load_file(argv[1]);
	if (!root)
        return 1;

	// TODO: idk, make it an editor or something

	free_rope(root);
	return 0;
}
