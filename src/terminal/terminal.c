#include <ctype.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

# define CTRL_PLUS(ch) ((ch) & 0x1f)  // 'Ctrl+Ch'
# define ABUF_INIT {NULL, 0}


// EditorState maintains the editor’s runtime data and configuration
typedef struct EditorState {
    int screenrows;           // number of visible rows in the screen
    int screencols;           // number of visible columns in the screen
    struct termios old_term;  // terminal state before enabling raw mode
} EditorState;

// AppendBuffer is a dynamic string type which supports appending
typedef struct AppendBuffer {
    char *buffer;  // buffer for the string
    int len;       // length of the buffer
} AppendBuffer;


// Global editor state
EditorState E;


// Terminal operations
void enable_raw(void);
void disable_raw(void);
char read_key(void);
int get_cursor_pos(int *row, int *col);
int get_window_size(int *rows, int *cols);

// Editor input operations
void process_keypress(void);

// Editor output operations
void refresh_screen(void);
void draw_rows(void);

// Editor append buffer
void ab_append(AppendBuffer *ab, char *str, int len);
void ab_free(AppendBuffer *ab);

// Editor initialization
void init_editor(void);

// Helper functions
void halt(char *str);




/*
-> Switches terminal from canonical mode to raw mode
-> Does it by altering a couple of terminal attributes
*/
void enable_raw(void) {
    // Save initial terminal state and load it after program execution ends
    if (tcgetattr(STDIN_FILENO, &E.old_term) == -1) {
        halt("tcgetattr");
    }
    atexit(disable_raw);

    struct termios raw_term = E.old_term;

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
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &E.old_term) == -1) {
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


/*
-> Fetches the current position (row, col) of the cursor
-> Updates the values pointed by 'row' and 'col'
-> Returns 0 in case of success and -1 in case of error
-> Cursor positions start from 1 (not 0)
*/
int get_cursor_pos(int *row, int *col) {
    char buffer[32];
    unsigned int i = 0;

    // Query for current cursor position
    // NOTE: we can retrieve this result from STDIN
    // NOTE: the result is an escape sequence that terminates with 'R'
    // NOTE: example -> "\x1b[30;40R" -> row = 30, col = 40
    if (write(STDOUT_FILENO, "\x1b[6n", 4) != 4)
        return -1;

    // Parse through the result produced in STDIN and put it in 'buffer'
    while (i < sizeof(buffer) - 1) {
        if (read(STDIN_FILENO, &buffer[i], 1) != 1)
            return -1;
        if (buffer[i] == 'R')
            break;
        i++;
    }
    buffer[i] = '\0';

    if (buffer[0] != '\x1b' || buffer[1] != '[')
        return -1;

    // Extract the values from the buffer
    if (sscanf(&buffer[2], "%d;%d", row, col) != 2)
        return -1;

    return 0;
}


/*
-> Fetches the dimensions of the terminal screen
-> Updates the values pointed to by 'rows' and 'cols'
-> Returns 0 in case of success and -1 in case of error
*/
int get_window_size(int *rows, int *cols) {
    struct winsize ws;

    // NOTE: ioctl() will fetch the terminal dimensions and update 'ws'

    // CASE-1: ioctl() fails -> use fallback mechanism
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
        // Move the cursor to bottom right and get the cursor position
        // NOTE: cursor indices start from 1 and not 0
        // NOTE: we try to move the cursor to (999, 999) but it clamps to the border if it goes out of bounds
        if (write(STDOUT_FILENO, "\x1b[999C\x1b[999B", 12) != 12)
            return -1;
        return get_cursor_pos(rows, cols);
    }

    // CASE-2: ioctl() works
    else {
        *rows = ws.ws_row;
        *cols = ws.ws_col;

        return 0;
    }
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
    for (int y = 0; y < E.screenrows; y++) {
        write(STDOUT_FILENO, "~", 1);

        if (y < E.screenrows - 1)
            write(STDOUT_FILENO, "\r\n", 2);
    }
}


// Append a string to an AppendBuffer
void ab_append(AppendBuffer *ab, char *str, int len) {
    char *new = realloc(ab->buffer, ab->len + len);
    if (new == NULL)
        return;

    memcpy(&new[ab->len], str, len);

    ab->buffer = new;
    ab->len += len;
}


// Frees an AppendBuffer in memory
void ab_free(AppendBuffer *ab) {
    free(ab->buffer);
}


// Initialize global editor state
void init_editor(void) {
    // Fetch terminal screen dimensions and handle error
    if (get_window_size(&E.screenrows, &E.screencols) == -1)
        halt("get_window_size");
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
    init_editor();

    while(true) {
        refresh_screen();
        process_keypress();
    }

    return 0;
}
