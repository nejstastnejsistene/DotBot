LWS_PATH=libwebsockets/lib

CC=gcc
CFLAGS=-g -Wall -O3 -I$(LWS_PATH) -L$(LWS_PATH)

TARGET=dotbot
OBJECTS=main.o dots.o grid.o

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^ -lwebsockets

%.o: %.c %.h
	$(CC) $(CFLAGS) -c $<

clean:
	rm $(TARGET) $(OBJECTS)
