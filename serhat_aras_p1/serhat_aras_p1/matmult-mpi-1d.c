#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
#include <mpi.h>

#define bufSize 1024

void printMatrix (int **mat,int dim){
    printf("-----------------------------------------------\n");
    printf("+++Content of the matrix\n");
    printf("-----------------------------------------------\n");
    for(int i=0; i < dim; i++)
    {
        for(int j = 0; j < dim; j++)
        {
            printf("%i,",mat[i][j]);
        }
        printf("\n");
    }
        fflush(stdout);
}
int getSize(char* arg){
    /* Open the file for reading */
    char *line_buf = NULL;
    size_t line_buf_size = 0;
    int line_count = 0;
    __ssize_t line_size;
    //MAT1
    FILE *fp = fopen(arg, "r");
    if (!fp){
        fprintf(stderr, "Error opening file '%s'\n", arg);
        return 1;
    }
    /* Get the first line of the file for mat1 */
    line_size = getline(&line_buf, &line_buf_size, fp);
    int dim = atoi(line_buf);
    fclose(fp);
    return dim;
}
int** readMatrix (char* arg){
    /* Open the file for reading */
    char *line_buf = NULL;
    size_t line_buf_size = 0;
    int line_count = 0;
    __ssize_t line_size;
    //MAT1
    FILE *fp = fopen(arg, "r");
    if (!fp){
        fprintf(stderr, "Error opening file '%s'\n", arg);
        return NULL;
    }
    /* Get the first line of the file for mat1 */
    line_size = getline(&line_buf, &line_buf_size, fp);
    int dim1 = atoi(line_buf);
    int **mat1= malloc ((sizeof(int*)*(dim1)));
    for(int i = 0; i < dim1; i++){
        mat1[i] = malloc((sizeof(int)*(dim1)));
    }  
    /* Loop through until we are done with the file. */
    int i=0; 
    while (line_size >= 0){
        /* Increment our line count */
        line_count++;
        int j = 0;
        /* Show the line details */
        if(line_count>1 && i<dim1){
            char * strtok_res;
            strtok_res = strtok(line_buf, " ");
            while (strtok_res != NULL){
                //printf ("-%s\n", strtok_res);
                //printf ("%i , %i\n", i ,j);
                mat1[i][j] = atoi(strtok_res);
                //consume next word
                strtok_res = strtok (NULL, " ");
                j++;
            }
            /*
            * Reading DONE!
            */
            i++;
        }
        /* Get the next line */
        line_size = getline(&line_buf, &line_buf_size, fp);
    }
    free(line_buf);
    line_buf = NULL;
    printf("\n              Reading matrix completed . . .\n");
    fclose(fp);
    return  mat1;
}
int** transpose (int** mat1, int dim1){
    int **mat1_cols = malloc (dim1 * sizeof(int*));
    for(int i = 0; i < dim1; i++){
        mat1_cols[i] = malloc(dim1 * sizeof(int));
        for(int j = 0; j < dim1; j++){
            mat1_cols[i][j] = mat1[j][i]; 
        }  
    }
    printf("                 +++Transpose of Matrix Completed! . . .\n");
    return mat1_cols;
}
int** matMult(int **mat1, int **mat2, int dim1, int dim2){
    if(dim1 == dim2){
        int **result  = malloc( dim1 * sizeof(int *));
        for(int i = 0; i < dim1; i++){
            result[i] = malloc ( sizeof(int) * dim1);
        }
        for(int i=0; i<dim1; ++i)
            for(int j=0; j<dim2; ++j)
            for(int k=0; k<dim1; ++k){
                    result[i][j]+=mat1[i][k]*mat2[k][j];
            }
        return result;
    }else{
        printf("    + + + Matrix dimentions are not compatible for multiplication operation.\nExiting!\n");
        return NULL;
    }
}
int writeResult(int **result,int dim, char* arg){

    FILE *fwrt = fopen(arg, "w");
    if (!fwrt){
        fprintf(stderr, "Error opening file '%s'\n", arg);
        return 1;
    }
    /* write lines into the file stream*/
    for(int i = 0; i < dim+1;i++){
        if(i == 0){
            fprintf (fwrt, "%i\n",dim);
        }else{
            for(int j = 0 ; j < dim ; j++){
                fprintf (fwrt, "%i ",result[i-1][j]); 
            }
            fprintf (fwrt, "\n"); 
        }
    }

    return 0;
}
int main(int argc, char *argv[]){
   clock_t begin = clock();
   int dim1,dim2;
   MPI_Status status;
   MPI_Status readStatus1, readStatus2;
   int numberOfProcess, pid, divider, rows_per_divider;
   MPI_Init (&argc, &argv);
   MPI_Comm_size (MPI_COMM_WORLD, &numberOfProcess);
   MPI_Comm_rank (MPI_COMM_WORLD, &pid);
   
   if(pid == 0){
      printf("- - - - ->Matrix 1");
      int **mat1 = readMatrix(argv[1]);
      dim1 = getSize(argv[1]);
      int **mat1_T = transpose(mat1, dim1);


      printf("- - - - ->Matrix 2");
      int **mat2 = readMatrix(argv[2]);
      dim2 = getSize(argv[2]);
      int **mat2_T = transpose(mat2, dim2);
      int **result = matMult(mat1,mat2,dim1,dim2);
      //Devide the  matrixes

      divider = sqrt(numberOfProcess);
      rows_per_divider = dim1 / divider ;
    
      //Send the Dimentions

      for(int i = 0; i<numberOfProcess; i++){
         MPI_Send(&rows_per_divider, 1,MPI_INT,i, 0, MPI_COMM_WORLD);
         MPI_Send(&dim1, 1,MPI_INT,i, 0, MPI_COMM_WORLD);  
      }
      int ***dev_mat_holder1 = malloc(sizeof(int **) * numberOfProcess);
      int ***dev_mat_holder2 = malloc(sizeof(int **) * numberOfProcess);
      for(int k = 0; k < numberOfProcess; k++){
         dev_mat_holder1[k] = malloc(sizeof(int *) * rows_per_divider);
         dev_mat_holder2[k] = malloc(sizeof(int *) * rows_per_divider);
         for(int i = 0; i < rows_per_divider; i++ ){
            dev_mat_holder1[k][i] = malloc(sizeof(int) * rows_per_divider);
            dev_mat_holder2[k][i] = malloc(sizeof(int) * rows_per_divider);
         }
      }
      for(int process = 0; process < numberOfProcess; process++){
        int index_X = process / divider ; // Ex. 5th process is on the 2nd row by 5/3 = 1 [ 0 1 2]
        int index_Y = process % divider ; 
        int x = index_X * rows_per_divider ;
        int y = index_Y * rows_per_divider ;
        int end_x = x + rows_per_divider;
        int end_y = y + rows_per_divider;
        //printf("Process[%i] x:[%i], x-end:[%i], y:[%i], y-end:[%i] \n",process,x,end_x,y,end_y);
        int lookup_i = 0; 
        for(int i = x ; i < end_x ; i++)
        {
            int lookup_j = 0; 
            for(int j = y; j < end_y ; j++)
            {
                dev_mat_holder1[process][lookup_i][lookup_j] = mat1[i][j];
                dev_mat_holder2[process][lookup_i][lookup_j] = mat2[i][j];
                lookup_j++;
                
            }
            lookup_i++;
        }
        if(process!=0){
            MPI_Send((dev_mat_holder1[process]), rows_per_divider,MPI_INT,process, 1, MPI_COMM_WORLD);
            MPI_Send((dev_mat_holder2[process]), rows_per_divider,MPI_INT,process, 2, MPI_COMM_WORLD);   
        }
    }

        int ** partialSum = calloc(sizeof(int *),rows_per_divider);
        for(int i = 1; i < rows_per_divider; i++ ){
            partialSum[i] = calloc(sizeof(int),rows_per_divider);
        } 
        for(int i = 1; i<numberOfProcess; i++){
           MPI_Recv(partialSum,rows_per_divider,MPI_INT, i, 0,MPI_COMM_WORLD, &status);
            printMatrix(*partialSum,rows_per_divider);
            printf("%i\n",partialSum[i]);
        }
        //printf("asdas");
      // devide each array 
      fflush(stdout);
      free(mat1);
      free(mat1_T);
      free(mat2);
      free(mat2_T);
      free(result);
   }
   if(pid!=0){
    int dim;
    MPI_Recv(&rows_per_divider,1,MPI_INT,0, 0,MPI_COMM_WORLD, &status);
    MPI_Recv(&dim,1,MPI_INT,0, 0,MPI_COMM_WORLD, &status);
    //Send the Dimentions
    printf("Process[%i] Divider %i, dimention %i\n",pid,rows_per_divider,dim);
        int **dev_mat_1 = calloc(sizeof(int *),rows_per_divider);
        int **dev_mat_2 = calloc(sizeof(int *),rows_per_divider);
        for(int i = 0; i < rows_per_divider; i++ ){
            dev_mat_1[i] = calloc(sizeof(int),rows_per_divider);
            dev_mat_2[i] = calloc(sizeof(int),rows_per_divider);
        }
        


        MPI_Recv(*dev_mat_1,rows_per_divider,MPI_INT,0, 1,MPI_COMM_WORLD, &readStatus1);
        
        MPI_Recv(*dev_mat_2,rows_per_divider,MPI_INT,0, 2,MPI_COMM_WORLD, &readStatus2);
        
    
        int small_dim = (dim/(sqrt(numberOfProcess)));
        int **result = matMult(dev_mat_1,dev_mat_1,rows_per_divider,rows_per_divider);
        
        MPI_Send(*result, rows_per_divider,MPI_INT,0, 0, MPI_COMM_WORLD);
    
   }
 
    MPI_Finalize();
    clock_t end = clock();
    double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
    printf("-----------------------------------------------------\nExecution Time of the sum-serial.c ---> %f\n-----------------------------------------------------\n",time_spent);
    return 0;
}