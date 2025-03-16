#include <stdio.h>
#include "mpi.h"
#include <iostream>

int main(int argc, char **argv) {
    int rank, size;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    // Обходим все числа от 1 до 10
    for (int number = 1; number <= 10; ++number) {
        // Проверяем, должен ли текущий процесс обработать это число
        if ((number - 1) % size == rank) {
            // Выводим таблицу умножения для данного числа
            for (int i = 1; i <= 10; ++i) {
                std::cout << number << " x " << i << " = " << (number * i) << std::endl;
            }
        }
    }

    MPI_Finalize();

    return 0;
}