#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define bufSize 2048

int *documents;
int documentSize;
int dictSize;
int *queryArr;
int **values;

int parseDocument(char* documents, int dictionarySize);
int parseQuery(char* query, int dictionarySize);

void printArray(int arr[], int size); 
void swap(int* a, int* b); 
int partition (int arr[], int low, int high,int dID_copy[]);
void quickSort(int arr[], int low, int high,int dID_copy[]); 

int getDocumentSize();
void setDocumentSize(int size);
void setDictionarySize(int size);
int getDictionarySize();
