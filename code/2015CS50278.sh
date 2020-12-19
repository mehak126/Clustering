#!/bin/bash

if [ "$1" = "-kmeans" ]; then
	./kmeans $2 $3
elif [ "$1" = "-dbscan" ]; then
 	./dbscan $2 $3 $4
elif [ "$1" = "-optics" ]; then
	python3 optics.py $4 $3 $2
else
	echo "Incorrect usage"
fi
#sh <RollNo>.sh <inputfile> -plot
