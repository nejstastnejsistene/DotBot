
DotBot: DotBot.c DotBot.h list.c list.h cycles.h
	gcc -g -Wall DotBot.c list.c -o DotBot

cycles.h: gen_cycles_h.py
	python gen_cycles_h.py > cycles.h

bitmaps: bitmaps.c set.c set.h
	gcc -g -Wall bitmaps.c set.c -o bitmaps

cython:
	python setup.py build_ext --inplace
