#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <ctype.h>

#define INT_SOCK_PATH "/tmp/pipe_int"
#define STRING_SOCK_PATH "/tmp/pipe_string"

void connect_and_request(const char* path, char request_type) {
    int sockfd;
    struct sockaddr_un remote;
    char buffer[1024];

    // Create socket
    sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Falha em criar o socket.");
        return;
    }

    // Connect to server
    memset(&remote, 0, sizeof(remote));
    remote.sun_family = AF_UNIX;
    strncpy(remote.sun_path, path, sizeof(remote.sun_path) - 1);

    if (connect(sockfd, (struct sockaddr*)&remote, sizeof(remote)) < 0) {
        perror("Falha em conectar no servidor.");
        close(sockfd);
        return;
    }

    printf("Conectado ao servidor!\n");

    // Send data to server
    buffer[0] = request_type;
    buffer[1] = '\0'; // Null-terminate the string

    if (write(sockfd, buffer, strlen(buffer) + 1) < 0) {
        perror("Falha em escrever no socket.");
        close(sockfd);
        return;
    }

    printf("Dado solicitado ao servidor.\n");

    // Read data from server
    memset(buffer, 0, sizeof(buffer));
    if (read(sockfd, buffer, sizeof(buffer)) < 0) {
        perror("Falha em ler do socket.");
        close(sockfd);
        return;
    }

    printf("Dado recebido: %s\n", buffer);

    // Close socket and exit
    close(sockfd);
}

int main() {
    char type;
    printf("Opção e : exit \n");
    while (type != 'e')
    {
    printf("Digite o tipo de dado a ser solicitado: (s/i)\n");
    scanf(" %c", &type);
    getchar(); // Clear newline from input buffer

    if (tolower(type) == 's') {
        connect_and_request(STRING_SOCK_PATH, 'S');
    } else if (tolower(type) == 'i') {
        connect_and_request(INT_SOCK_PATH, 'I');
    } else {
        printf("Tipo de dado inválido.\n");
    }
    }
    return 0;
}
