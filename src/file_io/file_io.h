#ifndef FILE_IO_H
#define FILE_IO_H

#include "rope.h"

#include <stdbool.h>
#include <stdio.h>

// File operations
RopeNode *load_file(char *filename);
void write_rope_to_file(RopeNode *node, FILE *fp);
bool save_file(RopeNode *root, char *filename);

#endif
