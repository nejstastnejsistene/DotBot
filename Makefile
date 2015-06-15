CC = gcc
CFLAGS = -Wall -Wextra -Werror -ansi -pedantic -O3

TARGETS = server benchmark compute_cycles
OBJECTS = dots.o
LIBRARIES = -lm -lwebsockets -ljson-c

all: $(TARGETS)

debug: CFLAGS += -g -pg
debug: all

$(TARGETS) : % : %.c $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBRARIES)

%.o: %.c %.h
	$(CC) $(CFLAGS) -c $< $(LIBRARIES)

clean:
	rm -f $(TARGETS) $(OBJECTS)
