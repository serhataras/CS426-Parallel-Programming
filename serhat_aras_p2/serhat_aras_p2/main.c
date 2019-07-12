#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
#include "utils.h"
#include "mpi.h"

int* similarity(int** globalVals, int* query, int documentLenght,int dictionaryLenght){
    int *simVals = calloc(documentLenght , sizeof(int));
    for(int i=0; i<documentLenght; i++){
        for(int j = 0; j < dictionaryLenght; j++){
            simVals[i] += pow(globalVals[i][j], query[j]); 
        }
    }
    return simVals;
    fflush(stdout); 
}
void kreduce(int * leastk, int * myids, int * myvals, int k, int world_size, int my_rank){
 
    
    //get the k top k values and return them to the master.
    if(my_rank!=0){
        
        MPI_Send((void*)myids, k, MPI_INT, 0, 7, MPI_COMM_WORLD);
        MPI_Send((void*)myvals, k, MPI_INT, 0, 8, MPI_COMM_WORLD);
       
       
    }else {

        if(my_rank==0){
            
            int *ids = calloc(k*world_size,sizeof(int));
            int *vals = calloc(k*world_size,sizeof(int));
            for(int l = 0; l < k; l++)
            {
                ids[l]=myids[l];
                vals[l]=myvals[l];
            }
        
            for(int i = 1; i < world_size; i++) {
                int * tempIDs=calloc (k,sizeof(int));
                int * tempVals=calloc (k,sizeof(int));

                MPI_Recv((void*)tempIDs, k, MPI_INT, i, 7, MPI_COMM_WORLD, NULL);
                MPI_Recv((void*)tempVals, k, MPI_INT, i, 8, MPI_COMM_WORLD, NULL);
                
                

                for(int l = 0; l < k; l++)
                {
                    ids[(i*k)+l]=tempIDs[l];
                    vals[(i*k)+l]=tempVals[l];
                }
                
                
        }
        
       

        int *newIDIndex=calloc(k*world_size,sizeof(int));
        for(int k = 0; k < k*world_size; k++){
            newIDIndex[k]=ids[k];
        }
        //printArray(ids,k*world_size); 
        quickSort(vals,0,(k*world_size)-1,newIDIndex);
        
       
        fflush(stdout); 
       // printArray(leastk,k);  
     
    
        leastk = vals;
        /*

        for(int i = 0; i < k; i++)
        {
            if(newIDIndex[i]!=0)
            leastk[i]=newIDIndex[i];
        }
        fflush(stdout); fflush(stdout); fflush(stdout); 
         

        //printArray(leastk,k);
        }
        */
    }   
    fflush(stdout); 
    }
}
int main(int argc, char *argv[]){
    //dictionarySize kValuedocuments.txt query.txt
   
    // Initialize the MPI environment
    MPI_Init(&argc, &argv);
    // Get the number of processes
    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    // Get the rank of the process
    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    setDictionarySize(atoi(argv[1]));
   
    int kValue = atoi(argv[2]);

    int *leastK; 
    int *dIDs; 
    int *dValues;

    int numOfIDsPerProcess = 0;
    int *IDStartMaster;
    int **valueStartMaster;
    int dictSizeMaster;
    int sizeofProcessArrMaster;
    double time_spent,totaltime;
    if(world_rank == 0){
        clock_t begin = clock();
        parseDocument(argv[3], getDictionarySize());
        parseQuery(argv[4],getDictionarySize());
        //if it is master recieve all MPI_Recv's add them to the leastK 
        //else if it is slave calculate the similarity with the given myvals, and send it to master
        //int* simVals = similarity(values,queryArr, getDocumentSize(), getDictionarySize());
        clock_t end = clock();
        time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
        printf("Sequential Part: %.3f ms\n",time_spent*1000);
        //Paralel Part
        //partition the similarity value set for parallelization
        numOfIDsPerProcess = (int)getDocumentSize()/world_size;
        int sizeofProcessArr=numOfIDsPerProcess;
        for(int i = 0; i < world_size; i++){
            //create 2 pointers for each process 
            //that marks the document start and value start
            int *IDStart;
            int **valueStart;
            sizeofProcessArr=numOfIDsPerProcess;
            if(i==world_size-1){
                int remainProcess = (int) getDocumentSize() % world_size;
                sizeofProcessArr = numOfIDsPerProcess + remainProcess;
                IDStart= documents +(i*sizeofProcessArr);
                valueStart= values + (i*sizeofProcessArr);
                /*
                for(int  h = 0; h < sizeofProcessArr; h++)
                {   printf("%i :",h);
                   for(int  a = 0; a < getDictionarySize(); a++){
                       printf("-%i",valueStart[h][a]);
                   }
                    printf("\n");
                }*/
            }
            else {
                IDStart= documents +(i*sizeofProcessArr);
                valueStart= values + (i*sizeofProcessArr);
            }
            if(i==0){
                IDStartMaster=IDStart;
                valueStartMaster=valueStart;   
                dictSizeMaster=getDictionarySize();
                sizeofProcessArrMaster=sizeofProcessArr;
            }
            else{
                MPI_Send((void*)&sizeofProcessArr, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
                int dictSize= getDictionarySize();
                MPI_Send((void*)&dictSize, 1, MPI_INT, i, 1, MPI_COMM_WORLD);
                MPI_Send((void*)IDStart, sizeofProcessArr, MPI_INT, i, 2, MPI_COMM_WORLD);
                MPI_Send((void*)&valueStart[0][0], sizeofProcessArr*dictSize, MPI_INT, i, 3, MPI_COMM_WORLD);
                MPI_Send((void*)queryArr, dictSize, MPI_INT, i, 4 , MPI_COMM_WORLD);
            }
            
             
        }
    }
    
    int sizeofProcessArr,dictSize;
    int *IDStart;
    int **valueStart;

    if(world_rank!=0){
        double t1, t2;
            t1 = MPI_Wtime(); 
            MPI_Status status1;
            MPI_Recv((void*)&sizeofProcessArr, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status1);
            MPI_Recv((void*)&dictSize, 1, MPI_INT, 0, 1, MPI_COMM_WORLD, &status1);
            IDStart = calloc(sizeofProcessArr,sizeof(int));
            MPI_Recv((void*)IDStart, sizeofProcessArr, MPI_INT, 0, 2, MPI_COMM_WORLD, &status1);
            
            valueStart = calloc(sizeofProcessArr*dictSize,sizeof(int *));
            for(int j = 0; j < sizeofProcessArr; j++){
                valueStart[j]=calloc(dictSize,sizeof(int));
            }

            MPI_Recv((void*)&valueStart[0][0], sizeofProcessArr*dictSize, MPI_INT, 0, 3, MPI_COMM_WORLD, &status1);
            
            int *query= calloc(dictSize,sizeof(int));
            MPI_Recv((void*)query, dictSize, MPI_INT, 0, 4, MPI_COMM_WORLD, &status1);


            //parseQuery(argv[4],getDictionarySize());
            int *simVal= calloc(sizeofProcessArr,sizeof(int));
            simVal=similarity(valueStart,query,sizeofProcessArr,dictSize);
           
            //printArray(simVal,kValue);
            int *newIDIndex=calloc(sizeofProcessArr,sizeof(int));
            for(int k = 0; k < sizeofProcessArr; k++){
                newIDIndex[k]=IDStart[k];
            }
            quickSort(simVal,0,sizeofProcessArr-1,newIDIndex);
            //printArray(newIDIndex,kValue);
            kreduce(NULL, newIDIndex,simVal,kValue,world_size,world_rank);
            //printArray(simVal,kValue);
            
          

            t2 = MPI_Wtime();
            double p_time = t2-t1;

            MPI_Send((void*)&p_time, 1, MPI_DOUBLE, 0, 9, MPI_COMM_WORLD);
        fflush(stdout); 
    }
     else{
        //MPI_Barrier(MPI_COMM_WORLD);
        parseQuery(argv[4],getDictionarySize());
        int *simVal= calloc(sizeofProcessArrMaster,sizeof(int));
        simVal=similarity(valueStartMaster,queryArr,sizeofProcessArrMaster,dictSizeMaster);
       
        int *newIDIndex=calloc(sizeofProcessArrMaster,sizeof(int));
        for(int k = 0; k < sizeofProcessArrMaster; k++){
            newIDIndex[k]=IDStartMaster[k];
        }
        //quickSort(simVal,0,sizeofProcessArrMaster-1,newIDIndex);
        // printArray(newIDIndex,kValue);
        //int *kleast = calloc(kValue,sizeof(int));
        double t1, t2,p_t_temp;
         t1 = MPI_Wtime();
            
        int *kleast = calloc(kValue,sizeof(int));;
        
        quickSort(simVal,0,sizeofProcessArrMaster-1,newIDIndex);
      
        kreduce(kleast, newIDIndex,simVal,kValue,world_size,world_rank);
        
     
     
        t2 = MPI_Wtime();
        double p_time = t2-t1;
        for(int i = 1; i < world_size; i++)
        {
            MPI_Recv((void*)&p_t_temp, 1, MPI_DOUBLE, i, 9, MPI_COMM_WORLD, NULL);
            
            p_time = p_time+p_t_temp;
        }
        printf("Parallel Part: %.3f ms\n",p_time*1000);
        totaltime=p_time+time_spent;
        printf("Total Time: %.3f ms\n",totaltime*1000);
        
        printArray(kleast,kValue);
        //printArray(kleast,kValue);

   fflush(stdout); 
     } 
        
        
    MPI_Finalize();
    return 0;
}
