#include <stdio.h>
#include "mpi.h"
#include <cstdlib>
#include <iostream>

int main(int argc, char **argv){
    int a_i ,b_i ,c_i;
    int a,b;
    int rank;
    MPI_Status status;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    srand(rank+1);
    a_i = rand()%10;
    b_i = rand()%10;
    std:: cout<< "Process with rank " << rank << " has a_i=" << a_i<< " and b_i="<<b_i<<"\n"; 
    switch (rank) 
    {
    case 0:
        MPI_Send(&a_i,1,MPI_INT,2,0,MPI_COMM_WORLD);
        MPI_Send(&b_i,1,MPI_INT,1,0,MPI_COMM_WORLD); 
        MPI_Recv(&a,1,MPI_INT,1,0,MPI_COMM_WORLD,&status);
        MPI_Recv(&b,1,MPI_INT,2,0,MPI_COMM_WORLD,&status);
        break;
        
    case 1:
        MPI_Recv(&a,1,MPI_INT,3,0,MPI_COMM_WORLD,&status);
        MPI_Recv(&b,1,MPI_INT,0,0,MPI_COMM_WORLD,&status);
        MPI_Send(&a_i,1,MPI_INT,0,0,MPI_COMM_WORLD);
        MPI_Send(&b_i,1,MPI_INT,3,0,MPI_COMM_WORLD);
        break;

    case 2:
        MPI_Recv(&a,1,MPI_INT,0,0,MPI_COMM_WORLD,&status);
        MPI_Recv(&b,1,MPI_INT,3,0,MPI_COMM_WORLD,&status);
        MPI_Send(&a_i,1,MPI_INT,3,0,MPI_COMM_WORLD);
        MPI_Send(&b_i,1,MPI_INT,0,0,MPI_COMM_WORLD);
        break;

    case 3:
        MPI_Send(&a_i,1,MPI_INT,1,0,MPI_COMM_WORLD);
        MPI_Send(&b_i,1,MPI_INT,2,0,MPI_COMM_WORLD);
        MPI_Recv(&a,1,MPI_INT,2,0,MPI_COMM_WORLD,&status);
        MPI_Recv(&b,1,MPI_INT,1,0,MPI_COMM_WORLD,&status);
        break;

    default:
        break;
    }
    c_i=a+b;
    std:: cout<< "Process with rank " << rank << " get a=" << a<< " and b="<<b<<", c="<<c_i<<"\n";

    MPI_Finalize();
}