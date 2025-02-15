#include <stdio.h>
#include "mpi.h"
#include <iostream>

int main(int argc, char **argv)
{
    int rank, size;   
    
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    for (int i = 1; i <= 10; ++i) {
       std:: cout << rank+1 << " x " << i << " = " <<(rank+1) * i << std::endl;
    }
    MPI_Finalize();
 
  return 0;
}