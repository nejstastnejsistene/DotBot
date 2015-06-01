CC=gcc
CFLAGS=-g -Wall -O3

TARGET=dotbot

$(TARGET): main.c dots.o
	$(CC) $(CFLAGS) -o $@ $^ -lwebsockets -ljson

dots.o: dots.c dots.h
	$(CC) $(CFLAGS) -c $<

clean:
	rm -rf $(TARGET) $(OBJECTS)
