#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

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

    printf("- - - - ->Matrix 1");
    int **mat1 = readMatrix(argv[1]);
    dim1 = getSize(argv[1]);
    int **mat1_T = transpose(mat1, dim1);
    /* M1 DONE !!! */
    printf("- - - - ->Matrix 2");
    int **mat2 = readMatrix(argv[2]);
    dim2 = getSize(argv[2]);
    int **mat2_T = transpose(mat2, dim2);
    /* M2 DONE !!! */
    int **result = matMult(mat1,mat2,dim1,dim2);
    //printMatrix(result,dim1);
    fflush(stdout);


    if(writeResult(result,dim1,argv[3])==0){
        printf("\n+ + + ->Multiplication Complete!\n+ + ->File Write Completed!\n+ ->Execution Finished!\n");
    }

    /* Close the file now that we are done with it */
    free(mat1);
    free(mat1_T);
    free(mat2);
    free(mat2_T);
    free(result);
    
    clock_t end = clock();
    double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
    printf("-----------------------------------------------------\nExecution Time of the sum-serial.c ---> %f\n-----------------------------------------------------\n",time_spent);
    return 0;
}