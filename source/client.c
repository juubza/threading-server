//Cliente pipe - testado no WSL
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <ctype.h>

#define INT_SOCK_PATH "/tmp/pipe_int"
#define STRING_SOCK_PATH "/tmp/pipe_string"

void connect_pipe(char type) {
    int sockfd, len;
    struct sockaddr_un remote;
    char buffer[1024];

    // Create socket
    sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("Falha em criar o socket.");
        return;
    }

    // Connect to server
    memset(&remote, 0, sizeof(remote));
    remote.sun_family = AF_UNIX;

    if (toupper(type) == 'S') {
        strncpy(remote.sun_path, STRING_SOCK_PATH, sizeof(remote.sun_path) - 1);
    }
    else {
        strncpy(remote.sun_path, INT_SOCK_PATH, sizeof(remote.sun_path) - 1);
    }

    len = strlen(remote.sun_path) + sizeof(remote.sun_family);
    if (connect(sockfd, (struct sockaddr *)&remote, len) < 0)
    {
        perror("Falha em conectar no servidor.");
        close(sockfd);
        return;
    }

    printf("Conectado ao servidor!\n");

    // Send data to server
    printf("Entre com o tipo dado a ser solicitado ao servidor: (s/i)");
    fgets(buffer, sizeof(buffer), stdin);
    if (write(sockfd, buffer, strlen(buffer) + 1) < 0)
    {
        perror("Falha em escrever no socket.");
        close(sockfd);
        return;
    }

    printf("Dado solicitado ao servidor.\n");

    // Read data from server
    if (read(sockfd, buffer, sizeof(buffer)) < 0)
    {
        perror("Falha em ler do socket.");
        close(sockfd);
        return;
    }

    printf("Dado recebido: %s\n", buffer);

    // Close socket and exit
    close(sockfd);
}

int main()
{ 
    connect_pipe('S');
    connect_pipe('I');

    return 0;
}