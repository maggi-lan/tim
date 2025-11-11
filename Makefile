CFLAGS=-g -std=c17

tim: editor.c
	gcc editor.c -o tim $(CFLAGS)
