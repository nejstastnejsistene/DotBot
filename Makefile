CC=gcc
CFLAGS=-c -g -O3 -Wall

DotBot: DotBot.c emu.o dots.o vector.o set.o
	$(CC) -o DotBot DotBot.c emu.o dots.o vector.o set.o

emu.o: emu.c emu.h
	$(CC) $(CFLAGS) emu.c

dots.o: dots.c dots.h cycles.h
	$(CC) $(CFLAGS) dots.c

vector.o: vector.c vector.h
	$(CC) $(CFLAGS) vector.c

set.o: set.c set.h
	$(CC) $(CFLAGS) set.c

cycles.h: gen_cycles_h.py find_cycles.c
	python gen_cycles_h.py

clean:
	rm -rf *.o DotBot find_cycles
