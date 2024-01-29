CC=gcc
CFLAGS=-I include -std=gnu2x -ggdb -Wformat -Wextra -Wall -Wpedantic -Wno-unused -Werror=return-type

LIB = $(wildcard ./lib/*.c)
HEADERS = $(wildcard ./include/*.h) $(wildcard ./lib/*.h)

all: hibiku
hibiku: ./src/hibiku.c $(LIB) $(HEADERS)
	$(CC) -o $@ ./src/hibiku.c $(LIB) $(CFLAGS)

clean:
	rm -f ./hibiku
