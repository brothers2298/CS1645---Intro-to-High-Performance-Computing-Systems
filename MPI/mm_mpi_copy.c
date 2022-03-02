#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <mpi.h>
#include <stdlib.h>

#define		NROW	100
#define		NCOL	NROW

#define 	NUM_THREADS 8

#define TEST_RESULTS


//Matrix A
int matrixA  [NROW * NCOL];
//Matrix B
int matrixB  [NROW * NCOL];
//Matrix C
int matrixC [NROW * NCOL];

//Temp array
int tempMatrix [NROW * NCOL];

//Output Array C
int outputMatrix [NROW * NCOL];

struct timeval startTime;
struct timeval finishTime;
double timeIntervalLength;

void verifyMatrixSum();

int main(int argc, char* argv[])
{
    int i, j, k;
    // Matrix initialization. Just filling with arbitrary numbers.
    for(i = 0; i < NROW; i++)
    {
        for(j = 0; j < NCOL; j++)
        {
            matrixA[(i * NCOL) + j] = (i + j)/128;
            matrixB[(i * NCOL) + j] = (j + j)/128;
            matrixC[(i * NCOL) + j] = (i + j)/128;
            tempMatrix[(i * NCOL) + j] = 0;
            outputMatrix[(i * NCOL) + j] = 0;
        }
    }

    //Get the start time
    gettimeofday(&startTime, NULL); /* START TIME */

    int rank, size;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

/*
    if(rank == 0)
    {
        for(i=0; i<NROW * NCOL; i++)
        {
            printf("%d\t", matrixA[i]);
        }
        printf("\n\n");
    }
    */
    //int localn;
    //int first_row, last_row, chunks;
    
    
    int localn[size];
    int disp[size];
    int chunksize[size];
    int last_row[size];
    

    for(i = 0; i < size; i++) 
    {
        if(i < NROW % size){
            localn[i] = (NROW / size) + 1;
            chunksize[i] = (localn[i]) * NCOL;
        } else {
            localn[i] = (NROW / size);
            chunksize[i] = (localn[i]) * NCOL;
        }
        if(i == 0) {
            disp[0] = 0;
        } else {
            disp[i] = disp[i - 1] + chunksize[i - 1];
        }
        last_row[i] = disp[i] / NCOL + localn[i]; 
    }
    
    

    //first_row = rank * localn;
    //chunks = (rank + 1) * localn;
    //last_row = chunks - 1;
    //int chunksize = (chunks - first_row) * NCOL;
    //printf("chunksize = %d\npseudochunksize = %d\n", chunksize, pseudochunksize);

    int *output = malloc(sizeof(int) * NROW * NCOL);
    int *result = malloc(sizeof(int) * NROW * NCOL);

    

    MPI_Bcast(&matrixA, NROW * NCOL, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&matrixB, NROW * NCOL, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&matrixC, NROW * NCOL, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&tempMatrix, NROW * NCOL, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&disp, size, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&chunksize, size, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&last_row, size, MPI_INT, 0, MPI_COMM_WORLD);
    
    
    //printf("rank: %d, first_row: %d, chunksize %d last_row: %d\n", rank, disp[rank] / NCOL, chunksize[rank] / NCOL, last_row[rank]);


    MPI_Scatterv(&outputMatrix, chunksize, disp, MPI_INT, output, chunksize[rank], MPI_INT, 0, MPI_COMM_WORLD);
    //printf("rank %d here\n", rank);


    /*
    printf("Rank: %d MatrixA:", rank);
    for(i=0; i<NROW * NCOL; i++)
    {
        printf("%d\t", matrixA[i]);
    }
    printf("\n\n");
    */


    //printf("rank: %d, first_row: %d, last_row: %d\n", rank, disp[rank] / NCOL, last_row[rank]);

    //A*B*C
    for(i = disp[rank] / NCOL; i < last_row[rank]; i++){
        for(j = 0; j < NCOL; j++){
            for(k = 0; k < NROW; k++){
                // K iterates rows on matrixA and columns in matrixB
                tempMatrix[(i * NCOL) + j] += matrixA[(i * NCOL) + k] * matrixB[(k * NCOL) + j];
            }
        }

        //printf("here");

        for(j = 0; j < NCOL; j++) {
            for (k = 0; k < NROW; k++) {
                // K iterates rows on matrixA and columns in matrixB
                output[(i * NCOL) - (disp[rank]) + j] += tempMatrix[(i * NCOL) + k] * matrixC[(k * NCOL) + j];
            }
        }
    }

    /*
    printf("Rank: %d output:", rank);
    for(i=0; i<NROW * NCOL; i++)
    {
        printf("%d\t", output[i]);
    }
    printf("\n\n");
    */

    //printf("Proc %d gets here.", rank);
    //printf("does chunksize (%d) = N (%d)", chunksize*size, NROW*NCOL);
    MPI_Gatherv(output, chunksize[rank], MPI_INT, result, chunksize, disp, MPI_INT, 0, MPI_COMM_WORLD);
    /*
    if(rank == 0)
    {
        for(j=0; j<NROW * NCOL; j++)
        {
            printf("Rank: %d: %d\t", rank, result[j]);
        }
        printf("\n\n");
    }
    */

    for(i = 0; i < NROW * NCOL; i++)
    {
        //printf("%d\t", outputMatrix[i]);
        outputMatrix[i] = result[i];
    }


    //Get the end time
    gettimeofday(&finishTime, NULL);  /* END TIME */

    //Calculate the interval length
    timeIntervalLength = (double)(finishTime.tv_sec-startTime.tv_sec) * 1000000
                         + (double)(finishTime.tv_usec-startTime.tv_usec);
    timeIntervalLength=timeIntervalLength/1000;

    if(rank == 0) {
        #ifdef TEST_RESULTS
        //[Verifying the matrix summation]
        verifyMatrixSum();
        #endif

        //Print the interval length
        printf("Interval length: %g msec.\n", timeIntervalLength);
    }

    MPI_Finalize();
    return 0;
}


// Helper function to verify if the sum from parallel and serial versions match
void verifyMatrixSum() {
    int i, j;

    double totalSum;
    totalSum=0;
    //
    for(i=0;i<NROW;i++){
        for(j=0;j<NCOL;j++)
        {
            totalSum+=(double)outputMatrix[(i * NCOL) + j];
            //printf("%d\t", outputMatrix[i * NCOL + j]);
        }
    }
    printf("\nTotal Sum = %g\n",totalSum);
}
