CC=gcc
CFLAGS=-Wall -Wextra -Werror -ansi -pedantic -g -O3

all: dotbot benchmark

dotbot: main.c dots.o
	$(CC) $(CFLAGS) -o $@ $^ -lwebsockets -ljson

benchmark: benchmark.c dots.o
	$(CC) $(CFLAGS) -o $@ $^

dots.o: dots.c dots.h
	$(CC) $(CFLAGS) -c $<

clean:
	rm -rf dotbot benchmark *.o
