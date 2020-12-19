// #include <bits/stdc++.h>
#include <iostream>
#include <unordered_map>
#include <set>
#include <vector>
#include <fstream>
#include <algorithm>
#include <sstream>
#include <cmath>
#include "nanoflann.hpp"
#include <ctime>
#include <cstdlib>
using namespace std;

vector<vector<float> > initialize_centres(int k, vector<vector<float> >* data_points);
bool assign_clusters(vector<vector<float> >* data_points, vector<vector<float> >* centres, vector<int>* cluster_assignments);
float calc_dist(vector<float> point1, vector<float> point2);
void update_centres(vector<vector<float> >* centres, vector<int>* cluster_assignments, vector<vector<float> >* data_points);

int main(int argc, char* argv[])
{
	if(argc != 3)
	{
		cout << "Please enter 2 arguments" << endl;
		exit(1);
	}

	int k = atoi(argv[1]);
	string input_file = argv[2];
	string output_file = "kmeans.txt";
	ifstream inFile;
	vector<vector<float> > data_points;

	inFile.open(input_file);
	if(!inFile)
	{
		cout << "File doesn't exist" << endl;
		exit(1);
	}

	string line;
	while(getline(inFile,line))
	{
		vector<float> point;
		istringstream iss(line);
		for(string s; iss >> s; )
			point.push_back(stoi(s));
		data_points.push_back(point);
	}

	int num_points = data_points.size();
	int dimension = data_points[0].size();

	// cout << "num_points = " << num_points << endl;
	// cout << "dimension = " << dimension << endl;

	// initialise centres
	vector<vector<float> > centres = initialize_centres(k, &data_points);

	// cout << "Printing initial centres\n";
	// for(int i=0; i<k; i++)
	// 	cout << centres[i][0] << endl;

	// assign clusters
	vector<int> cluster_assignments(num_points,0);
	bool converged;
	int num_iter = 0;
	
	while(true)
	{
		cout << "iter..." << num_iter << endl;
		// assign clusters
		converged = assign_clusters(&data_points, &centres, &cluster_assignments);
		if(converged)
			break;
		// update centres
		update_centres(&centres, &cluster_assignments, &data_points);
		num_iter ++;
		// if(num_iter == 100)
			// break;
	}

	cout << "num_iter = " << num_iter << endl;

	// print cluster assignments
	// for(int i = 0; i < cluster_assignments.size(); i ++)
	// {
	// 	cout << cluster_assignments[i] << endl;
	// }

	vector<vector<int> >final_assignments(k);

	for(int i = 0; i < num_points; i++)
	{
		int cluster = cluster_assignments[i];
		final_assignments[cluster].push_back(i);
	}

	ofstream o_file;
	o_file.open(output_file);
	for(int i = 0; i < k; i ++)
	{
		o_file << "#" << i << endl;
		for(int j = 0; j < final_assignments[i].size(); j++)
		{
			o_file << final_assignments[i][j] << endl;
		}
	}
	o_file.close();

	// ofstream check_file;
	// check_file.open("kmeans_labels.txt");
	// for(int i = 0; i < num_points; i ++)
	// {
	// 	check_file << cluster_assignments[i];
	// 	if(i != num_points-1)
	// 		check_file << endl;
	// }
	// check_file.close();





}


