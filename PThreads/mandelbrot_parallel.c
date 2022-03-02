//Austin Brothers
//arb204

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <sys/unistd.h>
#include <pthread.h>

#define XMAX 2.0
#define XMIN -2.0
#define YMAX 2.0
#define YMIN -2.0
#define N 10000             //number of divisions for the grid
#define ITER 50                //number of iterations for each point

#define TEST_RESULTS

#define 	NUM_THREADS 8


int pixels[N][N];

struct timeval startTime;
struct timeval finishTime;

//The printing is only for fun :)
void printMandelBrot();

void verifyMatrixSum();

double timeIntervalLength;

//Initialize the Mutex, Barrier and totalsum (of all pixels) as global variables
pthread_mutex_t mutex;
pthread_barrier_t barrier;
double totalsum = 0;

//Function that each thread runs, implements way to get Mandelbrot_set
void *Mandelbrot(void *threadid) {

    //Gets the threads id and stores it as a long in tid
    long tid;
    tid = (long)threadid;

    //initialize variables for for-loops and mandelbrot while loop
    int i, j, count;

    //Initialize mandelbrot variables
    double x, y;            //(x,y) point on the complex plane
    double x0, y0, tempx;
    double dx, dy;
    int first_row;
    int last_row;

    //increments in the real and imaginary directions
    dx = (XMAX - XMIN) / N;
    dy = (YMAX - YMIN) / N;

    //Declares variables and breaks up the data each thread will work on.
    int localn = N/NUM_THREADS;

    
    if(N % NUM_THREADS > 0) {
        localn += 1;
    }
    

    first_row = tid * localn;
    last_row = (tid + 1) * localn;

    /* Performs mandelbrot operations (Threads do not share elements in these for-loops).
    Threads share the pixels matrix, but no two threads have access to same indices. */
    for(i = first_row + 1; i <= last_row && i <= N; i++) {
        for(j = 1; j <= N; j++) {
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
            pixels[i - 1][j - 1] = count;
        }
    }

    //Sets up variables that will be used to get the average value of the pixels
    double sum = 0.0;
    double avg;

    //Adds all of this threads allocated pixels together
    for (i = first_row; i < last_row && i < N; i++) {
        for (j = 0; j < N; j++) {
            sum += (double) pixels[i][j];
        }
    }
    
    /* Once I have a summation of the current threads pixels I need to get a total sum for all the pixels.
    Since the totalsum is a shared variable that each thread will update I have to lock it in a mutex to avoid race conditions. */
    pthread_mutex_lock(&mutex); //Lock the mutex
    totalsum += sum;    //Update the global sum
    pthread_mutex_unlock(&mutex);   //Unlock the mutex

    /* Now I need to calculate the average value of the pixels, to do this each thread must add their sum to the total sum.
    I put a put a barrier here to keep all threads from executing util the totalsum is updated by the last thread, then I get the avg. */
    pthread_barrier_wait(&barrier);
    avg = totalsum/(N*N);

    //Divide each pixel allocated to the current thread by the average.
    for (i = first_row; i < last_row && i < N; i++) {
        for (j = 0; j < N; j++) {
            pixels[i][j]=pixels[i][j]/avg;
        }
    }

}

int main() {

    //Thread info initialization
    pthread_t threads[NUM_THREADS];
    long t;
    void *status;

    //Get the start time
    gettimeofday(&startTime, NULL); /* START TIME */

    //Initialize the mutex
    pthread_mutex_init(&mutex, NULL);

    //Initialize the barrier
    pthread_barrier_init(&barrier, NULL, NUM_THREADS);

    //Create the threads and set them to execute the function Mandelbrot
    for(t = 0; t < NUM_THREADS; t++) {
        pthread_create(&threads[t], NULL, Mandelbrot, (void *)t);
    }

    //Join the threads
    for(t = 0; t < NUM_THREADS; t++) {
        pthread_join(threads[t], &status);
    }

    //free the mutex
    pthread_mutex_destroy(&mutex);

    //free the barrier
    pthread_barrier_destroy(&barrier);

    //Get the end time
    gettimeofday(&finishTime, NULL);  /* END TIME */

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
            totalSum += (double) pixels[i][j];
        }
    }

    printf("\nTotal Sum = %g\n", totalSum);
}

void printMandelBrot() {
    int i, j;
    for (i = 0; i < N; i++) {
        for (j = 0; j < N; j++) {
            printf("%d ", pixels[i][j]);
        }
        printf("\n");
    }
}
