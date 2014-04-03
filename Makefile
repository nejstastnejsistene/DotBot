CC = gcc
CFLAGS = -g -Wall

ARM_CC = arm-linux-androideabi-gcc
ARM_CFLAGS = $(CFLAGS) -static -Iinclude

MKDIR = mkdir -p
PYTHON = python

SRC = src
BIN = bin

default: all

all: $(BIN)/DotBot $(BIN)/simulator $(BIN)/readscreen $(BIN)/sendevents

$(BIN)/DotBot: $(SRC)/DotBot.c moves.o dots.o cycles.o vector.o set.o
	$(MKDIR) $(BIN)
	$(CC) $(CFLAGS) -o $@ $^

$(BIN)/simulator: $(SRC)/simulator.c emu.o moves.o dots.o cycles.o vector.o set.o
	$(MKDIR) $(BIN)
	$(CC) $(CFLAGS) -o $@ $^

moves.o: $(SRC)/moves.c $(SRC)/moves.h
	$(CC) $(CFLAGS) -c $<

dots.o: $(SRC)/dots.c $(SRC)/dots.h $(SRC)/litcycles.h
	$(CC) $(CFLAGS) -c $<

cycles.o: $(SRC)/cycles.c $(SRC)/cycles.h
	$(CC) $(CFLAGS) -c $<

vector.o: $(SRC)/vector.c $(SRC)/vector.h
	$(CC) $(CFLAGS) -c $<

set.o: $(SRC)/set.c $(SRC)/set.h
	$(CC) $(CFLAGS) -c $<

$(SRC)/litcycles.h: gen_cycles_h.py $(BIN)/find_cycles
	$(BIN)/find_cycles | $(PYTHON) $< > $@

$(BIN)/find_cycles: $(SRC)/find_cycles.c vector.o set.o
	$(MKDIR) $(BIN)
	$(CC) -o $@ $^

$(BIN)/readscreen: $(SRC)/readscreen.c $(SRC)/readscreen.h conf.o monkey.o
	$(ARM_CC) $(ARM_CFLAGS) -o $@ $< conf.o monkey.o -lm
	adb push $@ /data/local/DotBot/.

conf.o: $(SRC)/conf.c $(SRC)/conf.h
	$(ARM_CC) $(ARM_CFLAGS) -c $<

monkey.o: $(SRC)/monkey.c $(SRC)/monkey.h
	$(ARM_CC) $(ARM_CFLAGS) -c $<

$(BIN)/sendevents: $(SRC)/sendevents.c conf.o monkey.o
	$(ARM_CC) $(ARM_CFLAGS) -o $@ $^
	adb push $@ /data/local/DotBot/.

d: data.c data.h
	$(CC) -o $@ $<

clean:
	rm -rf $(SRC)/litcycles.h *.o $(BIN) dotbot/*.pyc

.PHONY: clean
