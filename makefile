CC=gcc
CFLAGS=-lX11 -lXext -Wall -Wextra -Werror -pedantic

all: ambilight

ambilight:
	$(CC) ambilight.c -o ambilight $(CFLAGS)


clean:
	rm ambilight