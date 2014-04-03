import sys
import functools
import numpy as np
from collections import Counter, defaultdict


class TurnData(object):

    def __init__(self, data_path):
        parse_line = lambda x: map(int, x.strip().split())
        with open(data_path) as f:
            self.data = np.array(map(parse_line, f))
        self.turn_data = []
        for i in range(self.data.shape[1]):
            turn_data = self.data[:,i]
            if i > 0:
                turn_data = turn_data - self.data[:,i-1]
            hist = Counter(turn_data).items()
            dist = [(x0, x1 / float(len(self.data))) for x0, x1 in hist]
            self.turn_data.append(dist)

    def __getitem__(self, key):
        if isinstance(key, slice):
            return sum_dists(self.turn_data[key])
        else:
            return self.turn_data[key]

    def projected_score(self, turn, score):
        return [(x + score, p) for x, p in self[turn:]]


def sum_dists(dists):
    return reduce(add_dists, dists)

def add_dists(x, y):
    z = defaultdict(lambda: 0)
    for x0, x1 in x:
        for y0, y1 in y:
            z[x0 + y0] += float(x1 * y1)
    return z.items()


def mean(pdf):
    return sum(x * p for x, p in pdf)

def std(pdf):
    mu = mean(pdf)
    return sum(p * (x - mu)**2 for x, p in pdf) ** 0.5

def plot(data, num_turns):
    import matplotlib.pyplot as plt
    import matplotlib.mlab as mlab

    pdf = data[:num_turns]

    xs, ys = zip(*pdf)
    norm = mlab.normpdf(xs, mean(pdf), std(pdf))
    d = data.data[:,num_turns-1]
    plt.figure()
    plt.hist(d, normed=True, bins=max(d)-min(d), alpha=0.1)
    plt.plot(xs, ys, linewidth=5)
    plt.plot(xs, norm, 'r--', linewidth=5)
    plt.savefig('data/{}.png'.format(num_turns))


if __name__ == '__main__':
    data = TurnData(sys.argv[1])
    print 'double data[45][36] = {'
    for turn, row in enumerate(map(dict, data.turn_data), 1):
        print     '    /* Turn {} */'.format(turn)
        print     '    {'
        for i in range(36):
            print '        {!r},'.format(row.get(i + 1, 0))
        print     '    },'
    print '};'
    plot(data, 15)
