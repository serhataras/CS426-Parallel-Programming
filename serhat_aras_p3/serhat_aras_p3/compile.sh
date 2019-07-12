#!/bin/bash
echo "Compiler Initialized"
gcc -Wall -pg -o lbp_seq lbp_seq.c util.c -lm
gcc -Wall -fopenmp -pg -o lbp_omp lbp_omp.c util.c -lm