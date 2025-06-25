#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
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

int main() {
    int sock;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];

    // === 1. Abrir archivo original para leer ===
    FILE *fp_in = fopen("el_quijote.txt", "rb"); // Cambia el nombre si usas otro archivo
    if (!fp_in) {
        perror("No se pudo abrir el archivo original");
        return 1;
    }

    // === 2. Crear archivo cifrado para escribir el contenido encriptado ===
    FILE *fp_out = fopen("el_quijote_encriptado.txt", "wb");
    if (!fp_out) {
        perror("No se pudo crear el archivo encriptado");
        fclose(fp_in);
        return 1;
    }

    int bytes;
    // === 3. Leer el archivo original, cifrarlo y escribirlo en el archivo encriptado ===
    while ((bytes = fread(buffer, 1, BUFFER_SIZE, fp_in)) > 0) {
        xor_crypt(buffer, bytes, XOR_KEY); // Encriptar el bloque leído
        fwrite(buffer, 1, bytes, fp_out);  // Escribir el bloque encriptado
    }

    fclose(fp_in);
    fclose(fp_out);

    // === 4. Abrir el archivo encriptado para enviarlo al servidor ===
    fp_out = fopen("el_quijote_encriptado.txt", "rb");
    if (!fp_out) {
        perror("No se pudo abrir el archivo encriptado para enviar");
        return 1;
    }

    // === 5. Crear el socket ===
    sock = socket(AF_INET, SOCK_STREAM, 0);

    // === 6. Configurar la dirección del servidor ===
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);

    // === 7. Conectar al servidor ===
    connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr));

    // === 8. Leer el archivo encriptado y enviarlo al servidor por el socket ===
    while ((bytes = fread(buffer, 1, BUFFER_SIZE, fp_out)) > 0) {
        send(sock, buffer, bytes, 0);
    }

    fclose(fp_out);
    close(sock);
    printf("Archivo encriptado y enviado.\n");
    return 0;
}