#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>

#define INT_SOCK_PATH "/tmp/pipe_int"
#define STRING_SOCK_PATH "/tmp/pipe_string"

void connect_and_request(const char* path, char request_type) {
    int sockfd;
    struct sockaddr_un remote;
    char buffer[1024];

    sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Falha em criar o socket.");
        return;
    }

    memset(&remote, 0, sizeof(remote));
    remote.sun_family = AF_UNIX;
    strncpy(remote.sun_path, path, sizeof(remote.sun_path) - 1);

    if (connect(sockfd, (struct sockaddr*)&remote, sizeof(remote)) < 0) {
        perror("Falha em conectar no servidor.");
        close(sockfd);
        return;
    }

    printf("Conectado ao servidor!\n");

    buffer[0] = request_type;
    buffer[1] = '\0';

    if (write(sockfd, buffer, strlen(buffer) + 1) < 0) {
        perror("Falha em escrever no socket.");
        close(sockfd);
        return;
    }

    printf("Dado solicitado ao servidor.\n");

    memset(buffer, 0, sizeof(buffer));
    if (read(sockfd, buffer, sizeof(buffer)) < 0) {
        perror("Falha em ler do socket.");
        close(sockfd);
        return;
    }

    printf("Dado recebido: %s\n", buffer);
    close(sockfd);
}

int main() {
    // Array de tipos de dados para enviar automaticamente
    char request_types[] = {'S', 'I', 'S', 'I', 'S'}; // Exemplo de sequência de solicitações
    int num_requests = sizeof(request_types) / sizeof(request_types[0]);

    printf("Iniciando solicitações automáticas...\n");

    // Loop para enviar solicitações automaticamente
    for (int i = 0; i < num_requests; ++i) {
        if (tolower(request_types[i]) == 's') {
            connect_and_request(STRING_SOCK_PATH, 'S');
        } else if (tolower(request_types[i]) == 'i') {
            connect_and_request(INT_SOCK_PATH, 'I');
        } else {
            printf("Tipo de dado inválido.\n");
        }
    }

    printf("Todas as solicitações foram feitas.\n");
    return 0;
}
