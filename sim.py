#!/usr/bin/env python

import random
from commands import getoutput

for i in xrange(100):
    print getoutput('bin/simulator -s {}'.format(random.getrandbits(32)))
