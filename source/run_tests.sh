#!/bin/bash

# Compila o programa
g++ -o alocacao.exe alocacao.cpp -std=c++11

# Executa o programa para diferentes tamanhos de buffer
for mb in 8 16 32 64; do
    ./alocacao.exe $mb &
    pid=$!
    sleep 2  # Tempo para o programa rodar
    kill $pid
    echo "Finalizado o processo $pid para $mb MB"
done
