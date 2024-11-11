#include <iostream>
#include <chrono>
#include <thread>
#include <vector>
#include <cstring>

// Função de medição de tempo
class Timer {
public:
    Timer() : start(std::chrono::steady_clock::now()) {}
    double elapsed() const {
        auto end = std::chrono::steady_clock::now();
        return std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() / 1000.0;
    }
private:
    std::chrono::steady_clock::time_point start;
};

// Função de carga de memória
void memoryAllocationTest(size_t bufferSize, int iterations) {
    Timer timer;
    for (int i = 0; i < iterations; ++i) {
        int* buffer = new int[bufferSize / sizeof(int)];
        memset(buffer, 1, bufferSize); // Acesso de gravação
        delete[] buffer;
    }
    std::cout << "Tempo para alocar e desalocar " << bufferSize / (1024 * 1024) 
              << " MB, " << iterations << " vezes: " << timer.elapsed() << " s\n";
}

// Função para executar em várias threads
void threadedTest(int numThreads, size_t bufferSize, int iterations) {
    std::vector<std::thread> threads;
    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back(memoryAllocationTest, bufferSize, iterations);
    }
    for (auto& t : threads) t.join();
}

int main() {
    const size_t bufferSize = 32 * 1024 * 1024; // Tamanho do buffer de 32MB
    const int iterations = 100;                 // Número de iterações por thread

    std::cout << "Teste em execução com 1 thread...\n";
    memoryAllocationTest(bufferSize, iterations);

    std::cout << "Teste em execução com 4 threads...\n";
    threadedTest(4, bufferSize, iterations);

    std::cout << "Teste em execução com 8 threads...\n";
    threadedTest(8, bufferSize, iterations);

    return 0;
}
