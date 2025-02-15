#include <cstdlib>
#include <iostream>
#include <ctime>
#include "mpi.h" 

int main(int argc, char *argv[]) {
    int rank, size, ai, contribution, total_sum;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    
    srand(rank * time(NULL));
    ai = (rand() % 10) + 1;
    printf("rank = %d, a%d = %d\n", rank, rank, ai);
    
    contribution = (ai % 3 == 0) ? ai : 0;
    
    MPI_Reduce(&contribution, &total_sum, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
    
    if (rank == 0) {
        printf("sum = %d\n", total_sum);
    }
    
    MPI_Finalize();
    return 0;
}