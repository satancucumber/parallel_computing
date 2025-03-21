# Параллельные вычисления  
## Лабораторная работа №1  
### Запуск параллельной программы  
#### Цель работы.
Освоить процесс запуска программы на С++ с применением библиотеки MPICH2. Научиться получать сведения о количестве запущенных процессов и номере отдельного процесса.
#### Задание 1. Создать и запустить программу на 2-х процессах с применением функций int MPI_Init( int* argc, char*** argv) и int MPI_Finalize( void ).   
Компиляция и запуск программы на 2-х процессах.  
```
mpic++ lab1_1.cpp -o lab1_1
mpiexec -n 2 ./lab1_1
```
#### Задание 2. Создать и запустить программу на 3-х процессах с применением функций:
1)	int MPI_Init( int* argc, char*** argv);
2)	int MPI_Finalize( void );
3)	int MPI_Comm_size( MPI_Comm comm, int* size)
4)	int MPI_Comm_rank( MPI_Comm comm, int* rank)  
Программа должна выводить на экран номер процесса и какой-либо идентификатор процесса.
---
Компиляция и запуск программы на 3-х процессах. 
```
mpic++ lab1_2.cpp -o lab1_2
mpiexec -n 3 ./lab1_2
```
#### Задание 3. Создать и запустить программу на n-х процессах печати таблицы умножения.
Компиляция программы. 
```
mpic++ lab1_3.cpp -o lab1_3
```
Запуск программы на 2-х процессах. 
```
mpiexec -n 2 ./lab1_3
```
Запуск программы на 3-х процессах. 
```
mpiexec -n 3 ./lab1_3
```
---
## Лабораторная работа №2  
### Передача данных по процессам
#### Цель работы.
Освоить функции передачи данных между процессами.
#### Задание 1. 
1)	Запустить 4 процесса. 
2)	На каждом процессе создать переменные: ai,bi,ci, где I – номер процесса. Инициализировать переменные. Вывести данные на печать. 
3)	Передать данные на другой процесс. Напечатать номера процессов и поступившие данные. Найти: c0=a1+b2; с1=a3+b0;  c2=a0+b3; c3=a2+b1.
---
Компиляция и запуск программы на 4-х процессах. 
```
mpic++ lab2_1.cpp -o lab2_1
mpiexec -n 4 ./lab2_1
```
#### Задание 2 (вариант 5). Запустить n процессов и найти сумму элементов кратных 3.
Компиляция программы. 
```
mpic++ lab2_2.cpp -o lab2_1
```
Запуск программы на 2-х процессах. 
```
mpiexec -n 3 ./lab2_2
```
Запуск программы на 3-х процессах. 
```
mpiexec -n 5 ./lab2_2
```
