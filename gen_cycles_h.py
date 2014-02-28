import sys
from collections import defaultdict
from commands import getoutput

cycles = defaultdict(lambda: [])
for line in getoutput('bin/find_cycles').split('\n'):
    num_rows, num_cols, cycle = map(eval, line.split(','))
    cycles[num_rows, num_cols].append(cycle)

num_dimensions = len(cycles)
max_length = max(map(len, cycles.values()))

sys.stdout = open('cycles.h', 'w+')

print '/* This file was automatically generated by', __file__, '*/'
print
print '#ifndef CYCLES_H'
print '#define CYCLES_H'
print
print '#include "vector.h"'
print
print '#define SQUARE (3ULL | (3ULL << NUM_ROWS))'
print
print '#define CYCLE_INDEX(row, col) (4 * ((row) - 3) + (col))'
print '#define NUM_CYCLES(row, col) num_cycles[CYCLE_INDEX(row, col)]'
print '#define     CYCLES(row, col)     cycles[CYCLE_INDEX(row, col)]'
print
print 'int get_cycles(vector_t *moves, SET partition, color_t color, SET color_mask);'
print 'void get_convex_hull(SET mask, int *r0, int *c0, int *r1, int *c1);'
print
print 'int num_cycles[{}] = {{'.format(num_dimensions)
for m in range(3, 7):
    for n in range(3, 7):
        print '    {},'.format(len(cycles[m, n]))
print '};'
print
print 'SET cycles[{}][{}] = {{'.format(num_dimensions, max_length)
for m in range(3, 7):
    for n in range(3, 7):
        print     '    /* {} by {} cycles. */'.format(m, n)
        print     '    {'
        for cycle in cycles[m, n]:
            print '        0x{:09x}ULL,'.format(cycle)
        print     '    },'
print '};'
print
print '#endif // CYCLES_H'
