#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <sys/unistd.h>
#include <mpi.h>
#include <string.h>

#define XMAX 2.0
#define XMIN -2.0
#define YMAX 2.0
#define YMIN -2.0
#define N 30000        //number of divisions for the grid
#define ITER 50                //number of iterations for each point

#define TEST_RESULTS

#define 	NUM_THREADS 8


int pixels[N * N];

struct timeval startTime;
struct timeval finishTime;

//The printing is only for fun :)
void printMandelBrot();

void verifyMatrixSum();

double timeIntervalLength;

int main(int argc, char* argv[]) {
    int i, j, count;

    double x, y;            //(x,y) point on the complex plane
    double x0, y0, tempx;
    double dx, dy;

    //increments in the real and imaginary directions
    dx = (XMAX - XMIN) / N;
    dy = (YMAX - YMIN) / N;

    //Get the start time
    gettimeofday(&startTime, NULL); /* START TIME */

    int rank, size;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int localn[size];
    int chunksize[size];
    int disp[size];
    int last_row[size];

    for(i = 0; i < size; i++) {
        if(i < N % size) {
            localn[i] = (N / size) + 1;
            chunksize[i] = localn[i] * N;
        } else {
            localn[i] = N / size;
            chunksize[i] = localn[i] * N;
        }
        if(i == 0) {
            disp[0] = 0;
        } else {
            disp[i] = disp[i - 1] + chunksize[i - 1];
        }
        last_row[i] = (disp[i] / N) + localn[i];
    }

    int *pixel_arr = malloc(sizeof(int) * N * N);
    int *pixel_out = malloc(sizeof(int) * N * N);

    MPI_Bcast(&chunksize, size, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&disp, size, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&last_row, size, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Scatterv(&pixels, chunksize, disp, MPI_INT, pixel_arr, chunksize[rank], MPI_INT, 0, MPI_COMM_WORLD);
    
    //calculations for mandelbrot
    for (i = disp[rank] / N; i <= last_row[rank]; i++) {
        for (j = 1; j <= N; j++) {
            // c_real
            x0 = XMAX -
                 (dx * (i));  //scaled i coordinate of pixel (scaled to lie in the Mandelbrot X scale (-2.00, 2.00))
            // c_imaginary
            y0 = YMAX -
                 (dy * (j));  //scaled j coordinate of pixel (scaled to lie in the Mandelbrot X scale (-2.00, 2.00))

            // z_real
            x = 0;

            // z_imaginary
            y = 0;
            count = 0;

            while ((x * x + y * y < 4) && (count < ITER)) {
                tempx = (x * x) - (y * y) + x0;
                y = (2 * x * y) + y0;
                x = tempx;
                count++;
            }
            pixel_arr[(((i - 1) * N) - disp[rank]) + (j - 1)] = count;
        }
    }

    /*
    for(j=0; j< N * N; j++)
    {
        printf("Rank: %d: %d\t", rank, pixel_arr[j]);
    }
    printf("\n\n");
    */

    // Normalize the result based on the avg value
    double partialSum = 0.0;
    double totalSum = 0.0;
    double avg;
    for (i = disp[rank] / N; i < last_row[rank]; i++) {
        for (j = 0; j < N; j++) {
            partialSum += (double) pixel_arr[(i * N) - disp[rank] + j];
        }
    }
    
    MPI_Reduce(&partialSum, &totalSum, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

    if(rank == 0){
        avg = totalSum/(N*N);
    }

    MPI_Bcast(&avg, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    
    for (i = disp[rank] / N; i < last_row[rank]; i++) {
        for (j = 0; j < N; j++) {
            pixel_arr[(i * N) - disp[rank] + j] = pixel_arr[(i * N) - disp[rank] + j] / avg;
        }
    }

    MPI_Gatherv(pixel_arr, chunksize[rank], MPI_INT, pixel_out, chunksize, disp, MPI_INT, 0, MPI_COMM_WORLD);

    if(rank == 0) {

        //Get the end time
        gettimeofday(&finishTime, NULL);  /* END TIME */

        for(i = 0; i < N * N; i++) {
            pixels[i] = pixel_out[i];
        }

        //Calculate the interval length
        timeIntervalLength = (double) (finishTime.tv_sec - startTime.tv_sec) * 1000000
                            + (double) (finishTime.tv_usec - startTime.tv_usec);
        timeIntervalLength = timeIntervalLength / 1000;
        //Print the interval length
        printf("Interval length: %g msec.\n", timeIntervalLength);
        
        #ifdef TEST_RESULTS
            verifyMatrixSum(pixels);
        //    printMandelBrot(pixels);
        #endif

    }

    MPI_Finalize();

    return 0;
}

// Helper function to verify if the sum from parallel and serial versions match
void verifyMatrixSum() {
    int i, j;

    double totalSum;
    totalSum = 0;
    //
    for (i = 0; i < N; i++) {
        for (j = 0; j < N; j++) {
            totalSum += (double) pixels[(i * N) + j];
        }
    }

    printf("\nTotal Sum = %g\n", totalSum);
}

void printMandelBrot() {
    int i, j;
    for (i = 0; i < N; i++) {
        for (j = 0; j < N; j++) {
            printf("%d ", pixels[(i * N) + j]);
        }
        printf("\n");
    }
}
