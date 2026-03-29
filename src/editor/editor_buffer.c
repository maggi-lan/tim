#include "editor.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "terminal.h"
#include "rope.h"


// Append a string 'str' (of length 'len' excluding '\0') to an AppendBuffer
void ab_append(AppendBuffer *ab, char *str, int len) {
    // Edge case
    if (str == NULL)
        return;

    // 'ab' has an empty buffer -> initialize buffer
    if (ab->bufflen == 0) {
        ab->buffer = calloc(len + 1, 1);
        // Error handling
        if (ab->buffer == NULL)
            halt("ab_append");

        memcpy(ab->buffer, str, len + 1);  // copy 'str' to buffer

        ab->bufflen = ab->capacity;
        ab->capacity = len + 1;
    }

    else {
        // Expand buffer if needed
        int cap = ab->capacity;
        while (cap < (ab->bufflen + len))
            cap *= 2;
        if (cap != ab->capacity) {
            char *new = realloc(ab->buffer, cap);
            // If realloc fails
            if (new == NULL)
                halt("ab_append");

            ab->buffer = new;
            ab->capacity = cap;
        }

        // Append 'str' to buffer
        memcpy(&ab->buffer[ab->bufflen - 1], str, len + 1);
        ab->bufflen += len;
    }
}


// Frees an AppendBuffer in memory
void ab_free(AppendBuffer *ab) {
    free(ab->buffer);
}
