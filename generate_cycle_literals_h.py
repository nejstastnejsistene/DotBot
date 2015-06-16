#!/usr/bin/env python

import sys
import itertools

def num_bits(x):
    count = 0
    while x:
        x ^= x & -x
        count += 1
    return count

def read_input():
    for line in sys.stdin:
        yield map(int, line.split())

key = lambda x: num_bits(x[0])

sorted_cycles = sorted(read_input(), key=key)
num_cycles = len(sorted_cycles)

values = []
offsets = []

offset = 0
for num_dots, cycles in itertools.groupby(sorted_cycles, key=key):
    cycles = list(cycles)
    values += cycles
    offsets.append((num_dots, offset))
    offset += len(cycles)

cycles, encircled_dots = zip(*values)

print '/* This file was automatically generated by', __file__, '*/'
print
print '#ifndef CYCLE_LITERALS_H'
print '#define CYCLE_LITERALS_H'
print
print '#define NUM_CYCLES {}'.format(num_cycles)
print
for n, offset in offsets:
    print '#define CYCLES_OFFSET_{} {}'.format(n, offset)
print
print 'static const mask_t cycles[NUM_CYCLES] = {'
for x in cycles:
    print '    (mask_t){},'.format(x)
print '};'
print
print 'static const mask_t encircled_dots[NUM_CYCLES] = {'
for x in encircled_dots:
    print '    (mask_t){},'.format(x)
print '};'
print '#endif'
