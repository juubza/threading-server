//Servidor pipe (testado usando WSL)
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <ctype.h>
#include <pthread.h>
#include <assert.h>
#include <math.h>

#define INT_SOCK_PATH "/tmp/pipe_int"
#define STRING_SOCK_PATH "/tmp/pipe_string"

pthread_mutex_t em = PTHREAD_MUTEX_INITIALIZER;
int randomNumber;
char randomString[20] = "Morte aos ponteiros!";
char buffer[1024];

void thread_int(void) {
    pthread_mutex_lock(&em);
    randomNumber = rand() % (1000 - 100 + 1) + 100;
    buffer[0] = randomNumber;
    pthread_mutex_unlock(&em);
}

void thread_string(void) {
    pthread_mutex_lock(&em);
    for (int i = 0; i < strlen(randomString); i++)
    {
        buffer[i] = randomString[i];
    }
    pthread_mutex_unlock(&em);
}

void connect_pipe(char type) {
    int sockfd, newsockfd, len;
    struct sockaddr_un local, remote;
    char dataRequested;
    pthread_t t_string1, t_string2, t_string3, t_int1, t_int2, t_int3;

    sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("Falha em criar o pipe");
        return;
    }

    memset(&local, 0, sizeof(local));
    local.sun_family = AF_UNIX;

    if (toupper(type) == 'S') {
        strncpy(local.sun_path, STRING_SOCK_PATH, sizeof(local.sun_path) - 1);
    }
    else {
        strncpy(local.sun_path, INT_SOCK_PATH, sizeof(local.sun_path) - 1);
    }

    unlink(local.sun_path);
    len = strlen(local.sun_path) + sizeof(local.sun_family);

    if (bind(sockfd, (struct sockaddr *)&local, len) < 0)
    {
        perror("Falha em capturar o socket.");
        close(sockfd);
        return;
    }

    while (1) {
        // Listen for connections
        if (listen(sockfd, 5) < 0)
        {
            perror("Falha em escutar o socket.");
            close(sockfd);
            return;
        }

        if (toupper(type) == 'S') {
            printf("Servidor ouvindo em %s.\n", STRING_SOCK_PATH);
        }
        else {
            printf("Servidor ouvindo em %s.\n", INT_SOCK_PATH);
        }

        // Accept connections
        memset(&remote, 0, sizeof(remote));
        len = sizeof(remote);
        newsockfd = accept(sockfd, (struct sockaddr *)&remote, &len);
        if (newsockfd < 0)
        {
            perror("Falha em aceitar conexão.");
            close(sockfd);
            return;
        }

        printf("Cliente conectado!\n");

        // Read data from client
        if (read(newsockfd, buffer, sizeof(buffer)) < 0)
        {
            perror("Falha em ler do socket.");
            close(newsockfd);
            close(sockfd);
            return;
        }

        printf("Dado solicitado: %s\n", buffer);

        // Process data
        dataRequested = toupper(buffer[0]);

        switch (dataRequested) // TODO verificar se abrimos sempre as 3 threads ou ver um outro mecanismo para permitir que varios clientes se conectem a mesma pipe com threads separadas
        {
            case 'I':
                len = floor(log10(abs(randomNumber))) + 1;
                pthread_create(&t_int1, NULL, (void *) thread_int, NULL);
                pthread_create(&t_int2, NULL, (void *) thread_int, NULL);
                pthread_create(&t_int3, NULL, (void *) thread_int, NULL);
                break;
            case 'S':
                len = strlen(randomString);
                pthread_create(&t_string1, NULL, (void *) thread_string, NULL);
                pthread_create(&t_string2, NULL, (void *) thread_string, NULL);
                pthread_create(&t_string3, NULL, (void *) thread_string, NULL);
                break;
            default:
                perror("Dado solicitado é inválido.");
                close(newsockfd);
                close(sockfd);
                return;
        }

        // Write processed data back to client
        if (write(newsockfd, buffer, strlen(buffer) + 1) < 0)
        {
            perror("Falha em escrever no socket.");
            close(newsockfd);
            close(sockfd);
            return;
        }

        printf("Dado enviado ao cliente.\n");
    }
}

int main()
{
    // TODO fazer ambas as conexões rodarem paralelamente, atualmente só a primeira inicia e espera ela terminar para a segunda iniciar
    connect_pipe('S');
    connect_pipe('I');

    return 0;
}