CC=gcc
CFLAGS=-Wall -Wextra -Werror -ansi -pedantic -g -O3

TARGET=dotbot

$(TARGET): main.c dots.o
	$(CC) $(CFLAGS) -o $@ $^ -lwebsockets -ljson

dots.o: dots.c dots.h
	$(CC) $(CFLAGS) -c $<

clean:
	rm -rf $(TARGET) *.o
