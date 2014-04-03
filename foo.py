import sys
import functools
import numpy as np
from collections import Counter, defaultdict


def get_hist(data, turn):
    return Counter(get_turn_data(data, turn)).items()

def get_dist(data, turn):
    return [(x0, x1 / float(len(data))) for x0, x1 in get_hist(data, turn)]

def get_turn_data(data, turn):
    scores = data[:,turn-1]
    if turn > 1:
        scores = scores - data[:,turn-2]
    return scores

def sum_dists(dists):
    return reduce(add_dists, dists)

def add_dists(x, y):
    z = defaultdict(lambda: 0)
    for x0, x1 in x:
        for y0, y1 in y:
            z[x0 + y0] += x1 * y1
    return z.items()


for num_turns in range(1, 46):

    parse_line = lambda x: map(int, x.strip().split())
    with open(sys.argv[1]) as f:
        data = np.array(map(parse_line, f))

    joint_hist = sum_dists(get_hist(data, x+1) for x in range(num_turns))
    joint_dist = sum_dists(get_dist(data, x+1) for x in range(num_turns))

    mu = sum(x * p for x, p in joint_dist)

    sigma = 0
    for x0, x1 in joint_hist:
        sigma += float(x1) * (float(x0) - mu) ** 2
    sigma /= sum(map(lambda x: x[1], joint_hist))
    sigma **= 0.5

    import matplotlib.pyplot as plt
    import matplotlib.mlab as mlab

    xs, ys = zip(*joint_dist)
    norm = mlab.normpdf(xs, mu, sigma)
    d = data[:,num_turns-1]
    plt.figure()
    plt.hist(d, normed=True, bins=max(d)-min(d), alpha=0.1)
    plt.plot(xs, ys, linewidth=5)
    plt.plot(xs, norm, 'r--', linewidth=5)
    plt.savefig('data/{}.png'.format(num_turns))
