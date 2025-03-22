#include <stdio.h>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include "mpi.h"

#define ROWS 5       // Количество строк в матрице
#define COLUMNS 6    // Количество столбцов в матрице

int main(int argc, char **argv) {
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Локальные данные для каждого процесса
    int *local_matrix = nullptr;    // Исходные данные процесса
    int *local_modified = nullptr;  // Модифицированные данные процесса
    int local_rows = 0;             // Количество строк, обрабатываемых процессом

    // Этап 1: Генерация и распределение данных
    if (rank == 0) {
        // Только корневой процесс создает полную матрицу
        int matrix[ROWS][COLUMNS];
        srand(time(NULL));
        std::cout << "Исходная матрица:\n";
        
        // Заполнение матрицы случайными значениями 0/1
        for (int i = 0; i < ROWS; i++) {
            for (int j = 0; j < COLUMNS; j++) {
                matrix[i][j] = rand() % 2;
                std::cout << matrix[i][j] << " ";
            }
            std::cout << "\n";
        }

        // Распределение строк между процессами
        for (int dest = 0; dest < size; dest++) {
            // Подсчет строк для текущего процесса-получателя
            int num_rows = 0;
            for (int i = 0; i < ROWS; i++) {
                if (i % size == dest) num_rows++;
            }
            
            // Упаковка данных для отправки
            int *send_buffer = new int[num_rows * COLUMNS];
            int idx = 0;
            for (int i = 0; i < ROWS; i++) {
                if (i % size == dest) {
                    for (int j = 0; j < COLUMNS; j++) {
                        send_buffer[idx * COLUMNS + j] = matrix[i][j];
                    }
                    idx++;
                }
            }
            
            // Отправка данных процессам (или сохранение для себя)
            if (dest == 0) {
                local_rows = num_rows;
                local_matrix = new int[local_rows * COLUMNS];
                std::copy(send_buffer, send_buffer + num_rows * COLUMNS, local_matrix);
            } else {
                MPI_Send(&num_rows, 1, MPI_INT, dest, 0, MPI_COMM_WORLD);
                MPI_Send(send_buffer, num_rows * COLUMNS, MPI_INT, dest, 0, MPI_COMM_WORLD);
            }
            delete[] send_buffer;
        }
    } else {
        // Получение данных от корневого процесса
        MPI_Recv(&local_rows, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        local_matrix = new int[local_rows * COLUMNS];
        MPI_Recv(local_matrix, local_rows * COLUMNS, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }

    // Этап 2: Обработка данных
    // Инициализация модифицированной матрицы
    local_modified = new int[local_rows * COLUMNS];
    std::copy(local_matrix, local_matrix + local_rows * COLUMNS, local_modified);

    // Инвертирование нечетных строк
    for (int local_i = 0; local_i < local_rows; local_i++) {
        // Вычисление глобального индекса строки
        int global_i = rank + local_i * size;
        if (global_i % 2 == 1) {  // Если строка нечетная
            for (int j = 0; j < COLUMNS; j++) {
                local_modified[local_i * COLUMNS + j] = 1 - local_modified[local_i * COLUMNS + j];
            }
        }
    }

    // Этап 3: Сбор результатов на корневом процессе
    if (rank == 0) {
        int modified_matrix[ROWS][COLUMNS];  // Полная модифицированная матрица
        
        // Инициализация матрицы данными корневого процесса
        for (int i = 0; i < ROWS; i++) {
            for (int j = 0; j < COLUMNS; j++) {
                modified_matrix[i][j] = (i % size == 0) ? local_matrix[(i / size) * COLUMNS + j] : 0;
            }
        }

        // Заполнение данных корневого процесса
        int idx = 0;
        for (int i = 0; i < ROWS; i++) {
            if (i % size == 0) {
                for (int j = 0; j < COLUMNS; j++) {
                    modified_matrix[i][j] = local_modified[idx * COLUMNS + j];
                }
                idx++;
            }
        }

        // Получение данных от других процессов
        for (int src = 1; src < size; src++) {
            int num_rows;
            MPI_Recv(&num_rows, 1, MPI_INT, src, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            int *recv_buffer = new int[num_rows * COLUMNS];
            MPI_Recv(recv_buffer, num_rows * COLUMNS, MPI_INT, src, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            // Распаковка полученных данных
            idx = 0;
            for (int i = 0; i < ROWS; i++) {
                if (i % size == src) {
                    for (int j = 0; j < COLUMNS; j++) {
                        modified_matrix[i][j] = recv_buffer[idx * COLUMNS + j];
                    }
                    idx++;
                }
            }
            delete[] recv_buffer;
        }

        // Этап 4: Рассылка обновленной матрицы всем процессам
        for (int dest = 0; dest < size; dest++) {
            int num_rows = 0;
            for (int i = 0; i < ROWS; i++) {
                if (i % size == dest) num_rows++;
            }
            
            // Упаковка данных для рассылки
            int *send_buffer = new int[num_rows * COLUMNS];
            int idx = 0;
            for (int i = 0; i < ROWS; i++) {
                if (i % size == dest) {
                    for (int j = 0; j < COLUMNS; j++) {
                        send_buffer[idx * COLUMNS + j] = modified_matrix[i][j];
                    }
                    idx++;
                }
            }
            
            // Рассылка данных
            if (dest == 0) {
                std::copy(send_buffer, send_buffer + num_rows * COLUMNS, local_modified);
            } else {
                MPI_Send(send_buffer, num_rows * COLUMNS, MPI_INT, dest, 2, MPI_COMM_WORLD);
            }
            delete[] send_buffer;
        }

        // Вывод результатов
        std::cout << "\nМодифицированная матрица:\n";
        for (int i = 0; i < ROWS; i++) {
            for (int j = 0; j < COLUMNS; j++) {
                std::cout << modified_matrix[i][j] << " ";
            }
            std::cout << "\n";
        }
    } else {
        // Отправка данных корневому процессу
        MPI_Send(&local_rows, 1, MPI_INT, 0, 1, MPI_COMM_WORLD);
        MPI_Send(local_modified, local_rows * COLUMNS, MPI_INT, 0, 1, MPI_COMM_WORLD);

        // Получение обновленной матрицы
        MPI_Recv(local_modified, local_rows * COLUMNS, MPI_INT, 0, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }

    // Этап 5: Подсчет отрицательных элементов
    int local_count = 0;
    for (int i = 0; i < local_rows * COLUMNS; i++) {
        if (local_modified[i] < 0) {
            local_count++;
        }
    }

    // Сбор статистики на корневом процессе
    int global_count = 0;
    if (rank == 0) {
        global_count = local_count;
        for (int src = 1; src < size; src++) {
            int temp;
            MPI_Recv(&temp, 1, MPI_INT, src, 3, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            global_count += temp;
        }
        double average = static_cast<double>(global_count) / ROWS;
        std::cout << "\nСреднее количество отрицательных элементов: " << average << std::endl;
    } else {
        MPI_Send(&local_count, 1, MPI_INT, 0, 3, MPI_COMM_WORLD);
    }

    // Освобождение ресурсов
    delete[] local_matrix;
    delete[] local_modified;

    MPI_Finalize();
    return 0;
}