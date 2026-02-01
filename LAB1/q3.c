#include <stdio.h>
#include <stdlib.h>
#include <omp.h>


int main()
{
    long N = 1000000;
    double step = 1.0 / (double)N;
    double x, pi, sum = 0.0;

    double s_time, e_time;
    omp_set_num_threads(8);
    printf("----------------------------------------------\n");
    printf("Num of steps: %ld \n", N);
    printf("Threads: %d \n", omp_get_max_threads());
    s_time = omp_get_wtime();
    int i = 0;
#pragma omp parallel for private(x) reduction(+ : sum)
    for ( i = 0; i < N; i++)
    {
        x = (i + 0.5) * step;
        sum += 4.0 / (1.0 + x * x);
    }
    pi = step*sum;
    
    e_time = omp_get_wtime();
    printf("Execution time: %f\n", e_time - s_time);
    printf("pi = %f\n", pi);
    printf("----------------------------------------------\n");
    return 0;
}