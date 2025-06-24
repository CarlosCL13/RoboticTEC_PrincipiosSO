#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "../Algoritmos/Preprocessing_Algorithm.c"
#include "../Algoritmos/Count_Algorithm.c"
#include <sys/stat.h>
#include <sys/types.h>

#define PORT 9103
#define BUFFER_SIZE 1024

int main() {
    // Crear carpetas para organizar archivos
    mkdir("Fragmentos", 0777);
    mkdir("Preprocesados", 0777);
    mkdir("Conteos", 0777);

    int server_sock, client_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_size = sizeof(client_addr);

    // Crear socket y asociarlo al puerto del nodo
    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr));
    listen(server_sock, 5);

    printf("Nodo 3 esperando fragmento en el puerto %d...\n", PORT);

    // Bucle principal: el nodo queda siempre disponible para nuevas tareas
    while (1) {
        // Esperar conexión del servidor principal
        client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &addr_size);

        // === 1. Recibir fragmento y guardarlo ===
        FILE *fp = fopen("Fragmentos/fragmento3.txt", "wb");
        char buffer[BUFFER_SIZE];
        int bytes;
        while ((bytes = recv(client_sock, buffer, BUFFER_SIZE, 0)) > 0) {
            fwrite(buffer, 1, bytes, fp);
        }
        fclose(fp);

        // === 2. Preprocesar y contar palabras ===
        preprocess_file("Fragmentos/fragmento3.txt", "Preprocesados/fragmento_proc3.txt");
        count_words("Preprocesados/fragmento_proc3.txt", "Conteos/conteo3.txt");

        // === 3. Enviar archivo de conteo de vuelta al servidor ===
        fp = fopen("Conteos/conteo3.txt", "rb");
        while ((bytes = fread(buffer, 1, BUFFER_SIZE, fp)) > 0) {
            send(client_sock, buffer, bytes, 0);
        }
        fclose(fp);

        // Cerrar conexión y quedar listo para la siguiente tarea
        close(client_sock);
        printf("Nodo 3 terminó procesamiento y espera nueva conexión.\n");
    }

    close(server_sock);
    return 0;
}