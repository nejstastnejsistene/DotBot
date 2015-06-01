CC=gcc
CFLAGS=-g -Wall -O3

TARGET=dotbot
OBJECTS=dots.o grid.o

$(TARGET): main.c $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^ -lwebsockets -ljson

%.o: %.c %.h
	$(CC) $(CFLAGS) -c $<

clean:
	rm -rf $(TARGET) $(OBJECTS)
