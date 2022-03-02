module matrix_mult_par;

import std.algorithm;
import std.parallelism;
import std.range;
import std.stdio;
import core.time;

immutable int N = 1500;
immutable int NUM_THREADS = 8;

__gshared int [N][N] matrixA;
__gshared int [N][N] matrixB;
__gshared int [N][N] matrixC;

__gshared int [N][N] tempMatrix;
__gshared int [N][N] outputMatrix;

void main() {
	int i, j;
    for(i=0;i<N;i++)
    {
        for(j=0;j<N;j++)
        {
            matrixA[i][j] = (i + j)/128;
            matrixB[i][j] = (j + j)/128;
            matrixC[i][j] = (i + j)/128;
            tempMatrix[i][j] = 0;
            outputMatrix[i][j]= 0;
        }
    }

	MonoTime before = MonoTime.currTime;
    TaskPool taskpool = new TaskPool(NUM_THREADS);

	foreach (i; taskpool.parallel(N.iota)) {
		foreach (j; N.iota) {
			foreach (k; N.iota) {
				tempMatrix[i][j] += matrixA[i][k] * matrixB[k][j];
			}
		}
		foreach(j; N.iota) {
			foreach (k; N.iota) {
				outputMatrix[i][j] += tempMatrix[i][k] * matrixC[k][j];
			}
		}
	}

    taskpool.finish();
	MonoTime after = MonoTime.currTime;
	auto elapsed = after - before;
	writeln("\nInterval length: ", elapsed.total!"msecs", " msecs");

	verifyMatrixSum;
}

void verifyMatrixSum() {
    int i, j;

    double totalSum;
    totalSum=0;
    for(i=0;i<N;i++){
        for(j=0;j<N;j++)
        {
            totalSum+=cast(double)outputMatrix[i][j];
        }
    }
    printf("\nTotal Sum = %g\n",totalSum);
}