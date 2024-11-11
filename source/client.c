#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <pthread.h>

#define SOCK_PATH "/tmp/pipeso"
#define NUM_REQUESTS 500 // Número de interações a serem enviadas
#define REQUEST_TYPE 'S' // Tipo de dado: 'S' para string, 'I' para inteiro

// Função que realiza uma conexão e envia uma requisição para o servidor
void *client_interaction(void *arg) {
    int sockfd;
    struct sockaddr_un remote;
    char buffer[1024];

    // Criação do socket
    sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Falha em criar o socket.");
        return NULL;
    }

    // Conexão ao servidor
    memset(&remote, 0, sizeof(remote));
    remote.sun_family = AF_UNIX;
    strncpy(remote.sun_path, SOCK_PATH, sizeof(remote.sun_path) - 1);
    int len = strlen(remote.sun_path) + sizeof(remote.sun_family);

    if (connect(sockfd, (struct sockaddr *)&remote, len) < 0) {
        perror("Falha em conectar no servidor.");
        close(sockfd);
        return NULL;
    }

    // Envio da requisição
    snprintf(buffer, sizeof(buffer), "%c", REQUEST_TYPE);
    if (write(sockfd, buffer, strlen(buffer) + 1) < 0) {
        perror("Falha em escrever no socket.");
        close(sockfd);
        return NULL;
    }

    // Leitura da resposta do servidor
    if (read(sockfd, buffer, sizeof(buffer)) < 0) {
        perror("Falha em ler do socket.");
        close(sockfd);
        return NULL;
    }

    printf("Dado recebido do servidor: %s\n", buffer);
    close(sockfd);
    return NULL;
}

int main() {
    pthread_t threads[NUM_REQUESTS];
    
    // Cria múltiplas threads para realizar várias requisições ao servidor
    for (int i = 0; i < NUM_REQUESTS; i++) {
        if (pthread_create(&threads[i], NULL, client_interaction, NULL) != 0) {
            perror("Falha ao criar thread de cliente.");
            return 1;
        }
    }

    // Aguarda todas as threads terminarem
    for (int i = 0; i < NUM_REQUESTS; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("Todas as requisições foram concluídas.\n");
    return 0;
}
