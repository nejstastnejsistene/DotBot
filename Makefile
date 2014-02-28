CC=gcc
CFLAGS=-c -g -O3 -Wall

bin/DotBot: DotBot.c emu.o dots.o cycles.o vector.o set.o
	mkdir -p bin
	$(CC) -o bin/DotBot DotBot.c emu.o dots.o cycles.o vector.o set.o

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

cycles.h: bin/find_cycles gen_cycles_h.py
	python gen_cycles_h.py

bin/find_cycles: find_cycles.c vector.o set.o
	mkdir -p bin
	$(CC) -o bin/find_cycles find_cycles.c vector.o set.o

clean:
	rm -rf cycles.h *.o bin
