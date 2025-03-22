#include <stdio.h>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include "mpi.h"

#define ROWS 5       // Количество строк в матрице
#define COLUMNS 6    // Количество столбцов в матрице

int main(int argc, char **argv) {
    int rank, size;
    MPI_Init(&argc, &argv);                // Инициализация MPI
    MPI_Comm_rank(MPI_COMM_WORLD, &rank); // Получение номера процесса
    MPI_Comm_size(MPI_COMM_WORLD, &size); // Получение количества процессов

    // Локальные данные процесса
    int *local_matrix = nullptr;    // Локальная часть исходной матрицы
    int *local_modified = nullptr;  // Локальная часть модифицированной матрицы
    int local_rows = 0;             // Количество строк в локальной части

    // Массивы для распределения данных
    int *sendcounts = new int[size]; // Количество элементов для каждого процесса
    int *displs = new int[size];     // Смещения для каждого процесса
    int *matrix_1D = nullptr;        // Указатель на одномерное представление матрицы (только в корне)

    // Этап 1: Подготовка данных в корневом процессе
    if (rank == 0) {
        int matrix[ROWS][COLUMNS];    // Исходная матрица
        srand(time(NULL));           // Инициализация генератора случайных чисел
        
        // Заполнение матрицы случайными значениями 0/1 и вывод
        std::cout << "Исходная матрица:\n";
        for (int i = 0; i < ROWS; i++) {
            for (int j = 0; j < COLUMNS; j++) {
                matrix[i][j] = rand() % 2;
                std::cout << matrix[i][j] << " ";
            }
            std::cout << "\n";
        }

        // Преобразование матрицы в одномерный массив для MPI_Scatterv
        matrix_1D = new int[ROWS * COLUMNS];
        for (int i = 0; i < ROWS; i++) {
            for (int j = 0; j < COLUMNS; j++) {
                matrix_1D[i * COLUMNS + j] = matrix[i][j];
            }
        }

        // Вычисление параметров распределения данных
        int remainder = ROWS % size;  // Остаток от деления строк на процессы
        int base_rows = ROWS / size;  // Базовое количество строк на процесс
        int current_displ = 0;        // Текущее смещение
        
        for (int i = 0; i < size; i++) {
            sendcounts[i] = (i < remainder) ? (base_rows + 1) : base_rows; // Распределение строк
            sendcounts[i] *= COLUMNS;  // Пересчёт в количество элементов
            displs[i] = current_displ;  // Смещение для текущего процесса
            current_displ += sendcounts[i]; // Обновление смещения для следующего процесса
        }
    }

    // Рассылка параметров распределения всем процессам
    MPI_Bcast(sendcounts, size, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(displs, size, MPI_INT, 0, MPI_COMM_WORLD);

    // Вычисление размера локального блока данных
    int local_rows_elements = sendcounts[rank]; // Количество элементов в локальной части
    local_rows = local_rows_elements / COLUMNS; // Количество строк в локальной части
    
    // Выделение памяти под локальные данные
    local_matrix = new int[local_rows_elements];
    local_modified = new int[local_rows_elements];

    // Распределение данных между процессами с использованием MPI_Scatterv
    MPI_Scatterv(
        matrix_1D,          // Исходный буфер (только в корне)
        sendcounts,         // Массив количеств элементов для каждого процесса
        displs,             // Массив смещений
        MPI_INT,           // Тип данных
        local_matrix,      // Приёмный буфер
        local_rows_elements,// Максимальное количество элементов для приёма
        MPI_INT,            // Тип данных
        0,                  // Корневой процесс
        MPI_COMM_WORLD
    );

    // Освобождение памяти в корневом процессе после рассылки
    if (rank == 0) delete[] matrix_1D;

    // Копирование исходных данных в модифицированный массив
    std::copy(local_matrix, local_matrix + local_rows_elements, local_modified);

    // Этап 2: Инверсия нечётных строк
    int start_row = displs[rank] / COLUMNS; // Глобальный индекс первой строки в локальном блоке
    for (int local_i = 0; local_i < local_rows; local_i++) {
        int global_i = start_row + local_i; // Расчёт глобального индекса строки
        if (global_i % 2 == 1) {           // Если строка нечётная (индексация с 0)
            for (int j = 0; j < COLUMNS; j++) {
                // Инверсия элемента: 0 -> 1, 1 -> 0
                local_modified[local_i * COLUMNS + j] = 1 - local_modified[local_i * COLUMNS + j];
            }
        }
    }

    // Этап 3: Сбор модифицированных данных в корневом процессе
    int *modified_matrix_1D = nullptr;
    if (rank == 0) modified_matrix_1D = new int[ROWS * COLUMNS]; // Буфер для сбора данных

    MPI_Gatherv(
        local_modified,     // Отправляемые данные
        local_rows_elements,// Количество элементов
        MPI_INT,            // Тип данных
        modified_matrix_1D, // Буфер для сбора (только в корне)
        sendcounts,         // Массив количеств элементов
        displs,            // Массив смещений
        MPI_INT,            // Тип данных
        0,                  // Корневой процесс
        MPI_COMM_WORLD
    );

    // Вывод модифицированной матрицы в корневом процессе
    if (rank == 0) {
        std::cout << "\nМодифицированная матрица:\n";
        for (int i = 0; i < ROWS; i++) {
            for (int j = 0; j < COLUMNS; j++) {
                std::cout << modified_matrix_1D[i * COLUMNS + j] << " ";
            }
            std::cout << "\n";
        }
        delete[] modified_matrix_1D; // Освобождение памяти
    }

    // Этап 4: Подсчёт отрицательных элементов
    int local_count = 0;
    for (int i = 0; i < local_rows_elements; i++) {
        if (local_modified[i] < 0) local_count++; // Подсчёт в локальной части
    }

    // Суммирование результатов со всех процессов
    int global_count;
    MPI_Reduce(
        &local_count,   // Локальный счётчик
        &global_count,  // Глобальный счётчик (только в корне)
        1,              // Количество элементов
        MPI_INT,        // Тип данных
        MPI_SUM,        // Операция суммирования
        0,              // Корневой процесс
        MPI_COMM_WORLD
    );

    // Вывод результата в корневом процессе
    if (rank == 0) {
        double average = static_cast<double>(global_count) / ROWS;
        std::cout << "\nСреднее количество отрицательных элементов: " << average << std::endl;
    }

    // Освобождение ресурсов
    delete[] sendcounts;
    delete[] displs;
    delete[] local_matrix;
    delete[] local_modified;

    MPI_Finalize(); // Завершение работы MPI
    return 0;
}