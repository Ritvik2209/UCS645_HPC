#include <stdio.h>
#include <mpi.h>

int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    int value = 0;
    // only the root starts with the initial value
    if (rank == 0)
        value = 100;

    MPI_Barrier(MPI_COMM_WORLD);
    // starting timing
    double start_time = MPI_Wtime();

    for (int step = 0; step < size - 1; ++step)
    {
        if (rank == step)
        {
            int next = (rank + 1) % size;
            printf("Process %d sending %d to process %d\n", rank, value, next);

            MPI_Send(&value, 1, MPI_INT, next, 0, MPI_COMM_WORLD);
        }
        if (rank == (step + 1) % size)
        {
            int prev = (rank - 1 + size) % size;
            MPI_Recv(&value, 1, MPI_INT, prev, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            value += rank;

            printf("Process %d recieved value: %d\n", rank, value);
        }
        MPI_Barrier(MPI_COMM_WORLD); // ensure each step completes of the ring
    }
    double end_time = MPI_Wtime();
    if (rank == 0)
    {
        printf("Final value: %d\n", value);
        printf("Total execution time: %f sec\n", (end_time - start_time));
    }
    MPI_Finalize();
    return 0;
}