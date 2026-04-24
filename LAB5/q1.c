#include<stdio.h>
#include<stdlib.h>
#include<mpi.h>

#define N 65536

int main(int argc, char** argv){
    MPI_Init(&argc,&argv);
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD,&rank);
    MPI_Comm_size(MPI_COMM_WORLD,&size);

    double a = 2.5;
    if(N%size!=0 && rank==0) printf("%d is not perfectly divisible by %d Processes",N,size);

    int chunk = N/size;
    double *X = (double *)malloc(chunk*sizeof(double));
    double *Y = (double*)malloc(chunk *sizeof(double));

    //init array
    for(int i=0;i<chunk;i++){
        X[i] = 1.0;
        Y[i] = 2.0;
    }

    //sync
    MPI_Barrier(MPI_COMM_WORLD);
    double start = MPI_Wtime();

    // DAXPY Kernel
    for (int i = 0; i < chunk; i++) {
        X[i] = a * X[i] + Y[i];
    }
    double end = MPI_Wtime();
    if(rank==0){
        printf("MPI time: %f sec\n",(end-start));
    }
    free(X);
    free(Y);
    MPI_Finalize();
    return 0;
}