#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>


int main(int argc, char **argv){
    // initialize mpi
    MPI_Init(&argc, &argv);
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    const int N = 8;
    int A[8] = {1,2,3,4,5,6,7,8};
    int B[8] = {8,7,6,5,4,3,2,1};

       
    //ensure equal division
    if(N%size!=0){
        if(rank==0){
            printf("Error: N (%d) must be divisible by size (%d)\n",N,size);
        }
        MPI_Abort(MPI_COMM_WORLD,1);
    }
    int chunk_size = N/size;


    int* localA = (int*) malloc(chunk_size * sizeof(int));
    int* localB = (int*) malloc(chunk_size * sizeof(int));

    MPI_Barrier(MPI_COMM_WORLD);
    double start_time = MPI_Wtime();

    //distribute
    MPI_Scatter(
        A, chunk_size, MPI_INT,
        localA, chunk_size, MPI_INT,
        0, MPI_COMM_WORLD
    );
    MPI_Scatter(
        B, chunk_size, MPI_INT,
        localB, chunk_size, MPI_INT,
        0, MPI_COMM_WORLD
    );

    int local_dot=0;
    for(int i=0;i<chunk_size;i++){
        local_dot += localA[i]*localB[i];
    }

    int global_dot=0;

    MPI_Reduce(
        &local_dot,&global_dot,
        1,MPI_INT,MPI_SUM, 
        0, MPI_COMM_WORLD
    );
    MPI_Barrier(MPI_COMM_WORLD);
    double end_time = MPI_Wtime();

    if(rank==0){
        printf("Dot Product = %d, expected=120\n", global_dot);
        printf("Execution Time: %f\n",(end_time-start_time));
    }
    free(localA);
    free(localB);

    MPI_Finalize();
    return 0;

}