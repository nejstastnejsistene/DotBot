import sys
from collections import defaultdict
from commands import getoutput

cc = 'gcc -O3 -Wall find_cycles.c set.c -o find_cycles -DNUM_ROWS={} -DNUM_COLS={}'

print 'Generating all possible cycles. This might take a while...'

all_cycles = defaultdict(lambda: defaultdict(lambda: set()))
for r in range(3, 7):
    for c in range(3, 7):
        cmd = cc.format(r, c)
        err = getoutput(cmd)
        assert not err, '\n\n' + err
        lines = getoutput('./find_cycles').split('\n')

        cardinality = int(lines[0])
        for line in lines[1:]:
            cycle, encircled = map(eval, line.split())
            for r1 in range(6-r+1):
                for c1 in range(6-c+1):
                    all_cycles[cardinality][encircled].add(cycle << (6*c1+r1))

sys.stdout = open('cycles.h', 'w+')

dim1 = len(all_cycles)
dim2 = max(len(x) for x in all_cycles.values())
dim3 = max(len(y) for x in all_cycles.values() for y in x.values())

print '/* This file was automatically generated by ', __file__, '*/'
print
print '#include "set.h"'
print
print '#define NUM_SQUARES 25'
print
print 'SET SQUARES[NUM_SQUARES] = {'

for c in range(5):
    for r in range(5):
        p = 6 * c + r
        print '    ', hex((3|(3<<6))<<p) + 'ULL,'

print '};'
print
print '#define NUM_PERIMETERS', dim1
print '#define CYCLES_DIM_2', dim2
print '#define CYCLES_DIM_3', dim3
print
print 'int perimeters[NUM_PERIMETERS] = {',

for perimeter in sorted(all_cycles, reverse=True):
    print str(perimeter) + ',', 

print '};'
print
print 'SET cycles[NUM_PERIMETERS][CYCLES_DIM_2][CYCLES_DIM_3] = {'

for perimeter in sorted(all_cycles, reverse=True):
    print '    /* Cycles with', perimeter, 'dots. */'
    print '    {'
    groups = sorted(all_cycles[perimeter], reverse=True)
    for cardinality in groups:
        print '        /* Cycles with', perimeter, 'dots encircling', cardinality, 'dots. */'
        print '        {'
        cycles = sorted(all_cycles[perimeter][cardinality])
        for cycle in cycles:
            print '            ' + hex(cycle) + 'ULL,'
        print '        },'
    print '    },'
print '};'