#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#define bufSize 1024
 
int main(int argc, char *argv[])
{

   clock_t begin = clock();

   FILE* fp;
   char buf[bufSize];
   if (argc != 2)
   {
      fprintf(stderr,
            "Usage: %s <soure-file>\n", argv[0]);
      return 1;
   }
   if ((fp = fopen(argv[1], "r")) == NULL)
   { /* Open source file. */
      perror("fopen source-file");
      return 1;
   }

   int sum = 0, size = 0;
   while (fgets(buf, sizeof(buf), fp) != NULL)
   {
      buf[strlen(buf) - 1] = '\0'; // eat the newline fgets() stores
      //printf("%i\n",atoi(buf));
      sum += atoi(&buf);
      size++;
   }
   printf("The number of data in the file is %i, total sum is %i \n",size,sum);
   fclose(fp);

   clock_t end = clock();
   double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
   printf("---------------------------\nExecution Time of the sum-serial.c ---> %f\n---------------------------\n",time_spent);
   return 0;
}