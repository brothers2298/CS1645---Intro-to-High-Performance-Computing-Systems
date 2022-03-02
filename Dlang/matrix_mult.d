module matrix_mult;

import std.stdio;
import core.time;

immutable int N = 1024;

int [N][N] matrixA;
int [N][N] matrixB;
int [N][N] matrixC;

int [N][N] tempMatrix;
int [N][N] outputMatrix;

void main()
{
    int i,j,k;
    for(i=0;i<N;i++)
    {
        for(j=0;j<N;j++)
        {
            matrixA[i][j]= (i + j)/128;
            matrixB[i][j]= (j + j)/128;
            matrixC[i][j]= (i + j)/128;
            tempMatrix[i][j] = 0;
            outputMatrix[i][j]= 0;
        }
    }

    MonoTime before = MonoTime.currTime;

    for(i=0;i<N;i++){
        for(j=0; j<N; j++){
            for(k=0;k<N;k++){
                tempMatrix[i][j] += matrixA[i][k] * matrixB[k][j];
            }
        }

        for(j=0; j<N; j++) {
            for (k = 0; k < N; k++) {
                outputMatrix[i][j] += tempMatrix[i][k] * matrixC[k][j];
            }
        }
    }
    MonoTime after = MonoTime.currTime;
    auto timeElapsed = after - before;
    writeln("\nInterval length: ", timeElapsed.total!"msecs", " msecs");

    verifyMatrixSum();
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