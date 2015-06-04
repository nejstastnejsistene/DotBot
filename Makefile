CC=gcc
CFLAGS=-Wall -Wextra -Werror -ansi -pedantic -g -O3

all: dotbot benchmark

dotbot: main.c dots.o
	$(CC) $(CFLAGS) -o $@ $^ -lm -lwebsockets -ljson

benchmark: benchmark.c dots.o
	$(CC) $(CFLAGS) -o $@ $^ -lm

dots.o: dots.c dots.h
	$(CC) $(CFLAGS) -c $< -lm

clean:
	rm -rf dotbot benchmark *.o
