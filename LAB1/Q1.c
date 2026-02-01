#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

int main()
{
    int n = 1 << 16;
    double a = 2.0;

    double *x = (double *)malloc(n * sizeof(double));
    double *y = (double *)malloc(n * sizeof(double));

    for (int i = 0; i < n; i++)
    {
        x[i] = 1.0;
        y[i] = 0.5;
    }

    double start = omp_get_wtime();

    #pragma omp parallel for
    for (int i = 0; i < n; i++)
    {
        x[i] = a * x[i] + y[i];
    }

    double end = omp_get_wtime();

    printf("execution time : %f seconds\n", end - start);

    free(x);
    free(y);

    return 0;
}
