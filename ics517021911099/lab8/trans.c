/*
    Student Name:       Zhao Xuyang
    Student ID:               517021911099
*/


/*
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 */
#include <stdio.h>
#include "cachelab.h"

int is_transpose(int M, int N, int A[N][M], int B[M][N]);

/*
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded.
 *     s = 5, E = 1, b = 5;
 */

char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{
        int i,j,ii,jj;
        //   int bsize = 8;    // cannot use more than 8 variables
        if(M == 32)  // first case,  32x32
        {
            int index = -1, diagVal; // diagnose assignment wil incur conflicts
            for(ii = 0 ; ii < M; ii += 8)
            {
                for(jj = 0 ; jj < N; jj += 8)
                {
                    for(i = ii;  i < ii + 8; i ++)
                    {
                        for(j = jj ; j < jj + 8; j ++)
                        {
                            if(i == j)
                            {
                                index = i;
                                diagVal = A[i][j];
                            }
                            else
                                B[j][i] = A [i][j];
                        }
                        if(index != -1)
                        {
                            B[index][index] = diagVal;
                            index = -1;
                        }
                    }
                }
            }
        }
        if(M == 64)
        {
            int tmp1,tmp2,tmp3,tmp4;
            //int tmp5,tmp6,tmp7,tmp8;
            for(ii = 0 ; ii < M; ii += 8)
            {
                for(jj = 0 ; jj < N; jj += 8)// every block iterate
                {
                        for(i = 0;  i <  4; i ++)   // load  row 0~3
                        {
                            tmp1 = -1;
                            for(j = 0 ; j <  4; j ++)
                            {
                                if(jj +j == ii +i)
                                {
                                    tmp1= j + jj;
                                    tmp2= A[ii+i][jj+j];
                                    tmp3 = A[ii+i][jj+j + 4];
                                    //B[jj + j][ii + i] =  A[ii+i][jj+j];
                                    //B[jj +j][ii + i + 4] = A[ii+i][jj+j + 4];
                                }
                                else
                                {
                                    B[jj + j][ii + i] = A[ii+i][jj+j];
                                    B[jj +j][ii + i + 4] = A[ii+i][jj+j + 4];
                                }

                                //B[jj + j][ii + i] = A[ii+i][jj+j];
                                //B[jj +j][ii + i + 4] = A[ii+i][jj+j + 4];
                            }
                            if(tmp1 != -1)
                            {
                                B[tmp1][tmp1] = tmp2;
                                B[tmp1][tmp1+4] = tmp3;
                                tmp1= -1;
                            }
                        }
                        for(j = 0 ; j < 4; j ++) // transfer back-diag
                        {
                            tmp1 =B[jj+j][ii+4];
                            tmp2 =B[jj+j][ii+5];
                            tmp3 =B[jj+j][ii+6];
                            tmp4 =B[jj+j][ii+7];
                            for(i = 4; i < 8; i ++)
                            {
                                B[jj+j][ii+i] = A[ii+i][jj+j];
                            }
                            B[jj+j+4][ii] = tmp1;
                            B[jj+j+4][ii+1] = tmp2;
                            B[jj+j+4][ii+2] = tmp3;
                            B[jj+j+4][ii+3] = tmp4;
                        }
                        for(i = 4; i < 8; i ++)
                            for(j =4; j < 8; j ++)
                            {
                                B[jj + j][ii + i] = A[ii+ i][jj + j];
                            }
                    }
                }
          }
         if(M == 61)     // M=61, N=67   in this case, there's no need to worry about conflict eviction.
         {
            for(ii = 0 ; ii < N; ii += 16)
            {
                for(jj = 0 ; jj < M; jj += 16)
                {
                    for(i = ii;  i < ii + 16; i ++)
                    {
                        for(j = jj ; j < jj + 16; j ++)
                        {
                            if(i < N && j < M)
                                B[j][i] = A[i][j];
                        }
                    }
                }
            }
         }

}

/*
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started.
 */

/*
 * trans - A simple baseline transpose function, not optimized for the cache.
 */
char trans_desc[] = "Simple row-wise scan transpose";
void trans(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, tmp;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }

}

/*
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
void registerFunctions()
{
    /* Register your solution function */
    registerTransFunction(transpose_submit, transpose_submit_desc);

    /* Register any additional transpose functions */
    registerTransFunction(trans, trans_desc);

}

/*
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; ++j) {
            if (A[i][j] != B[j][i]) {
                return 0;
            }
        }
    }
    return 1;
}

