from sklearn.cluster import KMeans
import numpy as np
import sys


def main():
	in_file  = open(sys.argv[2],'r')
	k = int(sys.argv[1])

	contents = in_file.read().split('\n')
	in_file.close()

	all_points = [np.array(list(map(float,x.strip().split(' ')))) for x in contents]
	all_points = np.array(all_points)
	
	kmeans = KMeans(n_clusters=k).fit(all_points)
	predictions = kmeans.predict(all_points)

	with open('scikit-kmeans.txt','w') as fw:
		for i in range(len(all_points)):
			fw.write(str(predictions[i]) + '\n')






if __name__ == '__main__':
	main()