#include<stdio.h>
#include<stdlib.h>
#include<mpi.h>

#define N 500000000

int main(int argc, char** argv){
    MPI_Init(&argc,&argv);
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD,&rank);
    MPI_Comm_size(MPI_COMM_WORLD,&size);

    double x = 0.0; //multiplier
    if(rank==0) x = 2.0;

    //broadcast multiplier
    MPI_Bcast(&x,1,MPI_DOUBLE,0, MPI_COMM_WORLD);
   
    if(N%size!=0 && rank==0) printf("%d is not perfectly divisible by %d Processes",N,size);

    long chunk = N/size;
    double local_sum =0.0;
    //sync
    MPI_Barrier(MPI_COMM_WORLD);
    double start = MPI_Wtime();

    for(long i=0;i<chunk;i++){
        double A = 1.0;
        double B = 2.0*x;
        local_sum += A*B;
    }
    double global_sum =0.0;
    //reduce
    MPI_Reduce(&local_sum, &global_sum, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

    double end = MPI_Wtime();
    
    double expected = (double)N*4.0;
    if(rank==0){
        printf("Dot Product Result: %0.2f (Expected: %0.2f)\n",global_sum,expected);
        printf("MPI time: %.6f sec\n",(end-start));
    }
    MPI_Finalize();
    return 0;
}