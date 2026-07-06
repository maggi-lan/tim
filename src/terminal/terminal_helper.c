#include "terminal.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


// Exits process with an error message
void halt(const char *str) {
    write(STDOUT_FILENO, "\x1b[2J", 4);  // clear terminal screen
    write(STDOUT_FILENO, "\x1b[H", 3);   // move cursor to top left

    perror(str);
    exit(1);
}


/*
-> Reads + parses escape sequences and returns the key code
-> Assumes that the first character is already read from STDIN
-> Returns an escape character in case of failure
*/
int escape_parser(void) {
    char seq[3];

    if (read(STDIN_FILENO, &seq[0], 1) != 1)  // 2nd character
        return '\x1b';
    if (read(STDIN_FILENO, &seq[1], 1) != 1)  // 3rd character
        return '\x1b';

    if (seq[0] == '[') {
        // Escape sequence is 4 characters long
        if (seq[1] >= '0' && seq[1] <= '9') {
            if (read(STDIN_FILENO, &seq[2], 1) != 1)  // 4th character
                return '\x1b';

            if (seq[2] == '~') {
                switch (seq[1]) {
                    case '1':
                        return HOME_KEY;   // <esc>[1~
                    case '3':
                        return DEL_KEY;    // <esc>[3~
                    case '4':
                        return END_KEY;    // <esc>[4~
                    case '5':
                        return PAGE_UP;    // <esc>[5~
                    case '6':
                        return PAGE_DOWN;  // <esc>[6~
                    case '7':
                        return HOME_KEY;   // <esc>[7~
                    case '8':
                        return END_KEY;    // <esc>[8~
                }
            }
        }

        // Escape sequence is 3 characters long
        else {
            switch (seq[1]) {
                case 'A':
                    return ARROW_UP;     // <esc>[A
                case 'B':
                    return ARROW_DOWN;   // <esc>[B
                case 'C':
                    return ARROW_RIGHT;  // <esc>[C
                case 'D':
                    return ARROW_LEFT;   // <esc>[D
                case 'H':
                    return HOME_KEY;     // <esc>[H
                case 'F':
                    return END_KEY;      // <esc>[F
            }
        }
    }

    else if (seq[0] == 'O') {
        switch (seq[1]) {
            case 'H':
                return HOME_KEY;  // <esc>OH
            case 'F':
                return END_KEY;   // <esc>OF
        }
    }

    return '\x1b';  // invalid escape sequence
}
