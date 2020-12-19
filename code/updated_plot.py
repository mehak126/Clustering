# args-
# 0: data file
# 1: clusters file
# 2: k

import numpy as np
from sklearn.manifold import TSNE
import matplotlib.pyplot as plt
import sys
from sklearn.decomposition import PCA

def get_cmap(n, name='hsv'):
    '''Returns a function that maps each index in 0, 1, ..., n-1 to a distinct 
    RGB color; the keyword argument name must be a standard mpl colormap name.'''
    return plt.cm.get_cmap(name, n)




def main():
	np.random.seed(12)
	data_points_file = open(sys.argv[1],'r');
	cluter_file = open(sys.argv[2],'r');
	k = int(sys.argv[3])

	data_points = data_points_file.read().split('\n')
	data_points_file.close()
	all_points = [np.array(list(map(float,x.strip().split(' ')))) for x in data_points]
	all_points = np.array(all_points)
	
	clusters = list(cluter_file.read().split('#'))
	cluter_file.close()
	clusters.pop(0)
	
	all_clusters = []
	outliers = []
	for cluster in clusters:
		x = cluster.split('\n')
		label = x.pop(0)
		if(x[len(x)-1] == ''):
			x.pop()
		x = np.array(list(map(int,x)))

		if label == 'outlier':
			outliers.append(x)
		else:
			all_clusters.append(x)


	all_clusters = np.array(all_clusters)
	outliers = np.array(outliers)

	colours = get_cmap(k+1)

	for index, cluster in enumerate(all_clusters):
		X = [all_points[x][0] for x in cluster]
		Y = [all_points[x][1] for x in cluster]
		X = np.array(X)
		Y = np.array(Y)
		plt.scatter(X, Y, c = colours(index), s = 0.01)

	X = [all_points[x][0] for x in outliers]
	Y = [all_points[x][1] for x in outliers]
	X = np.array(X)
	Y = np.array(Y)
	plt.scatter(X, Y, c = 'black', s = 0.01)

	plt.show()
	plt.close()

















if __name__ == '__main__':
	main()