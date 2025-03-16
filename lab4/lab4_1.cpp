#include <cstdlib>
#include <iostream>
#include <ctime>
#include "mpi.h"

int main(int argc, char *argv[]) {
    int rank, size, ai, contribution, total_sum = 0;
    int *array = nullptr;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (rank == 0) {
        array = new int[size];
        srand(time(NULL));
        for (int i = 0; i < size; i++) {
            array[i] = (rand() % 10) + 1;
            printf("a%d = %d\n", i, array[i]);
        }
    }

    // Рассылка элементов из массива array процесса 0 всем процессам
    MPI_Scatter(array, 1, MPI_INT, &ai, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // Вычисление вклада текущего процесса
    contribution = (ai % 3 == 0) ? ai : 0;

    int *contributions = nullptr;
    if (rank == 0) {
        contributions = new int[size];
    }

    // Сбор всех вкладов на процессе 0
    MPI_Gather(&contribution, 1, MPI_INT, contributions, 1, MPI_INT, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        for (int i = 0; i < size; i++) {
            total_sum += contributions[i];
        }
        printf("sum = %d\n", total_sum);
        delete[] array;
        delete[] contributions;
    }

    MPI_Finalize();
    return 0;
}