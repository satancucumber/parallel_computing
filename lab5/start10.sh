#!/bin/bash

# Значения N для запуска
N_VALUES=(1 2 4 6 8 12 16 32)
REPEATS=10  # Количество запусков для каждого N

for n in "${N_VALUES[@]}"; do
    total=0
    successful_runs=0

    echo "Запуск для N=$n..."
    for ((i=1; i<=$REPEATS; i++)); do
        # Запуск программы и получение времени выполнения
        output=$(mpiexec -n $n ./parallel 2>&1)
        time_ms=$(echo "$output" | grep 'Time:' | awk '{print $2}')

        # Проверка корректности времени
        if [[ $time_ms =~ ^[0-9]+$ ]]; then
            total=$((total + time_ms))
            successful_runs=$((successful_runs + 1))
        else
            echo "Ошибка при запуске (N=$n, попытка $i)"
        fi
    done

    if [[ $successful_runs -eq 0 ]]; then
        avg="N/A"
    else
        avg=$((total / successful_runs))
    fi

    echo "$n: $avg"
done