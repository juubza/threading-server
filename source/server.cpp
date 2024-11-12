#include <chrono>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <pthread.h>
#include <ctype.h>
#include <stdlib.h>

#define INT_SOCK_PATH "/tmp/pipe_int"
#define STRING_SOCK_PATH "/tmp/pipe_string"
#define NUM_THREADS 4

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
char buffer[1024];

class Timer {
public:
    Timer() : start(std::chrono::steady_clock::now()) {}
    double GetElapsed() {
        auto end = std::chrono::steady_clock::now();
        auto duration = end - start;
        return std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count() * 1.e-9;
    }
private:
    std::chrono::steady_clock::time_point start;
};

void memoryAllocationTest(size_t bufferSize, int iterations, char* resultBuffer) {
    Timer timer;
    for (int i = 0; i < iterations; ++i) {
        int* p = new int[bufferSize / sizeof(int)];
        memset(p, 1, bufferSize);
        delete[] p;
    }
    snprintf(resultBuffer, 1024, "Alocação e desalocação de %d MB %d vezes levou %1.4f s\n",
             (int)(bufferSize / (1024 * 1024)), iterations, timer.GetElapsed());
}

// Função de teste de alocação contínua
void continuousMemoryAllocationTest(size_t bufferSize, int durationSeconds, char* resultBuffer) {
    Timer timer;
    int iterations = 0;
    auto endTime = std::chrono::steady_clock::now() + std::chrono::seconds(durationSeconds);
    
    while (std::chrono::steady_clock::now() < endTime) {
        int* p = new int[bufferSize / sizeof(int)];
        memset(p, 1, bufferSize);
        delete[] p;
        iterations++;
    }
    snprintf(resultBuffer, 1024, "Alocação e desalocação de %d MB por %d segundos levou %d iterações\n",
             (int)(bufferSize / (1024 * 1024)), durationSeconds, iterations);
}

void* server_thread(void* arg) {
    int sockfd = *(int*)arg;
    int newsockfd;
    struct sockaddr_un remote;
    socklen_t len = sizeof(remote);

    while (1) {
        newsockfd = accept(sockfd, (struct sockaddr*)&remote, &len);
        if (newsockfd < 0) {
            perror("Falha em aceitar conexão.");
            continue;
        }
        
        printf("Cliente conectado!\n");

        memset(buffer, 0, sizeof(buffer));
        if (read(newsockfd, buffer, sizeof(buffer)) < 0) {
            perror("Falha em ler do socket.");
            close(newsockfd);
            continue;
        }

        printf("Dado solicitado: %s\n", buffer);

        pthread_mutex_lock(&mutex);
        if (buffer[0] == 'I' || buffer[0] == 'i') {
            continuousMemoryAllocationTest(32 * 1024 * 1024, 60, buffer); // 32 MB por 60 segundos
        } else if (buffer[0] == 'S' || buffer[0] == 's') {
            continuousMemoryAllocationTest(16 * 1024 * 1024, 60, buffer); // 16 MB por 60 segundos
        } else {
            snprintf(buffer, sizeof(buffer), "Solicitação inválida");
        }
        pthread_mutex_unlock(&mutex);

        if (write(newsockfd, buffer, strlen(buffer) + 1) < 0) {
            perror("Falha em escrever no socket.");
        }

        close(newsockfd);
        printf("Dado enviado ao cliente.\n");
    }

    return NULL;
}

void setup_socket(const char* path, int* sockfd) {
    struct sockaddr_un local;
    memset(&local, 0, sizeof(local));
    local.sun_family = AF_UNIX;
    strncpy(local.sun_path, path, sizeof(local.sun_path) - 1);

    unlink(local.sun_path);
    if (bind(*sockfd, (struct sockaddr*)&local, sizeof(local)) < 0) {
        perror("Falha em capturar o socket.");
        exit(EXIT_FAILURE);
    }
    if (listen(*sockfd, 5) < 0) {
        perror("Falha em escutar o socket.");
        exit(EXIT_FAILURE);
    }
}

int main() {
    int int_sockfd, string_sockfd;
    pthread_t server_thread_int, server_thread_string;

    int_sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (int_sockfd < 0) {
        perror("Falha em criar o socket INT.");
        exit(EXIT_FAILURE);
    }

    string_sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (string_sockfd < 0) {
        perror("Falha em criar o socket STRING.");
        exit(EXIT_FAILURE);
    }

    setup_socket(INT_SOCK_PATH, &int_sockfd);
    setup_socket(STRING_SOCK_PATH, &string_sockfd);

    printf("Servidor ouvindo em %s e %s.\n", STRING_SOCK_PATH, INT_SOCK_PATH);

    pthread_create(&server_thread_int, NULL, server_thread, &int_sockfd);
    pthread_create(&server_thread_string, NULL, server_thread, &string_sockfd);

    pthread_join(server_thread_int, NULL);
    pthread_join(server_thread_string, NULL);

    close(int_sockfd);
    close(string_sockfd);

    return 0;
}