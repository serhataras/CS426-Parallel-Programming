#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
#include "util.h"
#include <float.h>
#include <time.h>

void create_histogram(int * hist, int ** img, int num_rows, int num_cols);
//Creates a histogram for image given by int **img and 
//returns histogram as int * hist

double distance(int * a, int *b, int size);
//Finds the distance between two vectors

int find_closest(int ***training_set, int num_persons, 
                int num_training, int size, int *test_image);
//Finds the closest histogram for test image's 
//histogram from training set histograms
//Returns person id of the closest histogram