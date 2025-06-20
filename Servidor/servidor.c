#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include "Algoritmos/Segmentation_Algorithm.c"
#include "Algoritmos/Preprocessing_Algorithm.c"
#include "Algoritmos/Count_Algorithm.c"
#include <sys/stat.h>
#include <sys/types.h>



#define PORT 8080
#define BUFFER_SIZE 1024
#define XOR_KEY 0xAA // Clave para el cifrado XOR
#define MAX_WORDS_GLOBAL 30000

/*
    * Estructura para almacenar palabras y sus conteos globales
    * Se usa un tamaño máximo de palabra y un número máximo de palabras globales
*/

typedef struct {
    char word[MAX_WORD_LENGTH];
    int count;
} WordGlobal;

/* 
    * Función para buscar una palabra en el arreglo de palabras globales
    * Retorna el índice de la palabra si se encuentra, o -1 si no se encuentra
*/

int find_word_global(WordGlobal words[], int total, const char *target) {
    for (int i = 0; i < total; i++) {
        if (strcmp(words[i].word, target) == 0) {
            return i;
        }
    }
    return -1;
}

/*
    * Función para combinar los conteos de palabras de tres archivos
    * Recibe un arreglo de nombres de archivos y combina los conteos de palabras
    * Encuentra la palabra más repetida entre los archivos y la imprime
    * Esta función asume que los archivos están en el formato "palabra: conteo"
*/

void combine_counts(const char* count_files[3]) {
    WordGlobal global_words[MAX_WORDS_GLOBAL];
    int total_global = 0;
    char word[MAX_WORD_LENGTH];
    int count;

    for (int i = 0; i < 3; i++) {
        FILE *f = fopen(count_files[i], "r");
        if (!f) continue;
        while (fscanf(f, "%99[^:]: %d\n", word, &count) == 2) {
            int idx = find_word_global(global_words, total_global, word);
            if (idx >= 0) {
                global_words[idx].count += count;
            } else {
                strcpy(global_words[total_global].word, word);
                global_words[total_global].count = count;
                total_global++;
            }
        }
        fclose(f);
    }

    // Encontrar la palabra más repetida
    int max_idx = 0;
    for (int i = 1; i < total_global; i++) {
        if (global_words[i].count > global_words[max_idx].count) {
            max_idx = i;
        }
    }
    printf("La palabra más repetida es '%s' con %d apariciones.\n",
           global_words[max_idx].word, global_words[max_idx].count);
}

// Función para encriptar/desencriptar usando XOR
void xor_crypt(char *buffer, int length, char key) {
    for (int i = 0; i < length; i++) {
        buffer[i] ^= key;
    }
}

// Función que maneja la conexión de cada cliente
void *handle_client(void *arg) {

    // Crear directorios para almacenar los archivos procesados
    mkdir("Segmentados", 0777);
    mkdir("Preprocesados", 0777);
    mkdir("Conteos", 0777);

    // Obtener el socket del cliente desde el argumento
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

    // Llamar al algoritmo de segmentación
    Segmentation_Algorithm("archivo_recibido.txt");

    // Rutas de los archivos de entrada y salida
    const char* node_files[3] = {
        "Segmentados/node1.txt",
        "Segmentados/node2.txt",
        "Segmentados/node3.txt"
    };
    
    // Rutas de los archivos preprocesados
    const char* proc_files[3] = {
        "Preprocesados/node1_proc.txt",
        "Preprocesados/node2_proc.txt",
        "Preprocesados/node3_proc.txt"
    };

    // Rutas de los archivos de conteo
    const char* count_files[3] = {
        "Conteos/count_node1.txt",
        "Conteos/count_node2.txt",
        "Conteos/count_node3.txt"
    };

    // Preprocesar los archivos y contar palabras
    for (int i = 0; i < 3; i++) {
        preprocess_file(node_files[i], proc_files[i]);
        count_words(proc_files[i], count_files[i]);
    }
    
    // Combinar los conteos de palabras de los tres archivos
    combine_counts(count_files);
    
    return NULL;
}

// Función principal del servidor
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