vector<vector<float> > initialize_centres(int k, vector<vector<float> >* data_points)
{
	// random initialisation --> replace with kmeans++?
	
	// vector<int> indices;
	// int num_points = (*data_points).size();
	// while(indices.size() != k)
	// {
	// 	int index = rand()%num_points;
	// 	if(find(indices.begin(), indices.end(), index) == indices.end())
	// 		indices.push_back(index);
	// }

	// // cout << "printing indices\n";
	// // for(int i=0; i<k; i++)
	// // 	cout << indices[i] << endl;

	// vector<vector<float> > initial_centres;
	// for(int i=0; i<k; i++)
	// {
	// 	initial_centres.push_back( (*data_points)[indices[i]] );
	// }

	// return initial_centres;


	// k-means++
	// cout << "Initial cluster centre indices" << endl;
	vector<int> indices;
	int num_points = (*data_points).size();
	int first = rand()%num_points;
	indices.push_back(first);
	// cout << first << endl;


	while(indices.size() != k)
	{
		vector<float> distances;
		float total_dist = 0;
		for(int i = 0; i < num_points; i ++)
		{
			if(find(indices.begin(), indices.end(), i) != indices.end()) //if that point is already a centre
				continue;

			vector<float> point = (*data_points)[i];
			float min_dist = calc_dist(point, (*data_points)[indices[0]]); //from 0th center

			for(int j = 1; j < indices.size(); j++)
			{
				float dist = calc_dist(point, (*data_points)[indices[j]]);
				if(dist < min_dist)
					min_dist = dist; 	
			}

			total_dist += pow(min_dist,2);
			distances.push_back(pow(min_dist,2));

		}


		vector<float> cum_dist;
		cum_dist.push_back(0.0);

		double r = ((double) rand() / (RAND_MAX));
		// cout << "r = " << r << endl;

		for(int i = 0; i < num_points; i++)
		{
			
			cum_dist.push_back(cum_dist[i] + float(distances[i]/total_dist) );
			if(find(indices.begin(), indices.end(), i) != indices.end()) //if that point is already a centre
				continue;

			if(r <= cum_dist[i+1])
			{
				indices.push_back(i);
				// cout << i << " dist " << cum_dist[i+1] << endl;
				break;
			}
		}


	}
	// cout << "DONE!" << endl;
	// exit(1);
	vector<vector<float> > initial_centres;
	for(int i=0; i<k; i++)
	{
		initial_centres.push_back( (*data_points)[indices[i]] );
	}
	return initial_centres;
	
}

bool assign_clusters(vector<vector<float> >* data_points, vector<vector<float> >* centres, vector<int>* cluster_assignments)
{
	bool converged = true;
	// for each point, calculate nearest centre
	int num_points = (*data_points).size();
	int dimension = (*data_points)[0].size();
	int k = (*centres).size();
	int i,j;
	
	for(i = 0; i < num_points; i ++) //for each point
	{
		float min_dist = calc_dist((*data_points)[i], (*centres)[0]); //initially assign it cluster 0
		int new_cluster = 0;
		for(j = 1; j < k; j++)
		{
			float dist = calc_dist((*data_points)[i], (*centres)[j]);
			if(dist < min_dist)
			{
				min_dist = dist;
				new_cluster = j;
			}
		}

		if( (*cluster_assignments)[i] != new_cluster)
		{
			converged = false;
			(*cluster_assignments)[i] = new_cluster;
		}

	}
	return converged;

}


float calc_dist(vector<float> point1, vector<float> point2) 
{
	int dimension = point1.size();
	float dist = 0;
	for(int i = 0; i < dimension ; i ++ )
	{
		dist += pow( abs(point1[i] - point2[i]), 2);
	}
	return sqrt(dist);
}


void update_centres(vector<vector<float> >* centres, vector<int>* cluster_assignments, vector<vector<float> >* data_points)
{
	int k = (*centres).size();
	int dimension = (*centres)[0].size();
	int num_points = (*data_points).size();
	vector<vector<float> >new_centres (k, vector<float> (dimension, 0.0));
	vector<int> points_per_cluster(k,0);

	for(int i = 0 ; i < num_points; i++)
	{
		int cluster = (*cluster_assignments)[i];
		points_per_cluster[cluster] += 1;
		transform(new_centres[cluster].begin(), new_centres[cluster].end(), (*data_points)[i].begin(), new_centres[cluster].begin(), plus<float>());
	}

	for(int i = 0; i < k; i ++)
	{
		for(int j = 0; j < dimension; j ++)
		{
			new_centres[i][j] = new_centres[i][j] / points_per_cluster[i];
			(*centres)[i][j] = new_centres[i][j];
		}
		// transform(new_centres[i].begin(), new_centres[i].end(), new_centres[i].begin(), bind(multiplies<T>(), placeholder::_1, float(1.0/points_per_cluster[i]) ));
	}

	// centres = &new_centres;
	// *centres = new_centres;

}





