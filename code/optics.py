import numpy as np
import heapq
import itertools
import sys
from scipy import spatial
from matplotlib import pyplot as plt


def optics(filename, eps, minPts):
    output = []
    indices = []
    DB = np.loadtxt(filename)
    ckdtree = spatial.cKDTree(DB, copy_data=True)
    DB = np.hstack((DB, np.zeros((DB.shape[0], 1)))) # column - whether it is processed
    DB = np.hstack((DB, np.full((DB.shape[0], 1), np.inf))) # column - reachability_distances (initialize with undefined/inf)

    for i, p in enumerate(DB):
        print(i)
        if p[-2] == 1:
            continue
        N = get_neighbours(p, eps, DB, kd_tree=ckdtree)
        p[-2] = 1
        output.append(p)
        indices.append(i)
        core_dist_p = core_dist(p, eps, minPts, DB, ckdtree, N)
        if core_dist_p != -1:
            seeds = PriorityQueue()
            update(N, p, seeds, core_dist_p, DB)
            while True:
                try:
                    q_idx = seeds.pop_task()
                    q = DB[q_idx]
                except KeyError:
                    break
                N_prime = get_neighbours(q, eps, DB, kd_tree=ckdtree)
                q[-2] = 1
                output.append(q)
                indices.append(q_idx)
                core_dist_q = core_dist(q, eps, minPts, DB, ckdtree, N_prime)
                if core_dist_q != -1:
                    update(N_prime, q, seeds, core_dist_q, DB)
    return output, indices


def get_neighbours(p, eps, DB, kd_tree):
    return kd_tree.query_ball_point(p[:-2], eps, n_jobs = -1)


def core_dist(p, eps, minPts, DB, kd_tree, nbrs):
    if len(nbrs) < minPts:
        return -1
    else:
        dd, ii = kd_tree.query(p[:-2], k=[minPts-1], n_jobs = -1)
        return dd


def update(N, p, seeds, coredist, DB):
    for o in N:
        if DB[o][-2] != 1:
            new_reach_dist = max(coredist, dist(p, DB[o]))
            if np.isinf(DB[o][-1]):
                DB[o][-1] = new_reach_dist
                seeds.add_task(o, new_reach_dist)
            else:
                if new_reach_dist < DB[o][-1]:
                    DB[o][-1] = new_reach_dist
                    seeds.remove_task(o)
                    seeds.add_task(o, new_reach_dist)
    return


def dist(p, q):
    return np.sqrt(sum([(x-y)**2 for (x,y) in zip(p[:-2],q[:-2])]))


# taken from python documentation
class PriorityQueue:

    def __init__(self):
        self.pq = []  # list of entries arranged in a heap
        self.entry_finder = {}  # mapping of tasks to entries
        self.REMOVED = '<removed-task>'  # placeholder for a removed task
        self.counter = itertools.count()  # unique sequence count

    def add_task(self, task, priority=0):
        'Add a new task or update the priority of an existing task'
        if task in self.entry_finder:
            self.remove_task(task)
        count = next(self.counter)
        entry = [priority, count, task]
        self.entry_finder[task] = entry
        heapq.heappush(self.pq, entry)

    def remove_task(self, task):
        'Mark an existing task as REMOVED.  Raise KeyError if not found.'
        entry = self.entry_finder.pop(task)
        entry[-1] = self.REMOVED

    def pop_task(self):
        'Remove and return the lowest priority task. Raise KeyError if empty.'
        while self.pq:
            priority, count, task = heapq.heappop(self.pq)
            if task is not self.REMOVED:
                del self.entry_finder[task]
                return task
        raise KeyError('pop from an empty priority queue')


# def extract_clusters(reach_distances, t):
#     steep_upward_pts = [idx for idx, dist in enumerate(reach_distances) if reach_distances[idx] <= (1-t)*reach_distances[idx+1] and idx < len(reach_distances - 1)]
#     steep_downward_pts = [idx for idx, dist in enumerate(reach_distances) if reach_distances[idx] >= (1+t)*reach_distances[idx+1] and idx < len(reach_distances - 1)]
#
#     sua = []
#     sda = []

filename = sys.argv[1]
epsilon = float(sys.argv[2])
minPts = int(sys.argv[3])
ordering, indices = optics(filename, epsilon, minPts)
reach_distances = [x[3] for x in ordering]

plt.figure()
plt.plot(reach_distances)
plt.title("Reachability plot for eps = " + str(epsilon) + ", minPts = " + str(minPts))
plt.xlabel("Point")
plt.ylabel("Reachability Distance")
plt.show()

outfile = open("indices.txt", "w")
for i in indices:
    outfile.write(str(i) + "\n" )
outfile.close()

