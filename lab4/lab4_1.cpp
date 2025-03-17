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
    }

    // Распределение частей массива с помощью MPI_Scatter
    MPI_Scatter(array, elements_per_process, MPI_INT, 
                local_array, elements_per_process, MPI_INT, 
                0, MPI_COMM_WORLD);

    // Вычисление локальной суммы кратных 3
    for (int i = 0; i < elements_per_process; i++) {
        if (local_array[i] % 3 == 0) {
            local_sum += local_array[i];
        }
    }

    // Сбор локальных массивов и сумм для вывода
    int *gathered_data = nullptr;
    int *gathered_sums = nullptr;

    if (rank == 0) {
        gathered_data = new int[total_elements];
        gathered_sums = new int[size];
    }

    MPI_Gather(local_array, elements_per_process, MPI_INT, 
               gathered_data, elements_per_process, MPI_INT, 
               0, MPI_COMM_WORLD);
    MPI_Gather(&local_sum, 1, MPI_INT, 
               gathered_sums, 1, MPI_INT, 
               0, MPI_COMM_WORLD);

    // Вывод информации по процессам
    if (rank == 0) {
        for (int i = 0; i < size; i++) {
            std::cout << "rank=" << i << " [";
            int start = i * elements_per_process;
            for (int j = 0; j < elements_per_process; j++) {
                std::cout << gathered_data[start + j];
                if (j < elements_per_process - 1) std::cout << ", ";
            }
            std::cout << "] sum=" << gathered_sums[i] << std::endl;
        }
        delete[] gathered_data;
        delete[] gathered_sums;
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