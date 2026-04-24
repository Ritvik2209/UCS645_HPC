#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

#define N 10000000

int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    double *A = (double *)malloc(N * sizeof(double));

    // init array
    if (rank == 0)
    {
        for (int i = 0; i < N; i++)
        {
            A[i] = (double)i;
        }
    }

    // sync
    MPI_Barrier(MPI_COMM_WORLD);
    double start = MPI_Wtime();

    if (rank == 0)
    {
        for (int i = 1; i < size; i++)
        {
            MPI_Send(A, N, MPI_DOUBLE, i, 0, MPI_COMM_WORLD);
        }
    }
    else
    {
        MPI_Recv(A,N,MPI_DOUBLE,0,0,MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }

    double end = MPI_Wtime();

    if (rank == 0)
    {
        printf("Manual Bcast MPI time: %f sec\n", (end - start));
    }

    MPI_Barrier(MPI_COMM_WORLD);
     start = MPI_Wtime();

    MPI_Bcast(A,N,MPI_DOUBLE,0,MPI_COMM_WORLD);

     end = MPI_Wtime();
    if (rank == 0)
    {
        printf("Standard Bcast MPI time: %f sec\n", (end - start));
    }

    free(A);
    MPI_Finalize();
    return 0;
}