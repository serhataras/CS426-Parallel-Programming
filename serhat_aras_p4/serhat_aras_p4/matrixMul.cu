#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include<sys/wait.h> 
#include<unistd.h> 

__global__ void matrixVectorMultCuda(int* row_ptr, int* col_ind, float* values, float* x,int row_size , int col_size, int value_size ,int threadSize);
__host__ void outer_VecMatMult_Started (struct SparseMatrix* sparse_Matrix, float *x, int row_size, int col_size, int value_size, int NUMBER_OF_CUDA_THREADS ,int NUMBER_OF_REPETITION , int OUTPUT_FLAG);

struct SparseMatrix{
    int *row_ptr;
    int *col_ind;
    float *values;
};

struct SparseMatrix* readSparseMatrix (char* arg, int &row_size, int &col_size, int &value_size){
    FILE * file;
    int i,j, int_tmp;
    float float_tmp;
    struct SparseMatrix *matrix =  (struct SparseMatrix *) malloc(sizeof(struct SparseMatrix));  
        
    if ((file = fopen(arg, "r+")) == NULL)
    {
        printf("ERROR: file open failed\n");
        //return(NULL);
    }
    for(int k = 0; k<3; k++){
        fscanf(file,"%d", &int_tmp);
        switch( k )
        {
            case 0:
                row_size = int_tmp;
               
                break;
            case 1:
                col_size = int_tmp;
                
                break;
            case 2:
                value_size = int_tmp;
               
                break;   
        }
    }
    matrix->row_ptr = (int*) malloc(sizeof(int)*value_size);
    matrix->col_ind = (int*) malloc(sizeof(int)*value_size);
    matrix->values =  (float*) malloc(sizeof(float)*value_size);                     
    for (i = 0; i < (value_size); i++){
        for (j = 0; j < (3); j++){
            switch( j ){
                case 0:
                    fscanf(file,"%d", &int_tmp);
                    matrix->row_ptr[i] = int_tmp-1;
                    break;
                case 1:
                    fscanf(file,"%d", &int_tmp);
                    matrix->col_ind[i] = int_tmp-1;
                    break;
                case 2:
                    fscanf(file,"%f", &float_tmp);
                    matrix->values[i] = float_tmp;
                    break;   
            }
        }
    }    
    return matrix;
}

__global__ void matrixVectorMultCuda(int* row_ptr, int* col_ind, float* values, float* x,int row_size , int col_size, int value_size, int *threadsID_ptr_Mapper, int * numberOfIndexesToProcess){
    //printf("ThreadID: %i\n", col_size);
    int tid=threadIdx.x+blockIdx.x*blockDim.x;
    float sum=0;

    //each thread search its matching row id from the list 
    //thread zero is responsible from row 0 - rowsperthreadsin sparse matrix.
    //since the given format for sparse matrix is actually optimized, this for loop 
    //is also runs on O(n) (non-zero value size); 
    if(tid<row_size){
       
        for(int r_i=0; r_i<numberOfIndexesToProcess[tid];r_i++){
            int tid_x = r_i+threadsID_ptr_Mapper[tid]; 
            sum=0;
            //printf("TID[%i]- %i - ",tid,tid_x );
            for(int i = 0; i<value_size; i++){
                if(row_ptr[i]==tid_x){
                    sum += (float) (x[tid_x]*values[i]);
                }
            }
           //printf("[%e]\n",sum );
            __syncthreads();
            x[tid_x]=sum;  
        }
       
          
    }
    //printf("%i\n",tid);
    // All threads will wait till they come to this point
    // We are now quite confident that all array values are updated.
}

