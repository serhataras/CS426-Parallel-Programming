#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>
#include <time.h>
#define bufSize 2048
 
int main (int argc, char *argv[]){  
    clock_t begin = clock();
    MPI_Status status,readStatusSize,readStatusData;
    int numberOfProcess, pid, i, j;
    MPI_Init (&argc, &argv);
    MPI_Comm_size (MPI_COMM_WORLD, &numberOfProcess);
    MPI_Comm_rank (MPI_COMM_WORLD, &pid);
    int *dataArr;
    int sum = 0, size = 0;
    FILE* fp;
    char buf[bufSize];
    if (pid == 0){// Master process - - - Reading 
        printf ("Reading data from Master . . .\n");
        if (argc != 2){
            fprintf(stderr,"Usage: %s <soure-file>\n", argv[0]);
            return 1;
        }
        if ((fp = fopen(argv[1], "r")) == NULL){ /* Open source file. */
            perror("fopen source-file");
            return 1;
        }
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
        fclose(fp);
        printf ("Reading completed . . .\n");
        //Reading Finished
    }
    MPI_Bcast((void *) &size, 1, MPI_INT, 0, MPI_COMM_WORLD);
    dataArr = (int *) malloc(size * sizeof(int));
  
    MPI_Barrier(MPI_COMM_WORLD);
    if (pid == 0) {
        fp = fopen(argv[1], "r");
        int i=0;
        while (fgets(buf, sizeof(buf), fp) != NULL){
            buf[strlen(buf) - 1] = '\0'; // eat the newline fgets() stores
            dataArr[i] = atoi(buf);
            i++;
        }
    }
    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Bcast(*&dataArr, size, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Barrier(MPI_COMM_WORLD);

    for(int i = 0; i < size; i++){
    //printf ("%i . . .\n",dataArr[i]);
      sum += dataArr[i];
    }

    printf("\nProcess[%i] calculates the overall sum: %i\n",pid,sum);
    //Wait for all process to completed up until this point
    //free(dataArr);
    fflush(stdout);
    MPI_Finalize();
    clock_t end = clock();
    double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
    printf("---------------------------\nExecution Time of the sum-serial.c ---> %f\n",time_spent);
    return 0;
}
