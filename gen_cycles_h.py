import sys
from collections import defaultdict
from commands import getoutput

cc = 'gcc -O3 -Wall find_cycles.c set.c -o find_cycles -DNUM_ROWS={} -DNUM_COLS={}'

print 'Generating all possible cycles. This might take a while...'

all_cycles = defaultdict(lambda: defaultdict(lambda: set()))
for r in range(3, 7):
    for c in range(3, 7):
        cmd = cc.format(r, c)
        print cmd
        err = getoutput(cmd)
        assert not err, '\n\n' + err
        lines = getoutput('./find_cycles').split('\n')

        cardinality = int(lines[0])
        for line in lines[1:]:
            cycle, encircled = line.split()
            all_cycles[cardinality][int(encircled)].add(cycle)

sys.stdout = open('cycles.h', 'w+')

dim1 = len(all_cycles)
dim2 = max(len(x) for x in all_cycles.values())
dim3 = max(len(y) for x in all_cycles.values() for y in x.values())

print '/* This fill was generated automatically by ', __file__, '*/'
print
print '#include "set.h"'
print
print '#define NUM_PERIMETERS', dim1
print '#define CYCLES_DIM_2', dim2
print '#define CYCLES_DIM_3', dim3
print
print 'int perimeters[NUM_PERIMETERS] = {',

for perimeter in all_cycles:
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
            print '            ' + cycle + ','
        print '        },'
    print '    },'
print '};'
