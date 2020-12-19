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
#include "KDTreeVectorOfVectorsAdaptor.h"

using namespace std;
using namespace nanoflann;

void run_dbscan(int min_points, double epsilon, const string input_file_name, const string output_file_name);
double get_distance(int one, int two);
void initialise_neighbourhood(double epsilon);
bool expand_cluster(int p, int cluster_label, double epsilon, int min_points);
void delete_vector(vector<int> &v, int value);
void print_vector(vector<int> vec);
void kdtree_demo(const size_t nSamples, const size_t dim, double epsilon);
void print_neighbourhood();

#define UNCLASSIFIED -2
#define NOISE -1

typedef std::vector<std::vector<double>> my_vector_of_vectors_t;
my_vector_of_vectors_t samples;

struct point
{
    int id;                    //unique id of each point
    int label_number;          // -2 -> unclassified, -1 -> noise, >= 0 -> cluster ID
    int neighbour_number;      // number of core points from the neightbourhood of this point earlier deleted from D
    vector<int> neighbourhood; //int ids of all the points which are in epsilon-neighbourhood of this point
};

int main(int argc, char *argv[])
{
    int min_points = atoi(argv[1]);
    double epsilon = atof(argv[2]);
    string infile = argv[3];
    string outfile = "dbscan.txt";
    run_dbscan(min_points, epsilon, infile, outfile);
    return 0;
}

unordered_map<int, point *> point_map;
unordered_map<int, int> retrieve_map;
int number_of_points;
int number_of_dims;

void run_dbscan(int min_points, double epsilon, const string input_file_name, const string output_file_name)
{
    //read the input file here, and generate a map fr
    ifstream inputFile(input_file_name);
    if (!inputFile)
    {
        cout << "Unable to open file";
        exit(1); // terminate with error
    }
    double dimension;
    long absolute_count = 1;
    string str;
    while (getline(inputFile, str))
    {
        point *p = (point *)calloc(1, sizeof(point));
        p->id = absolute_count;
        p->label_number = UNCLASSIFIED; //UNCLASSIFIED
        p->neighbour_number = 0;        // NO NEIGHBOURS DELETED INTITALLY
        //uncomment the line below when using the brute force implementation for finding the neighbourhood
        // p->neighbourhood.emplace_back(absolute_count);
        istringstream inf(str);
        vector<double> point_dimension;
        while (inf >> dimension)
        {
            point_dimension.emplace_back(dimension);
        }
        samples.emplace_back(point_dimension);
        point_map[absolute_count++] = p;
    }
    cout << "DATASET READ." << endl;

    //assigning some globals
    number_of_points = point_map.size();
    number_of_dims = samples[0].size();

    //initialising the neighbourhood value of each point
    initialise_neighbourhood(epsilon);
    cout << "INITIALISATION DONE." << endl;

    //Data stricture is done -> run the dbscan algorithm
    int cluster_label = 0;
    for (int i = 1; i <= number_of_points; i++)
    {
        if (point_map.find(i) != point_map.end() && point_map[i]->label_number == UNCLASSIFIED)
        {
            cout << "CURRENT POINT : " << i << endl;
            if (expand_cluster(i, cluster_label, epsilon, min_points))
            {
                cluster_label ++;
            }
        }
    }
    cout <<  "TOTAL NUMEBR OF CLUSTERS : " << cluster_label << endl;

    //output the results
    unordered_map<int, vector<int>> clusters;
    for(int i = 1; i <= number_of_points; i ++)
    {
        int current_label = -10000;
        if(point_map.find(i) != point_map.end())
        {
            current_label = point_map[i]->label_number;
        }
        else if(retrieve_map.find(i) != retrieve_map.end())
        {
            current_label = retrieve_map[i];
        }

        clusters[current_label].emplace_back(i);
    }

    ofstream outfile;
    outfile.open(output_file_name);
    for(int i = 0; i < cluster_label; i ++)
    {
        outfile << "#" << i << "\n";
        for(int point : clusters[i])
        {
            outfile << point - 1 << "\n";
        }
    }
    if(clusters[NOISE].size() > 0)
    {
        outfile << "#outlier\n";
        for(int point : clusters[NOISE])
        {
            outfile << point - 1 << "\n";
        }
    }
    outfile.close();
    cout << "FILE CREATED." << endl;

    //this is only for checking purposes
    outfile.open("dbscan_check.txt");
    for(int i = 1; i <= number_of_points; i ++)
    {
        int current_label = -10000;
        if (point_map.find(i) != point_map.end())
        {
            current_label = point_map[i]->label_number;
        }
        else if (retrieve_map.find(i) != retrieve_map.end())
        {
            current_label = retrieve_map[i];
        }
        outfile << current_label << endl;
    }
    outfile.close();

    return;
}

//returns the euclidean distance between two points
double get_distance(vector<double> &vec1, vector<double> &vec2)
{
    double distance = 0.0;
    for (int i = 0; i < number_of_dims; i++)
    {
        double diff = vec1[i] - vec2[i];
        distance += (diff * diff);
    }
    return sqrt(distance);
}

//this method is used for intialising the neighbourhood of all the points as per the epsilon value, and updates the point_map as well
void initialise_neighbourhood(double epsilon)
{
    //below is the kd tree used for finding the points within epsilon
    srand(static_cast<unsigned int>(time(nullptr)));
    kdtree_demo(number_of_points, number_of_dims, epsilon);
    // print_neighbourhood();
    // exit(0);

    //below is the bad algorithm used to find the neighbourhood
    // for (int i = 1; i <= number_of_points - 1; i++)
    // {
    //     // cout << i << endl;
    //     for (int j = i + 1; j <= number_of_points; j++)
    //     {
    //         if (get_distance(samples[i - 1], samples[j - 1]) <= epsilon)
    //         {
    //             point_map[i]->neighbourhood.emplace_back(j);
    //             point_map[j]->neighbourhood.emplace_back(i);
    //         }
    //     }
    // }
    // print_neighbourhood();
    // exit(0);
}

