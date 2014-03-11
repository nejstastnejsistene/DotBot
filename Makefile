CC = gcc
CFLAGS = -g -O0 -Wall

ARM_CC = arm-linux-gnueabi-gcc-4.6
ARM_CFLAGS = $(CFLAGS) -static

MKDIR = mkdir -p
PYTHON = python

SRC = src
BIN = bin

$(BIN)/DotBot: $(SRC)/DotBot.c emu.o dots.o cycles.o vector.o set.o
	$(MKDIR) $(BIN)
	$(CC) $(CFLAGS) -o $@ $^

emu.o: $(SRC)/emu.c $(SRC)/emu.h $(CC) $(CFLAGS) $<

dots.o: $(SRC)/dots.c $(SRC)/dots.h $(SRC)/cycles.h
	$(CC) $(CFLAGS) -c $<

cycles.o: $(SRC)/cycles.c $(SRC)/cycles.h
	$(CC) $(CFLAGS) -c $<

vector.o: $(SRC)/vector.c $(SRC)/vector.h
	$(CC) $(CFLAGS) -c $<

set.o: $(SRC)/set.c $(SRC)/set.h
	$(CC) $(CFLAGS) -c $<

$(SRC)/cycles.h: gen_cycles_h.py $(BIN)/find_cycles
	$(BIN)/find_cycles | $(PYTHON) $< > $@

$(BIN)/find_cycles: $(SRC)/find_cycles.c vector.o set.o
	$(MKDIR) $(BIN)
	$(CC) -o $@ $^

readscreen: readscreen.c readscreen.h
	$(ARM_CC) $(ARM_CFLAGS) -o $@ $< -lm
	adb push $@ /data/local/DotBot/$@

conf.o: conf.c conf.h
	$(ARM_CC) $(ARM_CFLAGS) -c $<

foo: foo.c conf.o
	$(ARM_CC) $(ARM_CFLAGS) -o $@ $^
	adb push $@ /data/local/DotBot/$@

clean:
	rm -rf $(SRC)/cycles.h *.o $(BIN) dotbot/*.pyc

.PHONY: clean
