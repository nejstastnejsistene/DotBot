CC=gcc
CFLAGS=-c -g -O3 -Wall

DotBot: DotBot.c emu.o dots.o cycles.o vector.o set.o
	$(CC) -o DotBot DotBot.c emu.o dots.o cycles.o vector.o set.o

emu.o: emu.c emu.h
	$(CC) $(CFLAGS) emu.c

dots.o: dots.c dots.h cycles.h
	$(CC) $(CFLAGS) dots.c

cycles.o: cycles.c cycles.h
	$(CC) $(CFLAGS) cycles.c

vector.o: vector.c vector.h
	$(CC) $(CFLAGS) vector.c

set.o: set.c set.h
	$(CC) $(CFLAGS) set.c

cycles.h: gen_cycles_h.py find_cycles
	python gen_cycles_h.py

find_cycles: find_cycles.c vector.o set.o
	$(CC) -o find_cycles find_cycles.c vector.o set.o

clean:
	rm -rf cycles.h *.o DotBot find_cycles
