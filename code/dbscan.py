# command line arguments
# INPUTS :
# param-1 : input dataset file
# param-2 : ground truth labels (actual classes)
# param-3 : epsilon value to run dbscan from - 0.3
# param-4 : minimum number of sample points - 10
#
# OUTPUTS :
#
# dbscan_gold.txt : results of running the actual dbscan as per the python implementation available in the sklearn

import numpy as np
from sklearn.cluster import DBSCAN
from sklearn import metrics
from sklearn.datasets.samples_generator import make_blobs
from sklearn.preprocessing import StandardScaler
import sys
import matplotlib.pyplot as plt

#params from the command line
dataset_file = sys.argv[1]
labels_file = sys.argv[2]
epsilon_value = float(sys.argv[3])
min_points = int(sys.argv[4])

# #############################################################################
# Load data
X = np.loadtxt(dataset_file)
labels_true = np.loadtxt(labels_file)
X = StandardScaler().fit_transform(X)

# #############################################################################
# Compute DBSCAN
db = DBSCAN(eps=epsilon_value, min_samples=min_points).fit(X)
core_samples_mask = np.zeros_like(db.labels_, dtype=bool)
core_samples_mask[db.core_sample_indices_] = True
labels = db.labels_

# Number of clusters in labels, ignoring noise if present.
n_clusters_ = len(set(labels)) - (1 if -1 in labels else 0)

#output the labels from this dbscan
fileoutput = 'dbscan_gold.txt'
with open(fileoutput, 'a+') as file:
    for label in labels:
        file.write(str(label) + '\n')
    file.close()

print('Estimated number of clusters: %d' % n_clusters_) 
print("Homogeneity: %0.3f" % metrics.homogeneity_score(labels_true, labels))
print("Completeness: %0.3f" % metrics.completeness_score(labels_true, labels))
print("V-measure: %0.3f" % metrics.v_measure_score(labels_true, labels))
print("Adjusted Rand Index: %0.3f" % metrics.adjusted_rand_score(
    labels_true, labels))
print("Adjusted Mutual Information: %0.3f" %
      metrics.adjusted_mutual_info_score(labels_true, labels))
print("Silhouette Coefficient: %0.3f" % metrics.silhouette_score(X, labels))

# #############################################################################
# Plot result

# Black removed and is used for noise instead.
unique_labels = set(labels)
colors = [
    plt.cm.Spectral(each) for each in np.linspace(0, 1, len(unique_labels))
]
for k, col in zip(unique_labels, colors):
    if k == -1:
        # Black used for noise.
        col = [0, 0, 0, 1]

    class_member_mask = (labels == k)

    xy = X[class_member_mask & core_samples_mask]
    plt.plot(
        xy[:, 0],
        xy[:, 1],
        'o',
        markerfacecolor=tuple(col),
        markeredgecolor='k',
        markersize=14)

    xy = X[class_member_mask & ~core_samples_mask]
    plt.plot(
        xy[:, 0],
        xy[:, 1],
        'o',
        markerfacecolor=tuple(col),
        markeredgecolor='k',
        markersize=6)

plt.title('Estimated number of clusters: %d' % n_clusters_)
plt.show()