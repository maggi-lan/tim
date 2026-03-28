#include "terminal.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>


// Append a string to an AppendBuffer
void ab_append(AppendBuffer *ab, char *str, int len) {
    char *new = realloc(ab->buffer, ab->len + len);
    // If realloc fails
    if (new == NULL)
        halt("ab_append");

    memcpy(&new[ab->len], str, len);  // copy 'str' to 'new'

    ab->buffer = new;
    ab->len += len;
}


// Frees an AppendBuffer in memory
void ab_free(AppendBuffer *ab) {
    free(ab->buffer);
}
