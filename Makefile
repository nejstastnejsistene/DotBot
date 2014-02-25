
DotBot: DotBot.c DotBot.h set.c set.h vector.c vector.h cycles.h
	gcc -g -O3 -Wall DotBot.c set.c vector.c -o DotBot

cycles.h: gen_cycles_h.py find_cycles.c set.c set.h
	python gen_cycles_h.py
