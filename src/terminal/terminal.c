#include <ctype.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>


// Terminal state before enabling raw mode
struct termios old_term;


// Raw mode operations
void enableRawMode(void);
void disableRawMode(void);




/*
-> Switches terminal from canonical mode to raw mode
-> Does it by altering a couple of terminal attributes
*/
void enableRawMode(void) {
    // Save initial terminal state and load it after program execution ends
    if (tcgetattr(STDIN_FILENO, &old_term) == -1) {
        perror("tcgetattr");
        exit(EXIT_FAILURE);
    }
    atexit(disableRawMode);

    struct termios raw_term = old_term;

    // a) disable Ctrl-S, Ctrl-Q
    // b) disable '\r' translation to '\n'
    // c) disable default behaviour of break conditions
    // d) disable parity checking
    // e) disable input byte stripping
    raw_term.c_iflag &= ~(IXON | ICRNL | BRKINT | INPCK | ISTRIP);

    // a) disable '\n' translation to '\r\n'
    raw_term.c_oflag &= ~(OPOST);

    // a) set character byte = 8 bits
    raw_term.c_cflag |= ~(CS8);

    // a) stop printing every keystroke
    // b) read input byte by byte
    // c) disable Ctrl-C, Ctrl-Z
    // d) disable Ctrl-V
    raw_term.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN);

    // a) set minimum number of bytes of input needed before read() can return
    // b) set maximum amount of time (in tenths of a second) to wait for read() to return
    raw_term.c_cc[VMIN] = 0;
    raw_term.c_cc[VTIME] = 1;  // basically adds a timeout for read()

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw_term) == -1) {
        perror("tcsetattr");
        exit(EXIT_FAILURE);
    }
}

/*
-> Switches terminal from raw mode to canonical mode
-> Does it by restoring the initial terminal state (which was in canonical mode)
*/
void disableRawMode(void) {
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &old_term) == -1) {
        perror("tcsetattr");
        exit(EXIT_FAILURE);
    }
}


int main(void) {
    enableRawMode();

    while(true) {
        char ch = '\0';
        if (read(STDIN_FILENO, &ch, 1) == -1 && errno != EAGAIN) {
            perror("read");
            exit(EXIT_FAILURE);
        }

        if (iscntrl(ch)) {
            printf("%d\r\n", ch);
        }
        else {
            printf("%d ('%c')\r\n", ch, ch);
        }

        if (ch == 'q')
            break;
    }

    return 0;
}
