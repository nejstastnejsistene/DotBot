#!/usr/bin/env python

import sys
import os
import random
import tempfile
import Queue
from commands import getoutput
from multiprocessing import Queue

output_file = 'data/data-{}.txt'.format(os.getpid())

count = int(sys.argv[1])

num_forks = 1
while count and (count >> num_forks) & 1 == 0:
    num_forks += 1
num_processes = 2 ** num_forks

q = Queue(num_processes)

if os.fork() == 0:
    for i in range(num_forks):
        os.fork()
    with tempfile.NamedTemporaryFile(delete=False) as f:
        r = random.SystemRandom()
        for i in xrange(count / num_processes):
            seed = r.getrandbits(32)
            print >> f, getoutput('bin/simulator -s {}'.format(seed))
        q.put(f.name)
else:
    for i in range(num_processes):
        filename = q.get()
        if not os.path.exists('data'):
            os.mkdir('data')
        with open(filename) as f:
            with open(output_file, 'a+') as g:
                g.write(f.read())
        os.unlink(filename)
