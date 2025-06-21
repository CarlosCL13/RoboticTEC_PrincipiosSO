#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "../Servidor/Algoritmos/Preprocessing_Algorithm.c"
#include "../Servidor/Algoritmos/Count_Algorithm.c"

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 9102
#define BUFFER_SIZE 1024

int main() {
    int sock;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    int bytes;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);

    printf("Nodo 2 conectando al servidor en puerto %d...\n", SERVER_PORT);
    connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr));

    // Recibir fragmento y guardarlo
    FILE *fp = fopen("fragmento2.txt", "wb");
    while ((bytes = recv(sock, buffer, BUFFER_SIZE, 0)) > 0) {
        fwrite(buffer, 1, bytes, fp);
    }
    fclose(fp);

    // Procesar
    preprocess_file("fragmento2.txt", "fragmento_proc2.txt");
    count_words("fragmento_proc2.txt", "conteo2.txt");

    // Enviar archivo de conteo de vuelta
    fp = fopen("conteo2.txt", "rb");
    while ((bytes = fread(buffer, 1, BUFFER_SIZE, fp)) > 0) {
        send(sock, buffer, bytes, 0);
    }
    fclose(fp);

    close(sock);
    printf("Nodo 2 terminó procesamiento.\n");
    return 0;
}