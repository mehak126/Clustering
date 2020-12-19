# command line arguments
# param-1 : actual label file name
# param-2 : predicted label file name

from sklearn import metrics
import numpy as np
import sys

actual_labels = str(sys.argv[1])
my_labels = str(sys.argv[2])

labels_true = np.loadtxt(actual_labels)
labels = np.loadtxt(my_labels)

print("Homogeneity: %0.3f" % metrics.homogeneity_score(labels_true, labels))
print("Completeness: %0.3f" % metrics.completeness_score(labels_true, labels))
print("V-measure: %0.3f" % metrics.v_measure_score(labels_true, labels))
print("Adjusted Rand Index: %0.3f" % metrics.adjusted_rand_score(
    labels_true, labels))
print("Adjusted Mutual Information: %0.3f" %
      metrics.adjusted_mutual_info_score(labels_true, labels))