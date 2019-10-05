CC=gcc
CFLAGS=-lX11 -Wall -Wextra -Werror -pedantic

all: ambilight

ambilight:
	$(CC) ambilight.c -o ambilight $(CFLAGS)