bool expand_cluster(int p, int cluster_label, double epsilon, int min_points)
{
    vector<int> seeds = point_map[p]->neighbourhood;
    point_map[p]->neighbour_number += seeds.size();
    if (point_map[p]->neighbour_number < min_points)
    {
        // cout << "1.9" << endl;
        point_map[p]->label_number = NOISE;
        return false;
    }
    else
    {
        point_map[p]->label_number = cluster_label;
        // cout << "2.0" << endl;
        // cout << "p : "  << p << endl;
        vector<int> p_neighbours = point_map[p]->neighbourhood;
        // print_vector(p_neighbours);
        for (int neighbour : p_neighbours)
        {
            // cout << "2.01" << endl;
            // cout << "neighbour : " << neighbour << endl;
            // print_vector(point_map[neighbour]->neighbourhood);
            delete_vector(point_map[neighbour]->neighbourhood, p);
            // print_vector(point_map[neighbour]->neighbourhood);
        }

        retrieve_map[p] = point_map[p]->label_number; //this will help to retrieve it back later
        point_map.erase(p);
        // cout << "point deleted from D : " << p << endl;
        // cout << "2.1" << endl;
        delete_vector(seeds, p);
        // cout << "point deleted from seeds : " << p << endl;
        // cout << "2.2" << endl;
        for (int q : seeds)
        {
            if (point_map[q]->label_number == NOISE)
            {
                point_map[q]->label_number = cluster_label;
                delete_vector(seeds, q);
            }
            else
            {
                point_map[q]->label_number = cluster_label;
                point_map[q]->neighbour_number++;
            }
        }
        // cout << "2.3" << endl;
        while (seeds.size() > 0)
        {
            // cout << "seeds.size() : " << seeds.size() << endl;
            // print_vector(seeds);
            int curPoint = seeds[0];
            // cout << "curPoint : " << curPoint << endl;
            vector<int> curSeeds = point_map[curPoint]->neighbourhood;
            // cout << "curSeeds.size() : " << curSeeds.size() << endl;
            point_map[curPoint]->neighbour_number += curSeeds.size();
            // cout << "2.4" << endl;
            if (point_map[curPoint]->neighbour_number >= min_points)
            {
                // cout << "2.44" << endl;
                for (int q : curSeeds)
                {
                    // cout << "q : " << q << endl;
                    // cout << "2.45" << endl;
                    if (point_map[q]->label_number == UNCLASSIFIED)
                    {
                        point_map[q]->label_number = cluster_label;
                        point_map[q]->neighbour_number++;
                        seeds.emplace_back(q);
                    }
                    else if (point_map[q]->label_number == NOISE)
                    {
                        point_map[q]->label_number = cluster_label;
                    }
                }
                // cout << "2.5" << endl;
                // cout << "curPoint : " << curPoint << endl;
                vector<int> curPoint_neighbours = point_map[curPoint]->neighbourhood;
                for (int neighbour : curPoint_neighbours)
                {
                    // cout << "neighbour : " << neighbour << endl;
                    // print_vector(point_map[neighbour]->neighbourhood);
                    delete_vector(point_map[neighbour]->neighbourhood, curPoint);
                    // print_vector(point_map[neighbour]->neighbourhood);
                }
                retrieve_map[curPoint] = point_map[curPoint]->label_number;
                point_map.erase(curPoint);
                // cout << "point deleted from point_map : " << curPoint << endl;
                // cout << "2.6" << endl;
            }
            delete_vector(seeds, curPoint);
            // cout << "point deleted from seeds : " << curPoint << endl;
            // cout << "2.7" << endl;
        }
        // cout << "2.8" << endl;
        return true;
    }
}

void delete_vector(vector<int> &vec, int value)
{
    vec.erase(std::remove(vec.begin(), vec.end(), value), vec.end());
}

void print_vector(vector<int> vec)
{
    for(int something :  vec)
    {
        cout << something << " ";
    }
    cout << endl;
    return;
}

void kdtree_demo(const size_t nSamples, const size_t dim, double epsilon)
{

    // Generate points:
    // generateRandomPointCloud(samples, nSamples, dim);

    // construct a kd-tree index:
    // Dimensionality set at run-time (default: L2)
    // ------------------------------------------------------------
    typedef KDTreeVectorOfVectorsAdaptor<my_vector_of_vectors_t, double> my_kd_tree_t;

    my_kd_tree_t mat_index(dim /*dim*/, samples, 10 /* max leaf */);
    mat_index.index->buildIndex();

    const double search_radius = static_cast<double>(epsilon * epsilon);
    std::vector<std::pair<size_t, double>> ret_matches;

    nanoflann::SearchParams params;
    //params.sorted = false;

    for(int i = 1; i <= number_of_points; i ++)
    {
        const size_t nMatches = mat_index.index->radiusSearch(&samples[i - 1][0], search_radius, ret_matches, params);
        for(size_t j = 0; j < nMatches; j ++)
        {
            point_map[i]->neighbourhood.emplace_back(ret_matches[j].first + 1);
        }
    }
}

void print_neighbourhood()
{
    for(int i = 1; i <= number_of_points; i ++)
    {
        cout << i << " : ";
        vector<int> vec = point_map[i]->neighbourhood;
        sort(vec.begin(), vec.end());
        for(int j : vec)
        {
            cout << j << " ";
        }
        cout << endl;
    }
}


