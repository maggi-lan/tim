#include <stdio.h>
#include <stdlib.h>
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

    char *filename = argv[1];
	RopeNode *root = load_file(filename);

    enable_raw();
    init_editor(root, filename);

    set_status_message("HELP: Ctrl-Q = quit");

    while(true) {
        refresh_screen();
        if (process_keypress() == -1)
            break;
    }

	free_rope(E.rope);
    free(E.filename);
	return 0;
}
