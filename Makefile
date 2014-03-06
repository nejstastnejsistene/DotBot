CC = gcc
CFLAGS = -c -g -O3 -Wall

MKDIR = mkdir -p
PYTHON = python

SRC = src
BIN = bin

$(BIN)/DotBot: $(SRC)/DotBot.c emu.o dots.o cycles.o vector.o set.o
	$(MKDIR) $(BIN)
	$(CC) -o $@ $^

emu.o: $(SRC)/emu.c $(SRC)/emu.h
	$(CC) $(CFLAGS) $<

dots.o: $(SRC)/dots.c $(SRC)/dots.h $(SRC)/cycles.h
	$(CC) $(CFLAGS) $<

cycles.o: $(SRC)/cycles.c $(SRC)/cycles.h
	$(CC) $(CFLAGS) $<

vector.o: $(SRC)/vector.c $(SRC)/vector.h
	$(CC) $(CFLAGS) $<

set.o: $(SRC)/set.c $(SRC)/set.h
	$(CC) $(CFLAGS) $<

$(SRC)/cycles.h: gen_cycles_h.py $(BIN)/find_cycles
	$(BIN)/find_cycles | $(PYTHON) $< > $@

$(BIN)/find_cycles: $(SRC)/find_cycles.c vector.o set.o
	$(MKDIR) $(BIN)
	$(CC) -o $@ $^

readscreen: readscreen.c readscreen.h
	arm-linux-gnueabi-gcc-4.6 -Wall -g -static -o $@ $< -lm
	adb push $@ /data/local/DotBot/$@

foo: foo.c
	arm-linux-gnueabi-gcc-4.6 -Wall -g -static -o $@ $<
	adb push $@ /data/local/DotBot/$@

clean:
	rm -rf $(SRC)/cycles.h *.o $(BIN) dotbot/*.pyc

.PHONY: clean
