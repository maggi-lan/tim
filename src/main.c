#include <stdio.h>
#include <stdbool.h>

#include "rope.h"
#include "file_io.h"
#include "terminal.h"
#include "editor.h"




int main(int argc, char **argv) {
	if (argc != 2) {
		printf("Incorrect usage\nTry: ./tim <file>\n");
		return 1;
	}

	RopeNode *root = load_file(argv[1]);

    enable_raw();
    init_editor(root);

    while(true) {
        refresh_screen();
        if (process_keypress() == -1)
            break;
    }

	free_rope(E.rope);
	return 0;
}
