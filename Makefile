CC=gcc
CFLAGS=-g -Wall -Wextra -pedantic -ansi -O3

TARGET=DotBot
OBJECTS=grid.o dots.o

$(TARGET): main.c $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c %.h
	$(CC) $(CFLAGS) -c $<

clean:
	rm $(TARGET) $(OBJECTS)
