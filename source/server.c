//Servidor pipe (testado usando WSL)
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <ctype.h>
#include <pthread.h>

#define SOCK_PATH "/tmp/pipeso"

pthread_mutex_t em = PTHREAD_MUTEX_INITIALIZER;
int *randomNumber;
char *randomString;

void thread_int(void) {
    pthread_mutex_lock(&em);
    randomNumber = rand() % (1000 - 100 + 1) + 100;
    pthread_mutex_unlock(&em);
    return randomNumber;
}

void thread_string(void) {
    pthread_mutex_lock(&em);
    randomString = malloc(15 + 1);
    if (randomString) {
        rand_string(randomString, 15);
    }
    pthread_mutex_unlock(&em);
    return randomString;
}

int main()
{
    //// NECESSÁRIO ADICIONAR MAIS UMA PIPE, de acordo com o desenho do professor é necessário duas pipes uma pra int e uma pra string

    int sockfd, newsockfd, len;
    struct sockaddr_un local, remote;
    char buffer[1024];
    char dataRequested;
    pthread_t t_string1, t_string2, t_string3, t_int1, t_int2, t_int3;

    // Create socket
    sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("Falha em criar o pipe");
        return 1;
    }

    // Bind socket to local address
    memset(&local, 0, sizeof(local));
    local.sun_family = AF_UNIX;
    strncpy(local.sun_path, SOCK_PATH, sizeof(local.sun_path) - 1);
    unlink(local.sun_path);
    len = strlen(local.sun_path) + sizeof(local.sun_family);

    if (bind(sockfd, (struct sockaddr *)&local, len) < 0)
    {
        perror("Falha em capturar o socket.");
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
        //// AQUI ao invés de processar o dado passando de volta para o buffer, abrir as threads e direcionar ao tipo correto
        dataRequested = toupper(buffer[0]);

        switch (dataRequested) //// verificar se abrimos sempre as 3 threads ou ver um outro mecanismo
        {
            case 'I':
                pthread_create(&t_int1, NULL, (void *) thread_int, NULL);
                pthread_create(&t_int2, NULL, (void *) thread_int, NULL);
                pthread_create(&t_int3, NULL, (void *) thread_int, NULL);
                break;
            case 'S':
                pthread_create(&t_string1, NULL, (void *) thread_string, NULL);
                pthread_create(&t_string2, NULL, (void *) thread_string, NULL);
                pthread_create(&t_string3, NULL, (void *) thread_string, NULL);
                break;
            default:
                perror("Dado solicitado é inválido.");
                close(newsockfd);
                close(sockfd);
                return 1;
        }

        for (int i = 0; i < 20; i++)
        {
            //// definir buffer com o que o que veio da thread
            //// é possível que seja necessário transformar randomInt e randomString em char[] pra fazer esse processamento
            buffer[i] = toupper(buffer[i]);
        }

        // Write processed data back to client
        if (write(newsockfd, buffer, strlen(buffer) + 1) < 0)
        {
            perror("Falha em escrever no socket.");
            close(newsockfd);
            close(sockfd);
            return 1;
        }

        printf("Dado enviado ao cliente.\n");
    }

    return 0;
}