
DotBot: DotBot.c DotBot.h list.c list.h cycles.h
	gcc -g -Wall DotBot.c list.c -o DotBot

cycles.h: gen_cycles_h.py find_cycles.c set.c
	python gen_cycles_h.py

cython:
	python setup.py build_ext --inplace
