
DotBot: DotBot.c DotBot.h list.c list.h
	gcc -g -Wall DotBot.c list.c -o DotBot
cython:
	python setup.py build_ext --inplace
