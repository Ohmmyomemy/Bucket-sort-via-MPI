//BucketSort via MPI
//Thanawath  6388016
//Nopparwut Sirisawat 6388056
//Tawan-ok Ruekveerawattana 6388142

#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"
#include <time.h>
#include <math.h>

//ranging from 0 to 9999
#define totalsize 10000

int cmpfunc (const void * a, const void * b) {
   return ( *(int*)a - *(int*)b );
}

// function swap 
void swap(int *x, int *y)
{
    int temp = *x;
    *x = *y;
    *y = temp;
}

// bubblesort function
void bubbleSort(int arr[], int n)
{
   int i, j;
   for (i = 0; i < n-1; i++)     
       for (j = 0; j < n-i-1; j++)
           if (arr[j] > arr[j+1])
              swap(&arr[j], &arr[j+1]);
}

int main(int argc, char *argv[])
{
    int size;
    int rank;
    int i;
    int arr[totalsize];
    int count;
    int basemin;
    int basemax;
    int mode;

    int totalsize =0;

    int *allcount;
    int *collector;
    int *basebucket;

    double initialtime;
    double endtime;
    
    scanf("Mode Select Size 1:10,000 2:100,000 3:1,000,000 = %d",&mode);
    if(mode ==1){
        totalsize = 10000
    }else if (mode == 2)
    {
       totalsize = 100000
    }else if (mode == 3)
    {
        totalsize = 1000000
    }else{
        exit(0);
    }
    
    
    // Structure to retrieve the information
    MPI_Status status;
    // Initiate an MPI computation and execution environment
    MPI_Init(&argc,&argv);
    // Determines the rank of the calling process in the communicator
    MPI_Comm_rank(MPI_COMM_WORLD,&rank);
    // Determines the size of the group associated with a communicator
    MPI_Comm_size(MPI_COMM_WORLD,&size);

    if (rank == 0){
        //Generate random number
        srand(time(NULL)); // set random seed for random integer
        for (i = 0; i < totalsize; i++){
            arr[i] = rand() % 10000;
        }
	// Show the random integer that didn't sort
        printf("\n\tUnsorted Array: ");
        for(i = 0;i < totalsize; i++){
            printf("%d ",arr[i]);
        }
        printf("\n");
        for (i = 1; i < size; i++){
	    // Performs a blocking send
            MPI_Send(arr,totalsize,MPI_INT,i,1,MPI_COMM_WORLD);
        }
    }
    else {
	// Status of a reception operation, returned by receive operations
        MPI_Recv(arr,totalsize,MPI_INT,0,MPI_ANY_TAG,MPI_COMM_WORLD,&status);
    }
    // Returns an elapsed time on the calling processor
    initialtime = MPI_Wtime();

    count = 0;
    basemin = floor((rank * totalsize)/size);
    basemax = floor(((rank + 1) * totalsize)/size) - 1;

   
    // count the size of each bucket
    for(i=0;i<totalsize;i++){
        if (arr[i]<= basemax && arr[i]>= basemin){
            count++;
        }
    }
    // printf("Rank: %d Counter: %d\n",rank,counter);
    // put value in the bucket
    basebucket = malloc(count * sizeof(int));
    int index = 0;
    for(i=0;i<totalsize;i++){
        if (arr[i]<= basemax && arr[i]>= basemin){
            basebucket[index] = arr[i];    
            index++;
        }
    }
    // sort locally
    qsort(basebucket, index, sizeof(int), cmpfunc);

       if (rank == 0){
        allcount = malloc(size * sizeof(int));
        collector = malloc(size * sizeof(int));
    }
    // Gathers together values from a group of processes
    MPI_Gather(&count, 1, MPI_INT,
               allcount, 1, MPI_INT,
               0, MPI_COMM_WORLD);
    if (rank == 0){
        collector[0] = 0;
        for (i = 0; i < size-1; i++){
            collector[i+1] = collector[i] + allcount[i];
        }
    }
    int sortedArr[totalsize];
    // Gathers into specified locations from all processes in a group
    // we need to use  MPI_Gatherv because the size of return array are not the same
    MPI_Gatherv(basebucket,count,MPI_INT,sortedArr,allcount,collector,MPI_INT,0,MPI_COMM_WORLD);
	
    // Returns an elapsed time on the calling processor
    endtime = MPI_Wtime();

    if(rank == 0){
        printf("\n\tSorted Array: ");
        for(i=0;i<totalsize;i++){
            printf("%d ",sortedArr[i]);
        }
        printf("\nTotalsize : %d",totalsize);
        printf("\nExecution time: %f seconds\n",endtime-initialtime);
    }
    // Terminates MPI execution environment
    MPI_Finalize();
    return 0;
}

