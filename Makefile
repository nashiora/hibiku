CC=gcc
CFLAGS=-I include -std=gnu2x

LIB = $(wildcard ./lib/*.c)

all: hibiku
hibiku: ./src/hibiku.c $(LIB)
	$(CC) -o $@ $^ $(CFLAGS)

clean:
	rm -f ./hibiku
