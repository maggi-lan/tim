CC      = gcc
CFLAGS  = -g -std=c17 -Wall -Wextra -I include
LDFLAGS = -lncurses

# Source files and object files
SRC = src/main.c \
      src/rope_core.c \
      src/rope_avl.c \
      src/rope_helper.c \
      src/rope_utility.c \
      src/file_io.c

OBJ = $(SRC:.c=.o)

# Main target
tim: $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Compile each .c to a .o
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Debug binary (no ncurses, uses debug/rope_debug.c as entry point)
DEBUG_SRC = debug/rope_debug.c \
            src/rope_core.c \
            src/rope_avl.c \
            src/rope_helper.c \
            src/rope_utility.c \
            src/file_io.c

tim_debug: $(DEBUG_SRC)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f src/*.o tim tim_debug

.PHONY: clean
