// Servidor pipe - testado no WSL
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <ctype.h>
#include <pthread.h>
#include <string.h>

#define SOCK_PATH "/tmp/pipeso"
#define MAX_STR_LEN 16

pthread_mutex_t em = PTHREAD_MUTEX_INITIALIZER;

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

// Função de thread para gerar número inteiro
void *thread_int(void *arg) {
    pthread_mutex_lock(&em);
    int randomNumber = rand() % (1000 - 100 + 1) + 100;
    printf("Número gerado: %d\n", randomNumber);
    pthread_mutex_unlock(&em);
    
    int *result = malloc(sizeof(int));
    *result = randomNumber;
    return result;
}

// Função de thread para gerar string
void *thread_string(void *arg) {
    pthread_mutex_lock(&em);
    char *randomString = malloc(MAX_STR_LEN);
    rand_string(randomString, MAX_STR_LEN - 1);
    printf("String gerada: %s\n", randomString);
    pthread_mutex_unlock(&em);
    
    return randomString;
}

int main() 
{
    int sockfd, newsockfd, len;
    struct sockaddr_un local, remote;
    char buffer[1024];
    char dataRequested;
    pthread_t t_string, t_int;

    // Create socket
    sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("Falha em criar o socket.");
        return 1;
    }

    // Bind socket to local address
    memset(&local, 0, sizeof(local));
    local.sun_family = AF_UNIX;
    strncpy(local.sun_path, SOCK_PATH, sizeof(local.sun_path) - 1);
    unlink(local.sun_path); // Remove qualquer instância anterior
    len = strlen(local.sun_path) + sizeof(local.sun_family);

    if (bind(sockfd, (struct sockaddr *)&local, len) < 0)
    {
        perror("Falha em associar o socket.");
        close(sockfd);
        return 1;
    }

    while (1) {
        // Listen for connections
        if (listen(sockfd, 5) < 0)
        {
            perror("Falha em escutar o socket.");
            close(sockfd);
            return 1;
        }

        printf("Servidor ouvindo em %s.\n", SOCK_PATH);

        // Accept connections
        memset(&remote, 0, sizeof(remote));
        len = sizeof(remote);
        newsockfd = accept(sockfd, (struct sockaddr *)&remote, &len);
        if (newsockfd < 0)
        {
            perror("Falha em aceitar conexão.");
            close(sockfd);
            return 1;
        }

        printf("Cliente conectado!\n");

        // Read data from client
        if (read(newsockfd, buffer, sizeof(buffer)) < 0)
        {
            perror("Falha em ler do socket.");
            close(newsockfd);
            close(sockfd);
            return 1;
        }

        printf("Dado solicitado: %s\n", buffer);

        // Process data
        dataRequested = toupper(buffer[0]);
        switch (dataRequested)
        {
            case 'I': {
                int *result;
                pthread_create(&t_int, NULL, thread_int, NULL);
                pthread_join(t_int, (void **)&result);
                sprintf(buffer, "%d", *result);  // Converte o número para string e coloca no buffer
                free(result);
                break;
            }
            case 'S': {
                char *result;
                pthread_create(&t_string, NULL, thread_string, NULL);
                pthread_join(t_string, (void **)&result);
                strncpy(buffer, result, sizeof(buffer));
                free(result);
                break;
            }
            default:
                strcpy(buffer, "Erro: Tipo de dado inválido.");
                break;
        }

        // Write processed data back to client
        if (write(newsockfd, buffer, strlen(buffer) + 1) < 0)
        {
            perror("Falha em escrever no socket.");
            close(newsockfd);
            close(sockfd);
            return 1;
        }

        printf("Dado enviado ao cliente: %s\n", buffer);
    }

    close(sockfd);
    return 0;
}
