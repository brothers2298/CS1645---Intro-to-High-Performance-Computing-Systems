//Austin Brothers
//arb204

#include <pthread.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>

#define		NROW	256
#define		NCOL	NROW

#define 	NUM_THREADS 4

#define TEST_RESULTS


//Matrix A
int matrixA  [NROW][NCOL];
//Matrix B
int matrixB  [NROW][NCOL];
//Matrix C
int matrixC [NROW][NCOL];

//Temp array
int tempMatrix [NROW][NCOL];

//Output Array C
int outputMatrix [NROW][NCOL];

struct timeval startTime;
struct timeval finishTime;
double timeIntervalLength;

void verifyMatrixSum();

//New method to get sum of temporary matrix (which mults 2 matrices)
void verifyMatrixSumTemp();

void *MatrixMult(void *threadid) {

    //Get and store thread id as a long
    long tid;
    tid = (long)threadid;

    //Allocate which elements of a matrix, the threads will perform functions on
    int i, j, k;
    int localn;
    int first_row, last_row;
    
    localn = (NROW / NUM_THREADS);

    if(NROW % NUM_THREADS > 0) {
        localn += 1;
    }

    first_row = tid * localn;
    last_row = (tid + 1) * localn - 1;

    //printf("%d\n", last_row - first_row);

    
    //printf("localn: %d\t first_row: %d\t last_row: %d\n", localn, first_row, last_row);

    //Each thread is broken up by 128 of rows in MatrixA/tempMatrix and columns in MatrixB/MatrixC
    for(i = first_row; i <= last_row && i < NROW; i++) {
        //printf("%d\n", tid);
        //Multiply the first two matrices together into tempMatrix
        for(j = 0; j < NCOL ; j++) {
            for(k = 0; k < NROW; k++) {
                tempMatrix[i][j] += matrixA[i][k] * matrixB[k][j];
            }
        }

        //Multiply the tempMatrix with the last matrix
        for(j = 0; j < NCOL; j++) {
            for (k = 0; k < NROW; k++) { 
                outputMatrix[i][j] += tempMatrix[i][k] * matrixC[k][j];
            }
        }
    }
    for(i=0; i<NROW; i++)
    {
        for(j = 0; j<NCOL; j++){
            printf("Rank: %d: %d\t", tid, outputMatrix[i][j]);
        }
    }
    printf("\n\n");

}


int main(int argc, char* argv[])
{
    int i,j,k;
    // Matrix initialization. Just filling with arbitrary numbers.
    for(i=0;i<NROW;i++)
    {
        for(j=0;j<NCOL;j++)
        {
            matrixA[i][j]= (i + j);//128;
            matrixB[i][j]= (j + j);//128;
            matrixC[i][j]= (i + j);//128;
            tempMatrix[i][j] = 0;
            outputMatrix[i][j]= 0;
        }
    }

    //Initialize thread information
    pthread_t threads[NUM_THREADS];
    long t;
    void *status;

    //Get the start time
    gettimeofday(&startTime, NULL); /* START TIME */

    //Create each thread to perform the MatrixMult function
    for(t = 0; t < NUM_THREADS; t++) {
        //printf("Creating thead %ld\n", t);
        pthread_create(&threads[t], NULL, MatrixMult, (void *)t);
        
    }

    //Join the threads
    for(t = 0; t < NUM_THREADS; t++) {
    pthread_join(threads[t], &status);
    }

    //Get the end time
    gettimeofday(&finishTime, NULL);  /* END TIME */

    //Calculate the interval length
    timeIntervalLength = (double)(finishTime.tv_sec-startTime.tv_sec) * 1000000
                         + (double)(finishTime.tv_usec-startTime.tv_usec);
    timeIntervalLength=timeIntervalLength/1000;

    #ifdef TEST_RESULTS
    //[Verifying the matrix summation]
    //verifyMatrixSumTemp();    DEBUG
    verifyMatrixSum();
    #endif

    //Print the interval length
    printf("Interval length: %g msec.\n", timeIntervalLength);

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
            totalSum+=(double)outputMatrix[i][j];
        }
    }
    printf("\nTotal Sum = %g\n",totalSum);
}

//Helper function to verify if the sum of the TempMatrix from the parallel and serial version match
void verifyMatrixSumTemp() {
    int i, j;

    double totalSum;
    totalSum=0;
    //
    for(i=0;i<NROW;i++){
        for(j=0;j<NCOL;j++)
        {
            totalSum+=(double)tempMatrix[i][j];
        }
    }
    printf("\nTotal Sum = %g\n",totalSum);
}