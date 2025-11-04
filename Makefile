CFLAGS=-g -std=c11

tim: editor.c
	gcc editor.c -o tim $(CFLAGS)
