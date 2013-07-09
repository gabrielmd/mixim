 #include <stdio.h>
 #include <mpi.h>

 int main(int argc, char *argv[]) 
 {
     int numPartitions, myRank;

     MPI_Init(&argc, &argv);
     MPI_Comm_size(MPI_COMM_WORLD, &numPartitions);
     MPI_Comm_rank(MPI_COMM_WORLD, &myRank);

     printf("Hi! I am process %d out of %d\n", myRank, numPartitions);

     MPI_Finalize();
     return 0;
 }

