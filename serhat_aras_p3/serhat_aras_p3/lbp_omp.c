#include "lbp_seq.h"
#include <omp.h>

//Creates a histogram for image given by int **img and 
//returns histogram as int * hist
void create_histogram(int * hist, int ** img, int num_rows, int num_cols){

    int **padded_image = alloc_2d_matrix(num_rows+2,num_cols+2);
   
    for (int i = 0; i < num_rows+2; i++)
    {
        for (int j = 0; j < num_cols+2; j++)
        {
            if(i==0||j==0||i==(num_rows+1)||j==(num_cols+1)){
                padded_image [i][j] = 0;
            }
            else{
                padded_image[i][j]=img[i-1][j-1];
            }
                
        }
    
    }
   
    
    #pragma omp parallel for collapse(2)
    for(int i = 1; i <= num_rows; i++){
        for(int j = 1; j <= num_cols; j++){
            int pixelVal = padded_image[i][j];
            //int binaryIndex=7;
            int histVal = 0;
            //travels neighbors
            for(int nb = 0; nb <= 7; nb++){
                int x=0;
                int y=0;
                switch(nb) {
                case 0 :
                    x = i;
                    y = j-1;
                    if(padded_image[x][y]>=pixelVal){
                        histVal=histVal+1;
                    }   
                    break;
                case 1 :
                    x = i+1;
                    y = j-1;
                  
                    if(padded_image[x][y]>=pixelVal){
                        histVal =histVal+2;
                    }  
                    break;
                case 2 :
                    x = i+1;
                    y = j;
                  
                    if(padded_image[x][y]>=pixelVal){
                        histVal =histVal+4;
                    }  
                    break;
                case 3 : 
                    x = i+1;
                    y = j+1;
                    
                    if(padded_image[x][y]>=pixelVal){
                        histVal =histVal+8;
                    }  
                    break;
                case 4 :
                    x = i;
                    y = j+1;
                   
                    if(padded_image[x][y]>=pixelVal){
                        histVal =histVal+16;
                    }  
                    break;
                case 5 :
                    x = i-1;
                    y = j+1;
                 
                    if(padded_image[x][y]>=pixelVal){
                        histVal =histVal+32;
                    }  
                    break;
                case 6 :
                    x = i-1;
                    y = j;
                
                    if(padded_image[x][y]>=pixelVal){
                        histVal =histVal+64;
                    }  
                    break;
                case 7 :
                    x = i-1;
                    y = j-1;
                    
                    if(padded_image[x][y]>=pixelVal){
                        histVal =histVal+128;
                    }  
                    break;
                default :
                    printf("Invalid\n" );
                } 
            }
            #pragma omp atomic  
            hist[histVal]= hist[histVal]+1 ;   
        }
    }
    dealloc_2d_matrix(padded_image,num_rows+2,num_cols+2);
    /*
    printf("ID:%i Pic: %i\n",i,j);*/
}
//Finds the distance between two vectors
double distance(int *a, int *b, int size){
    float dist=0;
    #pragma omp parallel for
    for (int i = 0; i < size; i++){
       if(a[i]==0 && 0==b[i]){
            dist+=0;
        }else{
            if(abs(a[i]+b[i]==0)){
                dist+=0;
            }
            else{
                #pragma omp atomic
                dist+=pow((a[i]-b[i]),2)/(2*(a[i]+b[i])); //((a[i]-b[i])*(a[i]-b[i]))
            }
        }

    }
    return (double)dist;
}
//Finds the closest histogram for test image's 
//histogram from training set histograms
//Returns person id of the closest histogram
int find_closest(int ***training_set, int num_persons, 
                int num_training, int size, int *test_image){
    
    int personID=0;
    double dist = DBL_MAX;
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < num_persons; i++){
        //#pragma omp parallel for collapse
        for (int j = 0; j < num_training; j++){ 
            double tempDist = distance(training_set[i][j],test_image,size);
            #pragma omp critical
            if(tempDist<dist){
                dist = tempDist;
                personID = i;
            }
        }
    }
    return personID+1;
}

int main(int argc, char *argv[]){
    
    if (argv[1]==NULL) {
        printf("Invalid K, please enter a number and restart the program!\n");
    }
    else{   
        clock_t begin = clock();
        /* here, do your time-consuming job */
        int k = atoi(argv[1]);
        char *pref="./images/";
        char *suff=".txt";
        double wrongRes = 0;
        //allocation done in sequential execution
        int *** histValues = (int ***)malloc(18*sizeof(int**));
        for (int i = 0; i< 18; i++){
            histValues[i] = (int **) malloc(k * sizeof(int *));
            for (int j = 0; j < k; j++){
                histValues[i][j] = (int *)malloc(256*sizeof(int));
                for (int m = 0; m < 256; m++){
                    histValues[i][j][m]=0;
                }
            }
        }
        clock_t end = clock();

        double seq_time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
        
        //start parallel region
        double omp_start = omp_get_wtime( );

        //train
        for(int ID = 0; ID < 18; ID++){
            for(int i = 0; i < k; i++){
                begin = clock();
                //create search key
                char tag[19];
                sprintf(tag,"%s%i.%i%s",pref,ID+1,i+1,suff);
      
                //allocation of the histogram array
                int *hist= malloc(sizeof(int)*256);
                for (int m = 0; m < 256; m++){
                    hist[m]=0;
                }
                end = clock();
                seq_time_spent += (double)(end - begin) / CLOCKS_PER_SEC;
                int **img = read_pgm_file(tag,200,180);

                //Create Histogram For the images and store them
                create_histogram(hist, img, 200, 180);
                for (int m = 0; m < 256; m++){
                      histValues[ID][i][m]=hist[m];
                }
                //Free the allocated arrays
                free(hist);
                dealloc_2d_matrix(img,200,180);
            }
        }
        //test
        for(int ID = 0; ID < 18; ID++){
            for(int i = k; i < 20; i++){
                begin = clock();
                //create search key
                char tag[19];
                char test_tag[19];
                sprintf(test_tag,"%i.%i%s",ID+1,i+1,suff);
                
                sprintf(tag,"%s%i.%i%s",pref,ID+1,i+1,suff);
               
                //allocation of the histogram array
                int *hist= malloc(sizeof(int)*256);  
                for (int m = 0; m < 256; m++){
                    hist[m]=0;
                }
                end = clock();
                seq_time_spent += (double)(end - begin) / CLOCKS_PER_SEC;

                //Read the img for the tag
                int **img = read_pgm_file(tag,200,180);
                //Create Histogram For the image
                create_histogram(hist, img, 200, 180);
                //get the closest ID from the training set
                int test_result_person_id = find_closest(histValues,18,k,256,hist);
                
                //Count the Wrong Guesses
                if(test_result_person_id != ID+1)
                    wrongRes++;
                
                printf("%s %i %i \n",  test_tag,test_result_person_id,ID+1 );

                //Free the allocated array
                free(hist);
                dealloc_2d_matrix(img,200,180);
            }
        }

        double omp_end = omp_get_wtime();// end the timer

        //results
        
        //double pers = (wrongRes/ totalTest)*100;
        printf("Accuracy: %i correct answers for %i tests \n", (int)((20-k)*18-wrongRes),(20-k)*18);       
        double parallel_exec_time = (omp_end - omp_start) ;
        printf("Parallel time: %2.2f ms\n", parallel_exec_time * 1000);
        printf("Sequential time: %2.2f ms\n", seq_time_spent * 1000);

    }
    
    return 0;
}
