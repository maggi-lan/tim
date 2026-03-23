#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

# define CTRL_PLUS(ch) ((ch) & 0x1f)  // 'Ctrl+Ch'


// Terminal state before enabling raw mode
struct termios old_term;


// Terminal operations
void enable_raw(void);
void disable_raw(void);
char read_key(void);

// Editor input operations
void process_keypress(void);

// Editor output operations
void refresh_screen(void);
void draw_rows(void);

// Helper functions
void halt(char *str);




/*
-> Switches terminal from canonical mode to raw mode
-> Does it by altering a couple of terminal attributes
*/
void enable_raw(void) {
    // Save initial terminal state and load it after program execution ends
    if (tcgetattr(STDIN_FILENO, &old_term) == -1) {
        halt("tcgetattr");
    }
    atexit(disable_raw);

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
        halt("tcsetattr");
    }
}


/*
-> Switches terminal from raw mode to canonical mode
-> Does it by restoring the initial terminal state (which was in canonical mode)
*/
void disable_raw(void) {
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &old_term) == -1) {
        halt("tcsetattr");
    }
}


// Captures keypress from STDIN and return it
char read_key(void) {
    int nread;
    char ch;

    // NOTE: read() has a timeout of 10 ms in raw mode
    // Loop ends when exactly one character is read from STDIN
    while ((nread = read(STDIN_FILENO, &ch, 1)) != 1) {
        // If read fails
        if (nread == -1 && errno != EAGAIN)
            halt("read");
    }

    return ch;
}


// Captures input keypress and executes editor commands
void process_keypress(void) {
    char ch = read_key();

    switch (ch) {
        // Quit command
        case CTRL_PLUS('q'):
            write(STDOUT_FILENO, "\x1b[2J", 4);  // clear terminal screen
            write(STDOUT_FILENO, "\x1b[H", 3);   // move cursor to top left
            exit(0);
            break;
    }
}


// Re-draws entire editor screen
void refresh_screen(void) {
    write(STDOUT_FILENO, "\x1b[2J", 4);  // clear terminal screen
    write(STDOUT_FILENO, "\x1b[H", 3);   // move cursor to top left

    draw_rows();

    write(STDOUT_FILENO, "\x1b[H", 3);   // move cursor to top left again
}


// Renders all visible rows in the editor viewport
void draw_rows(void) {
    for (int y = 0; y < 24; y++) {
        write(STDOUT_FILENO, "~\r\n", 3);
    }
}


// Exits process with an error message
void halt(char *str) {
    write(STDOUT_FILENO, "\x1b[2J", 4);  // clear terminal screen
    write(STDOUT_FILENO, "\x1b[H", 3);   // move cursor to top left

    perror(str);
    exit(1);
}


int main(void) {
    enable_raw();

    while(true) {
        process_keypress();
        refresh_screen();
    }

    return 0;
}
