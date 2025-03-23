#include <iostream>
#include <vector>
#include <algorithm>
#include <chrono>
#include <unordered_set>
#include <random>

#define M 1000    // Количество строк в C-системе
#define N 50      // Количество элементов в строке
#define MAX 50    // Максимальное значение элемента

using namespace std;

// Генерация случайной C-системы с уникальными элементами в строках
vector<vector<int>> generate_c_system(int rows) {
    vector<vector<int>> c(rows, vector<int>(N));
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dist(0, MAX);

    for (int i = 0; i < rows; ++i) {
        unordered_set<int> s;
        while (s.size() < N) {
            int num = dist(gen);
            s.insert(num);
        }
        copy(s.begin(), s.end(), c[i].begin());
    }
    return c;
}

// Вычисление пересечения двух C-систем
vector<vector<int>> intersect(const vector<vector<int>>& c1, const vector<vector<int>>& c2) {
    vector<vector<int>> res;
    // Для каждой пары строк из c1 и c2
    for (const auto& r1 : c1) {
        unordered_set<int> s1(r1.begin(), r1.end());  // Хэш-таблица для быстрого поиска
        for (const auto& r2 : c2) {
            vector<int> tmp;
            // Поиск общих элементов
            for (int num : r2) 
                if (s1.count(num)) 
                    tmp.push_back(num);
            // Сохранение непустых пересечений
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

int main() {
    // Генерация исходных данных
    auto c1 = generate_c_system(M);
    auto c2 = generate_c_system(M);

    // Вывод сгенерированных матриц
    // print_matrix(c1, "C1");
    // print_matrix(c2, "C2");

    auto start = chrono::high_resolution_clock::now();
    auto result = intersect(c1, c2);
    auto end = chrono::high_resolution_clock::now();
    
    // Вывод результата пересечения
    // print_matrix(result, "Intersection Result");

    cout << "Time: " << chrono::duration_cast<chrono::milliseconds>(end - start).count() << " ms\n";
}