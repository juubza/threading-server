#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/resource.h>
#include <ctype.h>
#include <time.h>

#define SOCK_PATH "/tmp/pipeso"
#define MAX_STR_LEN 16

pthread_mutex_t em = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t thread_count_mutex = PTHREAD_MUTEX_INITIALIZER;
int active_threads = 0; // Contador global de threads ativas

// Função para gerar string aleatória
void rand_string(char *str, size_t size) {
    const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    if (size) {
        --size;
        for (size_t n = 0; n < size; n++) {
            int key = rand() % (int)(sizeof(charset) - 1);
            str[n] = charset[key];
        }
        str[size] = '\0';
    }
}

// Função para gerar número inteiro
int generate_int() {
    int randomNumber = rand() % (1000 - 100 + 1) + 100;
    return randomNumber;
}

// Função para gerar string
void generate_string(char *result) {
    rand_string(result, MAX_STR_LEN - 1);
}

// Função de processamento do cliente
void *handle_client(void *arg) {
    int client_sock = *((int *)arg);
    free(arg);
    char buffer[1024];
    char dataRequested;

    // Incrementa o contador de threads
    pthread_mutex_lock(&thread_count_mutex);
    active_threads++;
    printf("Threads ativas: %d\n", active_threads);
    pthread_mutex_unlock(&thread_count_mutex);

    // Monitoramento de uso de recursos
    struct rusage usage_start, usage_end;
    getrusage(RUSAGE_SELF, &usage_start); // Uso inicial por thread

    // Ler dados do cliente
    if (read(client_sock, buffer, sizeof(buffer)) < 0) {
        perror("Falha em ler do socket.");
        close(client_sock);
        return NULL;
    }

    printf("Dado solicitado: %s\n", buffer);
    dataRequested = toupper(buffer[0]);

    char response[1024];
    if (dataRequested == 'I') {
        int result = generate_int();
        snprintf(response, sizeof(response), "%d", result);
    } else if (dataRequested == 'S') {
        generate_string(response);
    } else {
        snprintf(response, sizeof(response), "Erro: Tipo de dado inválido.");
    }

    // Enviar resposta ao cliente
    if (write(client_sock, response, strlen(response) + 1) < 0) {
        perror("Falha em escrever no socket.");
        close(client_sock);
        return NULL;
    }

    printf("Dado enviado ao cliente: %s\n", response);
    close(client_sock);

    // Monitoramento final e cálculo de mudanças de página
    getrusage(RUSAGE_SELF, &usage_end);
    long diff_page_faults = usage_end.ru_majflt - usage_start.ru_majflt;
    printf("Page faults (mudanças de página) na thread: %ld\n", diff_page_faults);

    // Decrementa o contador de threads
    pthread_mutex_lock(&thread_count_mutex);
    active_threads--;
    printf("Threads ativas: %d\n", active_threads);
    pthread_mutex_unlock(&thread_count_mutex);

    return NULL;
}

int main() {
    srand(time(NULL)); // Inicializa a semente aleatória

    int sockfd;
    struct sockaddr_un local, remote;

    // Criação do socket
    sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Falha em criar o socket.");
        return 1;
    }

    memset(&local, 0, sizeof(local));
    local.sun_family = AF_UNIX;
    strncpy(local.sun_path, SOCK_PATH, sizeof(local.sun_path) - 1);
    unlink(SOCK_PATH);  // Remove instância anterior do socket
    int len = strlen(local.sun_path) + sizeof(local.sun_family);

    if (bind(sockfd, (struct sockaddr *)&local, len) < 0) {
        perror("Falha em associar o socket.");
        close(sockfd);
        return 1;
    }

    if (listen(sockfd, 5) < 0) {
        perror("Falha em escutar o socket.");
        close(sockfd);
        return 1;
    }

    printf("Servidor ouvindo em %s\n", SOCK_PATH);

    while (1) {
        int client_sock = accept(sockfd, (struct sockaddr *)&remote, (socklen_t *)&len);
        if (client_sock < 0) {
            perror("Falha em aceitar conexão.");
            continue;
        }

        printf("Cliente conectado!\n");

        int *pclient_sock = malloc(sizeof(int));
        *pclient_sock = client_sock;

        pthread_t client_thread;
        pthread_create(&client_thread, NULL, handle_client, pclient_sock);
        pthread_detach(client_thread); // Permite que a thread se auto-limpe ao concluir
    }

    close(sockfd);
    return 0;
}
