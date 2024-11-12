#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#define INT_SOCK_PATH "/tmp/pipe_int"
#define STRING_SOCK_PATH "/tmp/pipe_string"
#define NUM_THREADS 256 // Número de threads no pool

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int randomNumber;
char randomString[20] = "Morte aos ponteiros!";
char buffer[1024];

void* handle_int_requests(void* arg) {
    while (1) {
        pthread_mutex_lock(&mutex);
        randomNumber = rand() % (1000 - 100 + 1) + 100;
        pthread_mutex_unlock(&mutex);
        
        // Simular um tempo de processamento
        sleep(1);
    }
    return NULL;
}

void* handle_string_requests(void* arg) {
    while (1) {
        pthread_mutex_lock(&mutex);
       for (int i = 0; i < 20; i++)
            randomString[i] = 'a' + (char)(rand() % 15);
        pthread_mutex_unlock(&mutex);
        
        // Simular um tempo de processamento
        sleep(1);
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

        // Read request from client
        memset(buffer, 0, sizeof(buffer));
        if (read(newsockfd, buffer, sizeof(buffer)) < 0) {
            perror("Falha em ler do socket.");
            close(newsockfd);
            continue;
        }

        printf("Dado solicitado: %s\n", buffer);

        // Write response to client
        pthread_mutex_lock(&mutex);
        if (buffer[0] == 'I' || buffer[0] == 'i') {
            // Process integer request
            sprintf(buffer, "%d", randomNumber);
        } else if (buffer[0] == 'S' || buffer[0] == 's') {
            // Process string request
            strncpy(buffer, randomString, sizeof(buffer) - 1);
            buffer[sizeof(buffer) - 1] = '\0'; // Garantir que o buffer é null-terminated
        } else {
            // Invalid request
            strcpy(buffer, "Invalid request");
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

int main() {
    int int_sockfd, string_sockfd;
    pthread_t threads[NUM_THREADS];
    int i;

    // Create sockets
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

    // Create thread pool
    for (i = 0; i < NUM_THREADS / 2; i++) {
        pthread_create(&threads[i], NULL, handle_int_requests, NULL);
    }
    for (i = NUM_THREADS / 2; i < NUM_THREADS; i++) {
        pthread_create(&threads[i], NULL, handle_string_requests, NULL);
    }

    pthread_t server_thread_int, server_thread_string;
    pthread_create(&server_thread_int, NULL, server_thread, &int_sockfd);
    pthread_create(&server_thread_string, NULL, server_thread, &string_sockfd);

    pthread_join(server_thread_int, NULL);
    pthread_join(server_thread_string, NULL);

    close(int_sockfd);
    close(string_sockfd);

    return 0;
}