__host__ void  outer_VecMatMult_Started(struct SparseMatrix* sparse_Matrix, float *x, int row_size, int col_size, int value_size, 
    int NUMBER_OF_CUDA_THREADS ,int NUMBER_OF_REPETITION , int OUTPUT_FLAG){
    
    int *dev_row_ptr, *dev_col_ind, *dev_threadsID_ptr_Mapper,*dev_numberOfIndexesToProcess;
    float * dev_values, *dev_x;

    int size = sizeof(int)*value_size;
    int size_f = sizeof(float)*value_size;
    int size_f_x = sizeof(float)* row_size; 
  
        
    if(OUTPUT_FLAG==1){
        printf("Initial Matrix:\n");
            printf("    Values Array: [ %e",x[0]);
            for(int i=1; i<value_size;i++){
                printf(", %e",sparse_Matrix->values[i]);
            }
            printf("]\n    Col_Ind Array: [ %i",sparse_Matrix->col_ind[0]);
            for(int i=1; i<value_size;i++){
                printf(", %i",sparse_Matrix->col_ind[i]);
            }
            printf("]\n    Row_Ptr Array: [ %i",sparse_Matrix->row_ptr[0]);
            for(int i=1; i<value_size;i++){
                printf(", %i",sparse_Matrix->row_ptr[i]);
            }
        printf("]\nVector: [ %e",x[0] );
            for(int i=1; i<value_size;i++){
                printf(", %e",x[i]);
        }
        printf("]\n");
    }

    int partitionSize=sizeof(int)*NUMBER_OF_CUDA_THREADS;
    int *threadsID_ptr_Mapper =(int*) malloc(partitionSize);
    int *numberOfIndexesToProcess = (int*) malloc(partitionSize);
    int threadSize,threadSizeRemaining;
    
    if(NUMBER_OF_CUDA_THREADS>row_size){
         threadSize = 1;
        threadSizeRemaining = 0;
         threadsID_ptr_Mapper[0] = 0;
         numberOfIndexesToProcess[0] = threadSize;
         for(int i = 1; i< NUMBER_OF_CUDA_THREADS; i++){
             threadsID_ptr_Mapper[i] =  threadsID_ptr_Mapper[i-1] +  threadSize;            
             numberOfIndexesToProcess[i] = threadSize;         
             //printf("size[%i]: %i - %i\n", i, threadsID_ptr_Mapper[i] ,numberOfIndexesToProcess[i]);     
         }
          
    }
    else{
            threadSize = value_size/NUMBER_OF_CUDA_THREADS;
            threadSizeRemaining = value_size%NUMBER_OF_CUDA_THREADS;
            threadsID_ptr_Mapper[0] = 0;
            numberOfIndexesToProcess[0] = threadSize;
            for(int i = 1; i< NUMBER_OF_CUDA_THREADS; i++){
                if(i<NUMBER_OF_CUDA_THREADS-1){
                    threadsID_ptr_Mapper[i] =  threadsID_ptr_Mapper[i-1] +  threadSize;
                    numberOfIndexesToProcess[i] = threadSize;
                }
                else{
                    threadsID_ptr_Mapper[i] = threadsID_ptr_Mapper[i-1] + threadSize;
                    numberOfIndexesToProcess[i] = threadSize+threadSizeRemaining ;
                }
                //printf("size[%i]: %i - %i\n", i, threadsID_ptr_Mapper[i] ,numberOfIndexesToProcess[i]);    
            }
    }

    

    cudaMalloc((void**)&dev_numberOfIndexesToProcess, partitionSize);
    cudaMalloc((void**)&dev_threadsID_ptr_Mapper, partitionSize);
    cudaMalloc((void**)&dev_row_ptr, size);
    cudaMalloc((void**)&dev_col_ind, size);
    cudaMalloc((void**)&dev_values, size_f);
    cudaMalloc((void**)&dev_x, size_f_x);
    
    cudaMemcpy(dev_row_ptr, sparse_Matrix->row_ptr, size,cudaMemcpyHostToDevice);
    cudaMemcpy(dev_col_ind, sparse_Matrix->col_ind, size,cudaMemcpyHostToDevice);
    cudaMemcpy(dev_values, sparse_Matrix->values, size,cudaMemcpyHostToDevice);
    cudaMemcpy(dev_threadsID_ptr_Mapper, threadsID_ptr_Mapper, partitionSize,cudaMemcpyHostToDevice);
    cudaMemcpy(dev_numberOfIndexesToProcess, numberOfIndexesToProcess, partitionSize,cudaMemcpyHostToDevice);
    dim3 threadsPerBlock(1, NUMBER_OF_CUDA_THREADS);
   

    for(int i=0; i<NUMBER_OF_REPETITION; i++){
        cudaMemcpy(dev_x, x, size_f_x,cudaMemcpyHostToDevice);
        matrixVectorMultCuda<<<1,NUMBER_OF_CUDA_THREADS>>>(dev_row_ptr,dev_col_ind,dev_values,dev_x,row_size,col_size,value_size,dev_threadsID_ptr_Mapper,dev_numberOfIndexesToProcess);
        cudaMemcpy(x, dev_x, size_f_x,cudaMemcpyDeviceToHost);
        //printf("Iteration %i out of %i Completed\n",i+1,NUMBER_OF_REPETITION);
    }
    if(OUTPUT_FLAG==1||OUTPUT_FLAG==2){
        printf("Resulting Vector: [ %e",x[0] );
        for(int i=1; i<value_size;i++){
            printf(", %e",x[i]);
        }
        printf("]\n");
    }
    cudaFree(dev_numberOfIndexesToProcess);
    cudaFree(dev_threadsID_ptr_Mapper);
    cudaFree(dev_row_ptr);
    cudaFree(dev_col_ind);
    cudaFree(dev_values);
    cudaFree(dev_x);

    free(threadsID_ptr_Mapper);threadsID_ptr_Mapper=NULL;
    free(numberOfIndexesToProcess);numberOfIndexesToProcess=NULL;
}

int main(int argc, char *argv[]){
    clock_t start = clock(), diff;
    int row_size,col_size,value_size;
    
    struct SparseMatrix * sparse_Matrix = readSparseMatrix (argv[4],row_size,col_size,value_size);
    float *x = (float *) malloc(sizeof(float)*row_size);
    for(int i = 0 ; i < row_size ; i++){
        x[i]=1;
    }
    /*printf("%i\n",row_size);
    printf("%i\n",col_size);
    printf("%i\n",value_size);
    printf("%i\n",sparse_Matrix->row_ptr[value_size-2]);
    //Reading Finished*/

    //Cuda zone
    /* initialization */

    int NUMBER_OF_CUDA_THREADS = atoi(argv[1]);
    if(NUMBER_OF_CUDA_THREADS>1024){
        printf("The number of allowed cuda threads is 1024!\nSetting the threads number automatically to 1024.\n------ Resetting the program! ------\n");
        NUMBER_OF_CUDA_THREADS = 1024;
        sleep(1);
    }   
    int NUMBER_OF_REPETITION = atoi(argv[2]);
    int OUTPUT_FLAG = atoi(argv[3]);

    outer_VecMatMult_Started(sparse_Matrix, x, row_size, col_size, value_size, NUMBER_OF_CUDA_THREADS ,NUMBER_OF_REPETITION , OUTPUT_FLAG);
  
    free(sparse_Matrix);
    sparse_Matrix= NULL;

    diff = clock() - start;
    int msec = diff * 1000 / CLOCKS_PER_SEC;
    printf("----\nTime taken: %d seconds %d milliseconds\n", msec/1000, msec%1000);
    return 0;
}

