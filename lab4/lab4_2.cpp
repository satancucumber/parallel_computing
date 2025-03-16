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
    int *sendcounts = nullptr;
    int *displs = nullptr;

    // Генерация матрицы на процессе 0
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

    // Рассылка матрицы всем процессам
    MPI_Bcast(matrix, ROWS * COLUMNS, MPI_INT, 0, MPI_COMM_WORLD);

    // Вычисление параметров распределения
    int chunk_size = ROWS / size;
    int remainder = ROWS % size;
    sendcounts = new int[size];
    displs = new int[size];
    
    int offset = 0;
    for (int i = 0; i < size; i++) {
        sendcounts[i] = (i < remainder) ? chunk_size + 1 : chunk_size;
        sendcounts[i] *= COLUMNS;
        displs[i] = offset;
        offset += sendcounts[i];
    }

    // Локальный блок для обработки
    int local_rows = (rank < remainder) ? chunk_size + 1 : chunk_size;
    int *local_block = new int[local_rows * COLUMNS];

    // Разделение матрицы на блоки
    MPI_Scatterv(matrix, sendcounts, displs, MPI_INT,
                local_block, sendcounts[rank], MPI_INT,
                0, MPI_COMM_WORLD);

    // Инвертирование нечетных строк
    for (int i = 0; i < local_rows; i++) {
        int global_row = (displs[rank] / COLUMNS) + i;
        if (global_row % 2 == 1) {
            for (int j = 0; j < COLUMNS; j++) {
                int idx = i * COLUMNS + j;
                local_block[idx] = 1 - local_block[idx];
            }
        }
    }

    // Сбор модифицированных блоков
    MPI_Gatherv(local_block, sendcounts[rank], MPI_INT,
               modified_matrix, sendcounts, displs, MPI_INT,
               0, MPI_COMM_WORLD);

    // Рассылка обновленной матрицы
    MPI_Bcast(modified_matrix, ROWS * COLUMNS, MPI_INT, 0, MPI_COMM_WORLD);

    // Подсчет отрицательных элементов
    int local_count = 0;
    int start_row = displs[rank] / COLUMNS;
    int end_row = start_row + local_rows;
    
    for (int i = start_row; i < end_row; i++) {
        for (int j = 0; j < COLUMNS; j++) {
            if (modified_matrix[i][j] < 0) local_count++;
        }
    }

    // Сбор и вывод результатов
    int global_count;
    MPI_Reduce(&local_count, &global_count, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        std::cout << "\nМодифицированная матрица:\n";
        for (int i = 0; i < ROWS; i++) {
            for (int j = 0; j < COLUMNS; j++) {
                std::cout << modified_matrix[i][j] << " ";
            }
            std::cout << "\n";
        }
        double average = static_cast<double>(global_count) / ROWS;
        std::cout << "\nСреднее количество отрицательных элементов: "
                  << average << std::endl;
    }

    // Освобождение ресурсов
    delete[] sendcounts;
    delete[] displs;
    delete[] local_block;

    MPI_Finalize();
    return 0;
}