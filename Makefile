
DotBot: DotBot.c DotBot.h list.c list.h cycles.h
	gcc -g -Wall DotBot.c list.c -o DotBot

cycles.h: gen_cycles_h.py
	python gen_cycles_h.py > cycles.h

cython:
	python setup.py build_ext --inplace
