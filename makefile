CC=gcc
CFLAGS=-lX11 -lXext -Wall -Wextra -Werror -pedantic -O2 -fomit-frame-pointer #-g

all: ambilight

ambilight: ambilight.c
	$(CC) ambilight.c -o ambilight $(CFLAGS)


dev:
	$(CC) development/ambilight.c -o ambilight $(CFLAGS)


clean:
	rm -f ambilight
