#include <stdio.h>
#include "mpi.h"
#include <sys/types.h>
#include <unistd.h>

int main(int argc, char **argv)
{  
    MPI_Init(&argc, &argv);
    int rank, size; 
    MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    printf("Size:%i, Process rank:%i, Process id:%i \n", size,rank,getpid());   
    MPI_Finalize();
 
  return 0;
}