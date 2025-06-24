#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include "../Algoritmos/Segmentation_Algorithm.c"
#include "../Algoritmos/Count_Algorithm.c"
#include <sys/stat.h>
#include <sys/types.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define XOR_KEY 0xAA // Clave para el cifrado XOR
#define MAX_WORDS_GLOBAL 30000

// Estructura para almacenar palabras y sus conteos globales
typedef struct {
    char word[MAX_WORD_LENGTH];
    int count;
} WordGlobal;

// Busca una palabra en el arreglo global, retorna su índice o -1 si no está
int find_word_global(WordGlobal words[], int total, const char *target) {
    for (int i = 0; i < total; i++) {
        if (strcmp(words[i].word, target) == 0) {
            return i;
        }
    }
    return -1;
}

// Combina los conteos de palabras de los tres archivos recibidos de los nodos
// y encuentra la palabra más repetida globalmente
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

// Estructura para pasar argumentos a cada hilo de nodo
typedef struct {
    const char* ip;
    int port;
    const char* fragment_file;
    const char* count_file;
} NodoArgs;

// ===================
// FUNCIÓN CLAVE: Procesamiento paralelo con los nodos
// ===================
// Esta función se ejecuta en un hilo por cada nodo.
// Se conecta al nodo, le envía el fragmento y recibe el conteo de palabras.
void* procesar_nodo(void* arg) {
    NodoArgs* nodo = (NodoArgs*)arg;
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in nodo_addr;
    nodo_addr.sin_family = AF_INET;
    nodo_addr.sin_port = htons(nodo->port);
    inet_pton(AF_INET, nodo->ip, &nodo_addr.sin_addr);

    printf("Conectando al nodo en puerto %d...\n", nodo->port);
    // Reintenta la conexión hasta que el nodo esté disponible
    while (connect(sock, (struct sockaddr *)&nodo_addr, sizeof(nodo_addr)) < 0) {
        printf("Nodo en puerto %d no disponible, reintentando...\n", nodo->port);
        sleep(1);
    }

    // Enviar fragmento al nodo
    FILE *fp = fopen(nodo->fragment_file, "rb");
    char buffer[BUFFER_SIZE];
    int bytes;
    while ((bytes = fread(buffer, 1, BUFFER_SIZE, fp)) > 0) {
        send(sock, buffer, bytes, 0);
    }
    fclose(fp);
    shutdown(sock, SHUT_WR); // Señaliza fin de envío

    // Recibir archivo de conteo del nodo
    fp = fopen(nodo->count_file, "wb");
    while ((bytes = recv(sock, buffer, BUFFER_SIZE, 0)) > 0) {
        fwrite(buffer, 1, bytes, fp);
    }
    fclose(fp);
    close(sock);
    return NULL;
}

// ===================
// FUNCIÓN CLAVE: Manejo de cada cliente principal (paralelismo con hilos)
// ===================
void *handle_client(void *arg) {

    // Crear directorios para almacenar los archivos procesados
    mkdir("Segmentados", 0777);
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

    // 1. Segmentar el archivo desencriptado en tres partes
    Segmentation_Algorithm("archivo_recibido.txt");

    // ===================
    // PARTE VITAL: Procesamiento distribuido y paralelo con los nodos
    // ===================
    // Se crean tres hilos, uno por cada nodo, para enviar fragmentos y recibir conteos en paralelo
    const char* node_ips[3] = {"127.0.0.1", "127.0.0.1", "127.0.0.1"};
    const int node_ports[3] = {9101, 9102, 9103};
    const char* node_files[3] = {
        "Segmentados/node1.txt",
        "Segmentados/node2.txt",
        "Segmentados/node3.txt"
    };
    const char* count_files[3] = {
        "Conteos/count_node1.txt",
        "Conteos/count_node2.txt",
        "Conteos/count_node3.txt"
    };

    pthread_t threads[3];
    NodoArgs nodo_args[3];

    for (int i = 0; i < 3; i++) {
        nodo_args[i].ip = node_ips[i];
        nodo_args[i].port = node_ports[i];
        nodo_args[i].fragment_file = node_files[i];
        nodo_args[i].count_file = count_files[i];
        // Cada hilo maneja la conexión y procesamiento con un nodo
        pthread_create(&threads[i], NULL, procesar_nodo, &nodo_args[i]);
    }
    // Espera a que todos los hilos terminen (sincronización)
    for (int i = 0; i < 3; i++) {
        pthread_join(threads[i], NULL);
    }

    // 3. Combinar los conteos de palabras de los tres archivos
    // Los archivos de conteo están en:
    // "Conteos/count_node1.txt", "Conteos/count_node2.txt", "Conteos/count_node3.txt"
    combine_counts(count_files);

    return NULL;
}

// ===================
// FUNCIÓN PRINCIPAL DEL SERVIDOR
// ===================
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
    // Escuchar conexiones entrantes de clientes principales
    listen(server_sock, 5);

    printf("Servidor escuchando en el puerto %d...\n", PORT);

    // Bucle principal para aceptar conexiones de clientes principales
    while (1) {
        client_sock = malloc(sizeof(int));
        *client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &addr_size);
        pthread_t tid;
        // Cada cliente principal es manejado en un hilo independiente
        pthread_create(&tid, NULL, handle_client, client_sock);
        pthread_detach(tid); // Liberar recursos del hilo automáticamente al terminar
    }

    close(server_sock);
    return 0;
}