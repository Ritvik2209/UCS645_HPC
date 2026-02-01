#include <stdio.h>
#include <omp.h>

#define n 1000

double a[n][n], b[n][n], c[n][n];

int main()
{
    // Matrix initialization
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
        {
            a[i][j] = 1.0;
            b[i][j] = 1.0;
            c[i][j] = 0.0;
        }
    }

    double start = omp_get_wtime();

    #pragma omp parallel for collapse(2)
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
        {
            for (int k = 0; k < n; k++)
            {
                c[i][j] += a[i][k] * b[k][j];
            }
        }
    }

    double end = omp_get_wtime();

    printf("Execution time: %f seconds\n", end - start);

    return 0;
}
