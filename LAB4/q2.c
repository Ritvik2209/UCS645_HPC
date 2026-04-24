#include<stdio.h>
#include<stdlib.h>
#include<mpi.h>

int main(int argc, char** argv){
    //initialize mpi
    MPI_Init(&argc,&argv);
    int rank,size;
    MPI_Comm_rank(MPI_COMM_WORLD,&rank);
    MPI_Comm_size(MPI_COMM_WORLD,&size);

    const int N = 100;
    
    //ensure equal division
    if(N%size!=0){
        if(rank==0){
            printf("Error: N (%d) must be divisible by size (%d)\n",N,size);
        }
        MPI_Abort(MPI_COMM_WORLD,1);
    }
    int chunk_size = N/size;

    int *data = NULL;
    int *local_data = (int*) malloc(chunk_size*sizeof(int));

    if(rank==0){
        data = (int*) malloc(N*sizeof(int));
        for(int i=0;i<N;i++) data[i] = i+1;
    }
    MPI_Barrier(MPI_COMM_WORLD);
    double start_time = MPI_Wtime();

    //distribute
    MPI_Scatter(
        data, chunk_size, MPI_INT,
        local_data, chunk_size, MPI_INT,
        0, MPI_COMM_WORLD
    );

    int local_sum = 0;
    for(int i=0;i<chunk_size;i++){
        local_sum += local_data[i];
    }

    int global_sum=0;

    MPI_Reduce(
        &local_sum,&global_sum,
        1,MPI_INT,MPI_SUM, 
        0, MPI_COMM_WORLD
    );
    MPI_Barrier(MPI_COMM_WORLD);
    double end_time = MPI_Wtime();

    if(rank==0){
        printf("Global Sum = %d, expected=5050\n", global_sum);
        printf("Execution Time: %f\n",(end_time-start_time));
    }
    if(rank==0) free(data);
    free(local_data);

    MPI_Finalize();
    return 0;

}