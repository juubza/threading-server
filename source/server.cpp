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

#define INT_SOCK_PATH "/tmp/pipe_int"    // Caminho do socket para inteiros
#define STRING_SOCK_PATH "/tmp/pipe_string" // Caminho do socket para strings
#define NUM_THREADS 4                    // Número de threads do servidor (não utilizado diretamente, mas pode ser útil)

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; // Inicialização do mutex para proteção de dados compartilhados entre threads
char buffer[1024];  // Buffer para comunicação entre o servidor e o cliente

// Classe para medir o tempo de execução de um código
class Timer {
public:
    Timer() : start(std::chrono::steady_clock::now()) {} // Construtor que armazena o tempo de início
    double GetElapsed() { // Função para retornar o tempo decorrido em segundos
        auto end = std::chrono::steady_clock::now(); // Captura o tempo final
        auto duration = end - start; // Calcula a duração entre o início e o fim
        return std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count() * 1.e-9; // Converte para segundos
    }
private:
    std::chrono::steady_clock::time_point start; // Variável para armazenar o tempo de início
};

// Função para testar alocação e desalocação de memória repetidamente
void memoryAllocationTest(size_t bufferSize, int iterations, char* resultBuffer) {
    Timer timer;  // Instancia o temporizador
    for (int i = 0; i < iterations; ++i) { // Loop para alocar e desalocar memória várias vezes
        int* p = new int[bufferSize / sizeof(int)]; // Aloca memória
        memset(p, 1, bufferSize); // Inicializa o bloco de memória com 1
        delete[] p; // Desaloca a memória
    }
    // Armazena o resultado no buffer, incluindo o tempo gasto
    snprintf(resultBuffer, 1024, "Alocação e desalocação de %d MB %d vezes levou %1.4f s\n",
             (int)(bufferSize / (1024 * 1024)), iterations, timer.GetElapsed());
}

// Função para testar alocação contínua de memória por um período específico
void continuousMemoryAllocationTest(size_t bufferSize, int durationSeconds, char* resultBuffer) {
    Timer timer;  // Instancia o temporizador
    int iterations = 0;  // Contador de iterações
    auto endTime = std::chrono::steady_clock::now() + std::chrono::seconds(durationSeconds); // Define o tempo de término

    while (std::chrono::steady_clock::now() < endTime) { // Loop enquanto o tempo não atingir o limite
        int* p = new int[bufferSize / sizeof(int)]; // Aloca memória
        memset(p, 1, bufferSize); // Inicializa o bloco de memória
        delete[] p; // Desaloca a memória
        iterations++;  // Incrementa o contador de iterações
    }
    // Armazena o resultado no buffer, incluindo o número de iterações realizadas
    snprintf(resultBuffer, 1024, "Alocação e desalocação de %d MB por %d segundos levou %d iterações\n",
             (int)(bufferSize / (1024 * 1024)), durationSeconds, iterations);
}

// Função que define o comportamento do servidor para lidar com conexões dos clientes
void* server_thread(void* arg) {
    int sockfd = *(int*)arg; // Obtém o socket do servidor
    int newsockfd;
    struct sockaddr_un remote; // Estrutura para armazenar informações sobre o cliente
    socklen_t len = sizeof(remote); // Tamanho da estrutura de endereço

    while (1) {  // Loop infinito para aceitar conexões
        newsockfd = accept(sockfd, (struct sockaddr*)&remote, &len); // Aceita uma conexão do cliente
        if (newsockfd < 0) { // Se não for possível aceitar a conexão, imprime erro e continua o loop
            perror("Falha em aceitar conexão.");
            continue;
        }

        printf("Cliente conectado!\n");

        memset(buffer, 0, sizeof(buffer)); // Limpa o buffer antes de usar
        if (read(newsockfd, buffer, sizeof(buffer)) < 0) { // Lê dados do cliente
            perror("Falha em ler do socket.");
            close(newsockfd);
            continue;
        }

        printf("Dado solicitado: %s\n", buffer);

        pthread_mutex_lock(&mutex);  // Bloqueia o mutex para garantir acesso exclusivo à memória
        if (buffer[0] == 'I' || buffer[0] == 'i') { // Se o tipo de solicitação for "I"
            continuousMemoryAllocationTest(32 * 1024 * 1024, 10, buffer); // Teste de alocação contínua com 32 MB
        } else if (buffer[0] == 'S' || buffer[0] == 's') { // Se o tipo de solicitação for "S"
            continuousMemoryAllocationTest(16 * 1024 * 1024, 10, buffer); // Teste de alocação contínua com 16 MB
        } else {
            snprintf(buffer, sizeof(buffer), "Solicitação inválida"); // Caso inválido
        }
        pthread_mutex_unlock(&mutex);  // Libera o mutex

        if (write(newsockfd, buffer, strlen(buffer) + 1) < 0) { // Envia resposta ao cliente
            perror("Falha em escrever no socket.");
        }

        close(newsockfd); // Fecha o socket da conexão com o cliente
        printf("Dado enviado ao cliente.\n");
    }

    return NULL; // Finaliza o thread do servidor
}

// Função para configurar o socket, criar e ouvir por conexões
void setup_socket(const char* path, int* sockfd) {
    struct sockaddr_un local;  // Estrutura de endereço para socket UNIX
    memset(&local, 0, sizeof(local)); // Limpa a estrutura
    local.sun_family = AF_UNIX;  // Define o tipo de socket como UNIX
    strncpy(local.sun_path, path, sizeof(local.sun_path) - 1); // Define o caminho do socket

    unlink(local.sun_path);  // Remove qualquer socket existente no caminho
    if (bind(*sockfd, (struct sockaddr*)&local, sizeof(local)) < 0) { // Vincula o socket
        perror("Falha em capturar o socket.");
        exit(EXIT_FAILURE);
    }
    if (listen(*sockfd, 5) < 0) { // Coloca o socket em modo de escuta
        perror("Falha em escutar o socket.");
        exit(EXIT_FAILURE);
    }
}

int main() {
    int int_sockfd, string_sockfd;
    pthread_t server_thread_int, server_thread_string;  // Threads para os servidores de inteiros e strings

    int_sockfd = socket(AF_UNIX, SOCK_STREAM, 0); // Cria o socket para inteiros
    if (int_sockfd < 0) {
        perror("Falha em criar o socket INT.");
        exit(EXIT_FAILURE);
    }

    string_sockfd = socket(AF_UNIX, SOCK_STREAM, 0); // Cria o socket para strings
    if (string_sockfd < 0) {
        perror("Falha em criar o socket STRING.");
        exit(EXIT_FAILURE);
    }

    setup_socket(INT_SOCK_PATH, &int_sockfd); // Configura o socket para inteiros
    setup_socket(STRING_SOCK_PATH, &string_sockfd); // Configura o socket para strings

    printf("Servidor ouvindo em %s e %s.\n", STRING_SOCK_PATH, INT_SOCK_PATH);

    pthread_create(&server_thread_int, NULL, server_thread, &int_sockfd); // Cria a thread do servidor para inteiros
    pthread_create(&server_thread_string, NULL, server_thread, &string_sockfd); // Cria a thread do servidor para strings

    pthread_join(server_thread_int, NULL); // Aguarda a finalização da thread do servidor para inteiros
    pthread_join(server_thread_string, NULL); // Aguarda a finalização da thread do servidor para strings

    close(int_sockfd);  // Fecha o socket de inteiros
    close(string_sockfd);  // Fecha o socket de strings

    return 0;  // Finaliza o programa
}