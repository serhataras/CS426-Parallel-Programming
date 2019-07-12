#include "utils.h"
int parseQuery(char* queryArg,int dictionarySize){
    FILE* fp;
    char buf[bufSize];
    int size=0;

    if ((fp = fopen(queryArg, "r")) == NULL){ /* Open documentsArg file. */
        perror("fopen source-file");
        return 1;
    }
    while (fgets(buf, sizeof(buf), fp) != NULL){
        buf[strlen(buf) - 1] = '\0'; // eat the newline fgets() stores
        size++;    
    }

    char *line_buf = NULL;
    size_t line_buf_size = 0;
    int line_count = 0;
    __ssize_t line_size;

     //reset document.
    if ((fp = fopen(queryArg, "r")) == NULL){ /* Open documentsArg file. */
        perror("fopen source-file");
        return 1;
    }
    //initialize the arrays
    queryArr = malloc(dictionarySize * sizeof(int));
   // printf ("asdasd");
    char * strtok_res;
    strtok_res = strtok(line_buf, " ");
    while (line_size >= 0){
        line_count++;
        int j = 0;
        if(line_count>1){
            char * strtok_res;
            strtok_res = strtok(line_buf, " ");
            while (strtok_res != NULL){
                queryArr[j] = atoi(strtok_res);
                //consume next word
                strtok_res = strtok (NULL, " ");
                j++;
            }
        /* Get the next line */
        line_size = getline(&line_buf, &line_buf_size, fp);
        }
    }
}
int parseDocument(char* documentsArg, int dictionarySize){
        
    FILE* fp;
    char buf[bufSize];
    int size=0;

    if ((fp = fopen(documentsArg, "r")) == NULL){ /* Open documentsArg file. */
        perror("fopen source-file");
        return 1;
    }
    while (fgets(buf, sizeof(buf), fp) != NULL){
        buf[strlen(buf) - 1] = '\0'; // eat the newline fgets() stores
        size++;    
    }

    setDocumentSize(size);
    //printf("Size: %i \n",getDocumentSize());


    char *line_buf = NULL;
    size_t line_buf_size = 0;
    int line_count = 0;
    __ssize_t line_size;
    
    //reset document.
    if ((fp = fopen(documentsArg, "r")) == NULL){ /* Open documentsArg file. */
        perror("fopen source-file");
        return 1;
    }

    //initialize the arrays
    documents = malloc(getDocumentSize()*sizeof(int));
    values = malloc(getDocumentSize()*sizeof(int*));
    for(int i = 0; i < getDocumentSize() ; i++){
        values[i]  = malloc ( dictionarySize * sizeof(int));
    }

    //read documents.txt
    int i=0; 
    while (line_size >= 0){
        /* Increment our line count */
        line_count++;
        
        /* Show the line details */
        if(line_count>1 && i<getDocumentSize()){
            char * strtok_res;
            strtok_res = strtok(line_buf, ": ");
            documents[i]= atoi(strtok_res);
            //printf ("%i:", documents"[i]);
            int j = 0;
            while (strtok_res != NULL && j<dictionarySize){
                //printf ("%i , %i\n", i ,j);
                strtok_res = strtok (NULL, " ");
                if(strtok_res!=NULL){
                    values[i][j] = atoi(strtok_res);
                    //printf (" %i,",weight[i][j]);
                    j++;
                }
                //consume next word
            }
        j++;
        i++;
        }
        /* Get the next line */
        line_size = getline(&line_buf, &line_buf_size, fp);
    }
    //reading complete


    
}

void setDocumentSize(int size){
    documentSize=size;
}
int getDocumentSize(){
    return documentSize;
}

void setDictionarySize(int size){
    dictSize=size;
}
int getDictionarySize(){
    return dictSize;
}


// A utility function to swap two elements 
void swap(int* a, int* b){ 
    int t = *a; 
    *a = *b; 
    *b = t; 
} 
/* This function takes last element as pivot, places 
   the pivot element at its correct position in sorted 
    array, and places all smaller (smaller than pivot) 
   to left of pivot and all greater elements to right 
   of pivot */
int partition (int arr[], int low, int high,int dID_copy[]){ 
    int pivot = arr[high];    // pivot 
    int i = (low - 1);  // Index of smaller element 
  
    for (int j = low; j <= high- 1; j++) 
    { 
        // If current element is smaller than or 
        // equal to pivot 
        if (arr[j] <= pivot) 
        { 
            i++;    // increment index of smaller element 
            swap(&arr[i], &arr[j]); 
            swap(&dID_copy[i],&dID_copy[j]);

        } 
    } 
    swap(&dID_copy[i + 1],&dID_copy[high]);
    swap(&arr[i + 1], &arr[high]); 
    return (i + 1); 
}   
/* The main function that implements QuickSort 
 arr[] --> Array to be sorted, 
  low  --> Starting index, 
  high  --> Ending index */
void quickSort(int arr[], int low, int high,int dID_copy[]){ 
    if (low < high) 
    { 
        /* pi is partitioning index, arr[p] is now 
           at right place */
        int pi = partition(arr, low, high,dID_copy); 
  
        // Separately sort elements before 
        // partition and after partition 
        quickSort(arr, low, pi - 1,dID_copy); 
        quickSort(arr, pi + 1, high,dID_copy); 
    } 
} 
void printArray(int arr[], int size) 
{ 
    int i; 
    for (i=0; i < size; i++) 
        printf("\n%i\n", arr[i]); 
    printf("\n"); 
} 