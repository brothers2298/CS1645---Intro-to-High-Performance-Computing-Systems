//Austin Brothers
//arb204

#include <stdio.h>
#include <math.h>
#include <time.h>
#include <sys/time.h>
#include <pthread.h>

#define		NSTEPS	100000000
#define		P_START	0
#define		P_END	5

#define 	NUM_THREADS 8



struct timeval startTime;
struct timeval finishTime;
double timeIntervalLength;

double polynomial(double x);

double area = 0.0;
pthread_mutex_t mutex;

void *Area(void *threadid) {

    long tid;
    tid = (long)threadid;

    int i;
    double step_size;
    double p_current = P_START;
    double f_result_low, f_result_high;
    double local_area;

    // Calculating intermediary step sizes
    step_size = (double)(P_END - P_START) / NSTEPS;

    //Initial step position
    p_current = P_START;

    int localn = (NSTEPS / NUM_THREADS);

    if(NSTEPS % NUM_THREADS > 0) {
        localn += 1;
    }

    int first_row = tid * localn;
    int last_row = (tid + 1) * localn - 1;

    for(i = first_row; i <= last_row && i < NSTEPS; i++) {
        p_current = i * step_size;

        f_result_low = polynomial(p_current);
        f_result_high = polynomial(p_current + step_size);

        local_area += (f_result_low + f_result_high) * step_size / 2;
    }

    pthread_mutex_lock(&mutex);
    area += local_area;
    pthread_mutex_unlock(&mutex);
}

int main() {
    /*
    int i;
    double step_size;
    double area;
    double p_current = P_START;
    double f_result_low, f_result_high;
    */

    pthread_t threads[NUM_THREADS];
    long t;
    void* status;

    /*
    // Calculating intermediary step sizes
    step_size = (double)(P_END - P_START) / NSTEPS;

    //Initial step position
    p_current = P_START;
    area=0.0;
    */

    //Get the start time
    gettimeofday(&startTime, NULL);

    pthread_mutex_init(&mutex, NULL);

    for(t = 0; t < NUM_THREADS; t++) {
        pthread_create(&threads[t], NULL, Area, (void *)t);
    }
    /*
    for(i = 0; i < NSTEPS; i++)
    {
        p_current = i * step_size;

        f_result_low = polynomial(p_current);
        f_result_high = polynomial(p_current + step_size);

        area += (f_result_low + f_result_high) * step_size / 2;
    }
    */

    for(t = 0; t < NUM_THREADS; t++) {
        pthread_join(threads[t], &status);
    }

    pthread_mutex_destroy(&mutex);

    //Get the end time
    gettimeofday(&finishTime, NULL);  /* after time */

    //Calculate the interval length
    timeIntervalLength = (double)(finishTime.tv_sec-startTime.tv_sec) * 1000000
                         + (double)(finishTime.tv_usec-startTime.tv_usec);
    timeIntervalLength=timeIntervalLength/1000;

    //Print the interval length
    printf("Interval length: %g msec.\n", timeIntervalLength);

    printf("Result: %f \n",area);

    return 0;
}

// Calculates x->y values of a fixed polynomial
// Currently is https://www.desmos.com/calculator/swxvru1xxn
double polynomial(double x){

    // x^2
    double numerator = pow(x, 2);
    //(-4x^3+2x^4)
    double temp_poly = -4 * pow(x, 3) + 2 * pow(x, 4);

    //(-4x^3+2x^4)^2
    double temp_poly_2 = pow(temp_poly, 4);

    // x^3 + 2x^2 * (-4x^3+2x^4)^2
    double temp_poly_3 = pow(x, 3) + 2 * pow(x, 2) * temp_poly_2;

    // root square of (x^3 + 2x^2 * (-4x^3+2x^4)^2)
    double denominator = sqrt(temp_poly_3);

    double y = 0;
    if (denominator != 0)
        y = numerator / denominator;

    return y;
}