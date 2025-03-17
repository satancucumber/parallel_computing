#include <cstdlib>
#include <iostream>
#include <ctime>
#include "mpi.h" 

int main(int argc, char *argv[]) {
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    const int total_elements = 20;
    int elements_per_process = total_elements / size;

    int *array = nullptr;
    int *local_array = new int[elements_per_process];
    int local_sum = 0, total_sum = 0;

    if (rank == 0) {
        array = new int[total_elements];
        srand(time(NULL));
        std::cout << "Generated array: [";
        for (int i = 0; i < total_elements; i++) {
            array[i] = (rand() % 10) + 1;
            std::cout << array[i] << (i < total_elements-1 ? ", " : "]");
        }
        std::cout << std::endl;

        // Отправка частей массива другим процессам
        for (int i = 1; i < size; i++) {
            int start = i * elements_per_process;
            MPI_Send(array + start, elements_per_process, MPI_INT, i, 0, MPI_COMM_WORLD);
        }

        // Обработка своей части
        for (int i = 0; i < elements_per_process; i++) {
            local_array[i] = array[i];
        }
    } else {
        // Получение своей части массива
        MPI_Recv(local_array, elements_per_process, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }

    // Вычисление локальной суммы кратных 3
    for (int i = 0; i < elements_per_process; i++) {
        if (local_array[i] % 3 == 0) {
            local_sum += local_array[i];
        }
    }

    // Сбор данных для вывода
    if (rank == 0) {
        int **gathered_arrays = new int*[size];
        int *gathered_sums = new int[size];
        
        gathered_arrays[0] = local_array;
        gathered_sums[0] = local_sum;

        // Получение данных от других процессов
        for (int i = 1; i < size; i++) {
            gathered_arrays[i] = new int[elements_per_process];
            MPI_Recv(gathered_arrays[i], elements_per_process, MPI_INT, i, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Recv(&gathered_sums[i], 1, MPI_INT, i, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }

        // Вывод информации по процессам
        for (int i = 0; i < size; i++) {
            std::cout << "rank=" << i << " [";
            for (int j = 0; j < elements_per_process; j++) {
                std::cout << gathered_arrays[i][j];
                if(j < elements_per_process-1) std::cout << ", ";
            }
            std::cout << "] sum=" << gathered_sums[i] << std::endl;
        }

        // Очистка памяти
        for(int i = 1; i < size; i++) delete[] gathered_arrays[i];
        delete[] gathered_arrays;
        delete[] gathered_sums;
    } else {
        // Отправка данных нулевому процессу
        MPI_Send(local_array, elements_per_process, MPI_INT, 0, 1, MPI_COMM_WORLD);
        MPI_Send(&local_sum, 1, MPI_INT, 0, 2, MPI_COMM_WORLD);
    }

    // Сбор всех сумм и вывод результата
    MPI_Reduce(&local_sum, &total_sum, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        std::cout << "Total sum of elements divisible by 3: " << total_sum << std::endl;
        delete[] array;
    }

    delete[] local_array;
    MPI_Finalize();
    return 0;
}