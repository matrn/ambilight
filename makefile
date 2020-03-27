CC=gcc
CFLAGS=-lX11 -lXext -Wall -Wextra -Werror -pedantic -g

all: ambilight

ambilight:
	$(CC) ambilight.c -o ambilight $(CFLAGS)


dev:
	$(CC) development/ambilight.c -o ambilight $(CFLAGS)


clean:
	rm -f ambilight
