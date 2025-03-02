#include <stdio.h>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include "mpi.h"

#define ROWS 5
#define COLUMNS 6

int main(int argc, char **argv) {
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int matrix[ROWS][COLUMNS];
    int modified_matrix[ROWS][COLUMNS];

    // Генерация исходной матрицы (0 и 1) на процессе 0
    if (rank == 0) {
        srand(time(NULL));
        std::cout << "Исходная матрица:\n";
        for (int i = 0; i < ROWS; i++) {
            for (int j = 0; j < COLUMNS; j++) {
                matrix[i][j] = rand() % 2;
                std::cout << matrix[i][j] << " ";
            }
            std::cout << "\n";
        }
    }

    // Рассылка исходной матрицы от процесса 0 всем остальным
    if (rank == 0) {
        for (int dest = 1; dest < size; dest++) {
            for (int i = 0; i < ROWS; i++) {
                MPI_Send(matrix[i], COLUMNS, MPI_INT, dest, 0, MPI_COMM_WORLD);
            }
        }
    } else {
        for (int i = 0; i < ROWS; i++) {
            MPI_Recv(matrix[i], COLUMNS, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
    }

    // Инициализация modified_matrix
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLUMNS; j++) {
            modified_matrix[i][j] = matrix[i][j];
        }
    }

    // Обработка нечетных строк
    for (int i = 0; i < ROWS; i++) {
        if (i % size == rank) {  // Распределение строк между процессами
            if (i % 2 == 1) {   // Инвертирование нечетных строк
                for (int j = 0; j < COLUMNS; j++) {
                    modified_matrix[i][j] = 1 - modified_matrix[i][j];
                }
            }
        }
    }

    // Сбор модифицированных строк на процессе 0
    if (rank == 0) {
        for (int i = 0; i < ROWS; i++) {
            int source = i % size;
            if (source != 0) {
                MPI_Recv(modified_matrix[i], COLUMNS, MPI_INT, source, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            }
        }
    } else {
        for (int i = 0; i < ROWS; i++) {
            if (i % size == rank) {
                MPI_Send(modified_matrix[i], COLUMNS, MPI_INT, 0, 1, MPI_COMM_WORLD);
            }
        }
    }

    // Рассылка обновленной матрицы всем процессам
    if (rank == 0) {
        for (int dest = 1; dest < size; dest++) {
            for (int i = 0; i < ROWS; i++) {
                MPI_Send(modified_matrix[i], COLUMNS, MPI_INT, dest, 2, MPI_COMM_WORLD);
            }
        }
    } else {
        for (int i = 0; i < ROWS; i++) {
            MPI_Recv(modified_matrix[i], COLUMNS, MPI_INT, 0, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
    }

    // Подсчет отрицательных элементов
    int local_count = 0;
    for (int i = rank; i < ROWS; i += size) {
        for (int j = 0; j < COLUMNS; j++) {
            if (modified_matrix[i][j] < 0) {
                local_count++;
            }
        }
    }

    // Сбор результатов на процессе 0
    int global_count = 0;
    if (rank == 0) {
        global_count = local_count;
        for (int src = 1; src < size; src++) {
            int temp;
            MPI_Recv(&temp, 1, MPI_INT, src, 3, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            global_count += temp;
        }
        double average = static_cast<double>(global_count) / ROWS;
        
        std::cout << "\nМодифицированная матрица:\n";
        for (int i = 0; i < ROWS; i++) {
            for (int j = 0; j < COLUMNS; j++) {
                std::cout << modified_matrix[i][j] << " ";
            }
            std::cout << "\n";
        }
        std::cout << "\nСреднее количество отрицательных элементов: " 
                  << average << std::endl;
    } else {
        MPI_Send(&local_count, 1, MPI_INT, 0, 3, MPI_COMM_WORLD);
    }

    MPI_Finalize();
    return 0;
}