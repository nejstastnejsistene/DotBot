
DotBot: DotBot.c DotBot.h list.c list.h set.c set.h cycles.h
	gcc -g -Wall DotBot.c list.c set.c -o DotBot

cycles.h: gen_cycles_h.py find_cycles.c set.c set.h
	python gen_cycles_h.py

cython:
	python setup.py build_ext --inplace
