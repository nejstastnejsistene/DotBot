CC = gcc
CFLAGS = -Wall -Wextra -Werror -ansi -pedantic -O3

TARGETS = server benchmark
OBJECTS = dots.o cycles.o
LIBRARIES = -lm -lwebsockets -ljson-c

all: $(TARGETS)

debug: CFLAGS += -g -pg
debug: all

$(TARGETS) : % : %.c $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBRARIES)

%.o: %.c %.h cycle_literals.h
	$(CC) $(CFLAGS) -c $< $(LIBRARIES)

cycle_literals.h: compute_cycles generate_cycle_literals_h.py
	./$< | ./$(word 2,$^) > $@

compute_cycles: compute_cycles.c
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f $(TARGETS) $(OBJECTS) compute_cycles cycle_literals.h
