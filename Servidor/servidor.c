#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define XOR_KEY 0xAA // Clave para el cifrado XOR

// Función para encriptar/desencriptar usando XOR
void xor_crypt(char *buffer, int length, char key) {
    for (int i = 0; i < length; i++) {
        buffer[i] ^= key;
    }
}

// Función que maneja la conexión de cada cliente
void *handle_client(void *arg) {
    int client_sock = *(int *)arg;
    free(arg);

    // Abrir archivo para guardar el contenido recibido y desencriptado
    FILE *fp = fopen("archivo_recibido.txt", "wb");
    if (!fp) {
        perror("No se pudo abrir el archivo desencriptado");
        close(client_sock);
        return NULL;
    }

    // Abrir archivo para guardar el contenido cifrado recibido
    FILE *fp_enc = fopen("archivo_recibido_encriptado.txt", "wb");
    if (!fp_enc) {
        perror("No se pudo abrir el archivo encriptado");
        fclose(fp);
        close(client_sock);
        return NULL;
    }

    char buffer[BUFFER_SIZE];
    int bytes;
    // Recibir datos del cliente, guardar cifrado y desencriptar para guardar
    while ((bytes = recv(client_sock, buffer, BUFFER_SIZE, 0)) > 0) {
        fwrite(buffer, 1, bytes, fp_enc);      // Guardar bloque cifrado
        xor_crypt(buffer, bytes, XOR_KEY);     // Desencriptar el bloque recibido
        fwrite(buffer, 1, bytes, fp);          // Guardar bloque desencriptado
    }

    fclose(fp);
    fclose(fp_enc);
    close(client_sock);
    printf("Archivo recibido, guardado cifrado y desencriptado. Conexión cerrada.\n");
    return NULL;
}

int main() {
    int server_sock, *client_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_size = sizeof(client_addr);

    // Crear socket del servidor
    server_sock = socket(AF_INET, SOCK_STREAM, 0);

    // Configurar la dirección del servidor
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Asociar el socket a la dirección y puerto
    bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr));
    // Escuchar conexiones entrantes
    listen(server_sock, 5);

    printf("Servidor escuchando en el puerto %d...\n", PORT);

    // Bucle principal para aceptar conexiones de clientes
    while (1) {
        client_sock = malloc(sizeof(int));
        *client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &addr_size);
        pthread_t tid;
        // Crear un hilo para manejar cada cliente
        pthread_create(&tid, NULL, handle_client, client_sock);
        pthread_detach(tid); // Liberar recursos del hilo automáticamente al terminar
    }

    close(server_sock);
    return 0;
}