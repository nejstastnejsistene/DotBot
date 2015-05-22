LWS=libwebsockets
LWS_LIB=$(LWS)/lib
LWS_SO=$(LWS_LIB)/$(LWS).so

CC=gcc
CFLAGS=-g -Wall -O3

TARGET=dotbot
OBJECTS=main.o dots.o grid.o

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^ -lwebsockets

%.o: %.c %.h $(LWS_SO)
	$(CC) $(CFLAGS) -c $<

clean:
	rm -rf $(TARGET) $(OBJECTS)
