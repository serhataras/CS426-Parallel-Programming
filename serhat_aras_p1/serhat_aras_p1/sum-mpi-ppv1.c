#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>
#include <time.h>
#define bufSize 2048
 
int main (int argc, char *argv[]){ 
    clock_t begin = clock();
    MPI_Status status;
    MPI_Status readStatusSize, readStatusData;
    int numberOfProcess, pid, i, j;
    MPI_Init (&argc, &argv);
    MPI_Comm_size (MPI_COMM_WORLD, &numberOfProcess);
    MPI_Comm_rank (MPI_COMM_WORLD, &pid);
    if (pid == 0){// Master process  
        printf ("Reading data from Master . . .\n");
        FILE* fp;
        char buf[bufSize];
        if (argc != 2){
            fprintf(stderr,"Usage: %s <soure-file>\n", argv[0]);
            return 1;
        }
        if ((fp = fopen(argv[1], "r")) == NULL){ /* Open source file. */
            perror("fopen source-file");
            return 1;
        }
        int sum = 0, size = 0;
        while (fgets(buf, sizeof(buf), fp) != NULL){
            buf[strlen(buf) - 1] = '\0'; // eat the newline fgets() stores
            size++;    
        }
        printf("---> Data size is %i\n", size);
        //reset the file 
        if ((fp = fopen(argv[1], "r")) == NULL){ /* Open source file. */
            perror("fopen source-file");
            return 1;
        }
        int i=0;
        int *dataArr = (int *) malloc(size* sizeof(int));
        while (fgets(buf, sizeof(buf), fp) != NULL){
            buf[strlen(buf) - 1] = '\0'; // eat the newline fgets() stores
            dataArr[i] = atoi(buf);
            i++;
        }
        fclose(fp);
        printf("\n");
        printf ("Reading completed . . .\n");
        //Reading Finished
        
        /*
        *Dividing the jobs first
        */
        int numOfWorkers = numberOfProcess-1;
        int dataPerWorker = size / numOfWorkers;
        int dataRemained = size % numOfWorkers;
        printf("\n");
        printf("Workers:%i, DataPerWorker:%i, Data to Master:%i\n",numOfWorkers,dataPerWorker,dataRemained);
        printf("------\n");
        //Seperate the the Work
        int **buffers = malloc(numberOfProcess*sizeof(int *));
        for(int i = 1; i <numberOfProcess; i++){
            int startingIndex = ((i-1) * dataPerWorker); 
            int finishIndex = ((i)*dataPerWorker);
            int writeIndex = 0;
            buffers[i-1] = (int *) malloc((finishIndex-startingIndex) * sizeof(int));                    
            for(int j = startingIndex; j < finishIndex; j++){
                buffers[i-1][writeIndex] = dataArr[j];
                writeIndex++;
            }
            /*
            printf("\nContents of the Buffer[%i]\n",i);
            for(int j = 0; j < (dataPerWorker); j++){
                printf("%i,",buffers[i-1][j]);
            }
            printf("\n");
            */
        }
  
        //printf("\nNumber of data Remained for the Master Process is %i\n",dataRemained);
        if(dataRemained>0){
            int start = numOfWorkers*dataPerWorker;
            int index =0;
            buffers[numberOfProcess-1] = (int *) malloc((dataRemained) * sizeof(int));    
            while(start<size&&index<dataRemained){
                buffers[numberOfProcess-1][index]=dataArr[start];
                start++;
                index++;
            }
            /*
            //printf("Contents of the Master\n");
            for(int j = 0; j < (dataRemained); j++){
                printf("%i,",buffers[numberOfProcess-1][j]);
            }
            printf("\n");
            */
        }
         sum=0;
        //Do the Master's work
        for(int k = 0; k < dataRemained; k++){
            sum+=buffers[numberOfProcess-1][k];
        }
        //Send the Works

        printf ("Distributing data to Workers . . .\n");
        for(int i = 1; i<numberOfProcess; i++)
        {
            MPI_Send((void *)&dataPerWorker, 1,MPI_INT,i, 0, MPI_COMM_WORLD);
            MPI_Send((void *)buffers[i-1], dataPerWorker,MPI_INT,i, 0, MPI_COMM_WORLD);
        
        }
        int partialSum;
        printf ("Receiving calculations from Workers . .\n");
        for(int i = 1; i<numberOfProcess; i++)
        {
            MPI_Recv((void *)&partialSum,1,MPI_INT,i, 0,MPI_COMM_WORLD, &status);
            sum += partialSum;
            printf ("[%i] Sends partial sum : %i . . . \n",i,partialSum);
        }
        
        printf("\nSum: %i\n",sum);
        fflush(stdout);
    }
    else{
        int size;
        MPI_Recv((void *)&size,1,MPI_INT,0, 0,MPI_COMM_WORLD, &readStatusSize);
        //printf("Process [%i] recieved size %i \n",pid, size);
        fflush(stdout);
        int *partialData = (int *) malloc(size * sizeof(int));      
        MPI_Recv((void *)partialData,size,MPI_INT,0, 0,MPI_COMM_WORLD, &readStatusData);
        //printf("Process [%i] recieved data\n",pid);
        //printf("Process [%i] recieved data %i \n",pid, partialData[0]);
        fflush(stdout);
        int sum =0;
        
        for(int k = 0; k < size; k++){
            sum+=partialData[k];
        }
        MPI_Send((void *)&sum, 1,MPI_INT,0, 0, MPI_COMM_WORLD);
    }

    MPI_Finalize();
    clock_t end = clock();
    double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
    printf("---------------------------\nExecution Time of the sum-serial.c ---> %f\n---------------------------\n",time_spent);
    return 0;
}