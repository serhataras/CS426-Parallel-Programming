#!/bin/bash
> serhat_aras.output
> prof_sequential.txt
> prof_omp.txt

./compile.sh 
printf "Compiled Succesfully\n";
printf "Executing...'Local Binary Pattern Face Recognition System'\n";
for k in 1 2 5 7 10
do
    #echo "\n*\n*\n*---> Sequential Execution for K = $k\n*\n*\n*" >> serhat_aras.output
    ./lbp_seq $k >> serhat_aras.output
    printf "\n|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||" >> prof_sequential.txt
    printf "\n|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||" >> prof_sequential.txt
    printf "\n|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||" >> prof_sequential.txt
    printf "\n||||\n||||\n||||\n||||||||---> Sequential Execution for K = $k\n||||\n||||\n||||" >> prof_sequential.txt
    printf "\n|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||" >> prof_sequential.txt
    printf "\n|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||" >> prof_sequential.txt
    printf "\n|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||\n" >> prof_sequential.txt
    gprof -b lbp_seq gmon.out >> prof_sequential.txt
  
    for num_of_threads in 1 2 4 6 8 16
    do
        #echo "\n*\n*\n*---> Paralel Execution for \n------> K = $k \n------> OMP_NUM_THREADS=$num_of_threads\n*\n*\n*" >> serhat_aras.output
        OMP_NUM_THREADS=$num_of_threads ./lbp_omp $k >> serhat_aras.output

        printf "\n*********************************************************************************************">> prof_omp.txt
        printf "\n*********************************************************************************************" >> prof_omp.txt
        printf "\n*********************************************************************************************" >> prof_omp.txt
        printf "\n****\n****\n****\n********---> Paralel Execution for \n********------> K = $k \n********------> OMP_NUM_THREADS=$num_of_threads\n****\n****\n****" >> prof_omp.txt
        printf "\n*********************************************************************************************" >> prof_omp.txt
        printf "\n*********************************************************************************************" >> prof_omp.txt
        printf "\n*********************************************************************************************\n" >> prof_omp.txt

        gprof -b lbp_omp gmon.out >> prof_omp.txt
    done
done
printf "**Execution Finished Succesfully! \nExiting...'Local Binary Pattern Face Recognition System'\n";