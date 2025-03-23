#include <iostream>
#include <vector>
#include <algorithm>
#include <chrono>
#include <unordered_set>
#include <random>
#include <mpi.h>

// Константы программы
#define M 1000    // Количество строк в системах C1 и C2
#define N 50      // Количество элементов в каждой строке
#define MAX 50     // Максимальное значение элементов

using namespace std;

// Генерация случайной C-системы с уникальными элементами в строках
vector<vector<int>> generate_c_system(int rows) {
    vector<vector<int>> c(rows, vector<int>(N));
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dist(0, MAX);

    for (int i = 0; i < rows; ++i) {
        unordered_set<int> s; // Используем set для гарантии уникальности
        while (s.size() < N) {
            int num = dist(gen);
            s.insert(num);
        }
        // Копируем элементы из set в строку матрицы
        copy(s.begin(), s.end(), c[i].begin());
    }
    return c;
}

// Вычисление пересечения двух C-систем
vector<vector<int>> local_intersect(const vector<vector<int>>& lc1, const vector<vector<int>>& c2) {
    vector<vector<int>> res;
    for (const auto& r1 : lc1) {
        unordered_set<int> s1(r1.begin(), r1.end());
        for (const auto& r2 : c2) {
            vector<int> tmp;
            // Поиск общих элементов между строками
            for (int num : r2) 
                if (s1.count(num)) 
                    tmp.push_back(num);
            // Сохраняем непустые пересечения
            if (!tmp.empty()) 
                res.push_back(tmp);
        }
    }
    return res;
}

// Функция для вывода матрицы с ограничением количества строк
// void print_matrix(const vector<vector<int>>& matrix, const string& name, int limit = 5) {
//     cout << name << " (" << matrix.size() << " rows):" << endl;
//     for (size_t i = 0; i < min(matrix.size(), static_cast<size_t>(limit)); ++i) {
//         cout << "Row " << i+1 << ": ";
//         for (int num : matrix[i]) {
//             cout << num << " ";
//         }
//         cout << endl;
//     }
//     if (matrix.size() > limit) {
//         cout << "... (showing first " << limit << " rows)" << endl;
//     }
//     cout << endl;
// }

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    vector<vector<int>> local_c1, local_res; // Локальные данные процесса
    vector<int> c1_flat;                     // Буфер для рассылки C1 (только root)
    vector<vector<int>> c2;                  // Полная система C2 (есть у всех)
    int c2_rows;                             // Количество строк в C2
    chrono::time_point<chrono::high_resolution_clock> start;

    // Этап 1: Подготовка данных (только root процесс)
    if (rank == 0) {
        // Генерация исходных систем
        auto c1 = generate_c_system(M);
        c2 = generate_c_system(M);
        c2_rows = c2.size();

        // Вывод сгенерированных матриц
        // print_matrix(c1, "C1");
        // print_matrix(c2, "C2");

        // "Расплющиваем" C1 для передачи через MPI
        for (const auto& row : c1) 
            c1_flat.insert(c1_flat.end(), row.begin(), row.end());

        // Запуск таймера (после генерации данных)
        start = chrono::high_resolution_clock::now();
    }

    // Этап 2: Рассылка C2 всем процессам
    // 2.1 Передача количества строк в C2
    MPI_Bcast(&c2_rows, 1, MPI_INT, 0, MPI_COMM_WORLD);
    
    // 2.2 Подготовка буфера и передача данных C2
    c2.resize(c2_rows, vector<int>(N));
    for (auto& row : c2) 
        MPI_Bcast(row.data(), N, MPI_INT, 0, MPI_COMM_WORLD);

    // Этап 3: Распределение C1 между процессами
    vector<int> sendcounts(size), displs(size); // Параметры рассылки
    int base_rows = M / size;                   // Базовое число строк на процесс
    int extra_rows = M % size;                  // Остаток для первых процессов

    // Расчет параметров распределения данных
    int offset = 0;
    for (int i = 0; i < size; ++i) {
        int rows = (i < extra_rows) ? base_rows + 1 : base_rows;
        sendcounts[i] = rows * N;  // Количество элементов для процесса i
        displs[i] = offset;        // Смещение в исходном массиве
        offset += sendcounts[i];    // Обновление смещения
    }

    // Приемный буфер для локальной части C1
    int local_size = (rank < extra_rows) ? (base_rows + 1)*N : base_rows*N;
    vector<int> local_c1_flat(local_size);

    // 3.1 Распределение данных C1
    MPI_Scatterv(
        c1_flat.data(),     // Исходный буфер (только root)
        sendcounts.data(),  // Количество элементов для каждого процесса
        displs.data(),      // Смещения в исходном буфере
        MPI_INT,            // Тип данных
        local_c1_flat.data(), // Приемный буфер
        local_size,         // Максимальное количество элементов для приема
        MPI_INT,            // Тип данных
        0,                  // Root-процесс
        MPI_COMM_WORLD
    );

    // 3.2 Восстановление локальной C1 из плоского массива
    int local_rows = local_size / N;
    local_c1.resize(local_rows, vector<int>(N));
    for (int i = 0; i < local_rows; ++i) 
        copy(
            local_c1_flat.begin() + i*N,
            local_c1_flat.begin() + (i+1)*N,
            local_c1[i].begin()
        );

    // Этап 4: Локальные вычисления пересечений
    local_res = local_intersect(local_c1, c2);

    // Этап 5: Подготовка к сбору результатов
    vector<int> flat; // Упакованные данные: [длина строки, элементы...]
    for (const auto& row : local_res) {
        flat.push_back(row.size());
        flat.insert(flat.end(), row.begin(), row.end());
    }

    // 5.1 Сбор информации о размерах данных от всех процессов
    vector<int> counts(size), displs_gather(size);
    int local_flat_size = flat.size();
    MPI_Gather(&local_flat_size, 1, MPI_INT, counts.data(), 1, MPI_INT, 0, MPI_COMM_WORLD);

    // 5.2 Расчет смещений для Gatherv (только root)
    vector<int> global_flat;
    if (rank == 0) {
        displs_gather[0] = 0;
        for (int i = 1; i < size; ++i) 
            displs_gather[i] = displs_gather[i-1] + counts[i-1];
        
        global_flat.resize(displs_gather.back() + counts.back());
    }

    // 5.3 Сбор всех данных на root-процесс
    MPI_Gatherv(
        flat.data(),         // Локальные данные
        local_flat_size,     // Размер локальных данных
        MPI_INT,             // Тип данных
        global_flat.data(),  // Приемный буфер (только root)
        counts.data(),       // Количество элементов от каждого процесса
        displs_gather.data(), // Смещения в приемном буфере
        MPI_INT,            // Тип данных
        0,                   // Root-процесс
        MPI_COMM_WORLD
    );

    // Этап 6: Обработка результатов (только root)
    if (rank == 0) {
        vector<vector<int>> result;
        int idx = 0;
        // Распаковка данных из плоского массива
        while (idx < global_flat.size()) {
            int len = global_flat[idx++];
            vector<int> row(global_flat.begin() + idx, global_flat.begin() + idx + len);
            idx += len;
            result.push_back(row);
        }
        
        // Замер времени и вывод результатов
        auto end = chrono::high_resolution_clock::now();

        // Вывод результата пересечения
        // print_matrix(result, "Intersection Result");

        cout << "Time: " << chrono::duration_cast<chrono::milliseconds>(end - start).count() << " ms\n";
    }

    MPI_Finalize();
    return 0;
}