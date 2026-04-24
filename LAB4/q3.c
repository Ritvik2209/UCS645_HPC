#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

typedef struct{
    int value;
    int rank;
} DataPair;

int main(int argc, char **argv){
    // initialize mpi
    MPI_Init(&argc, &argv);
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    const int count = 10;
    int local_values[count];

    srand(1337 + rank);

    for (int i = 0; i < count; i++)
    {
        local_values[i] = rand() % 1000; // b/w (0-999)
    }

    int local_min = local_values[0];
    int local_max = local_values[0];

    for (int i = 1; i < count; i++)
    {
        if (local_values[i] < local_min)
            local_min = local_values[i];
        if (local_values[i] > local_max)
            local_max = local_values[i];
    }

    DataPair in_max = {local_max, rank};
    DataPair in_min = {local_min, rank};
    DataPair out_max, out_min;

    MPI_Barrier(MPI_COMM_WORLD);
    double start = MPI_Wtime();

    MPI_Reduce(&in_max, &out_max, 1, MPI_2INT, MPI_MAXLOC, 0, MPI_COMM_WORLD);
    MPI_Reduce(&in_min, &out_min, 1, MPI_2INT, MPI_MINLOC, 0, MPI_COMM_WORLD);

    MPI_Barrier(MPI_COMM_WORLD);
    double end = MPI_Wtime();

    if (rank == 0)
    {
        printf("Global Max = %d, from process %d\n", out_max.value, out_max.rank);
        printf("Global Min = %d, from process %d\n", out_min.value, out_min.rank);
        printf("Execution Time: %f\n", (end - start));
    }

    MPI_Finalize();
    return 0;
}