#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

#define MAX_VAL 10000
#define STOP_SIGNAL -1

int isPerfect(int n)
{
    if (n < 2)
        return 0;
    int sum = 1;
    for (int i = 2; i * i <= n; i++)
    {
        if (n % i == 0)
        {
            sum += i;
            if (i != n / i) // avoid double count for square numbers
                sum += n / i;
        }
    }
    return sum == n;
}

int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (size < 2)
    {
        if (rank == 0)
            printf("Requires at least 2 processes\n");
        MPI_Finalize();
        return 0;
    }

    if (rank == 0)
    {
        int next_num = 2;
        int next_node = size - 1;

        while (next_node > 0)
        {
            int request;
            MPI_Status status;
            MPI_Recv(&request, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);

            if (next_num <= MAX_VAL)
            {
                // send next_num to node that just finished
                MPI_Send(&next_num, 1, MPI_INT, status.MPI_SOURCE, 0, MPI_COMM_WORLD);
                next_num++;
            }
            else
            {
                int stop = STOP_SIGNAL;
                // no more work, stop
                MPI_Send(&stop, 1, MPI_INT, status.MPI_SOURCE, 0, MPI_COMM_WORLD);
                next_node--;
            }
        }
    }
    else
    {
        int num_to_check = 0; // initial signal
        while (1)
        {
            // send prev result(or intial signal) and get work
            MPI_Send(&num_to_check, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
            // recv new num to check
            MPI_Recv(&num_to_check, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            if (num_to_check == STOP_SIGNAL)
                break;
            if (isPerfect(num_to_check))
            {
                printf("Process:%d found perfect no.%d\n", rank, num_to_check);
            }
        }
    }
    MPI_Finalize();
    return 0;
}