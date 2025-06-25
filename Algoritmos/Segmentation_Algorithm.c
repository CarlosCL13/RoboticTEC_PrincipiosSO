#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Función para segmentar el archivo de entrada en tres partes, distribuyendo las palabras equitativamente
void Segmentation_Algorithm(const char* input_filename) {
    // Nombres de los archivos de salida para cada nodo
    const char* output_names[] = {
        "Segmentados/node1.txt",
        "Segmentados/node2.txt",
        "Segmentados/node3.txt"
    };

    // Abrir el archivo de entrada
    FILE *input_file = fopen(input_filename, "r");
    if (!input_file) {
        perror("Error al abrir el archivo de entrada");
        exit(EXIT_FAILURE);
    }

    // Leer todo el archivo en memoria
    fseek(input_file, 0, SEEK_END);
    long filesize = ftell(input_file);
    rewind(input_file);

    char *file_content = malloc(filesize + 1);
    if (!file_content) {
        perror("Error de asignación de memoria");
        fclose(input_file);
        exit(EXIT_FAILURE);
    }
    fread(file_content, 1, filesize, input_file);
    file_content[filesize] = '\0';
    fclose(input_file);

    // Contar el número total de palabras
    int total_words = 0;
    char *copy = strdup(file_content);
    char *token = strtok(copy, " \t\r\n");
    while (token) {
        total_words++;
        token = strtok(NULL, " \t\r\n");
    }
    free(copy);

    // Calcular cuántas palabras van en cada parte
    int words_per_part = total_words / 3;
    int remainder = total_words % 3;
    int part_words[3] = {
        words_per_part + (remainder > 0 ? 1 : 0),
        words_per_part + (remainder > 1 ? 1 : 0),
        words_per_part
    };

    // Abrir los archivos de salida para cada nodo
    FILE *output_files[3];
    for (int i = 0; i < 3; i++) {
        output_files[i] = fopen(output_names[i], "w");
        if (!output_files[i]) {
            perror("Error al crear el archivo de salida");
            for (int j = 0; j < i; j++) fclose(output_files[j]);
            free(file_content);
            exit(EXIT_FAILURE);
        }
    }

    // Escribir palabras en los archivos, manteniendo los saltos de línea originales
    int current_part = 0, words_in_part = 0;
    char *p = file_content;
    while (*p) {
        // Saltar espacios iniciales
        while (*p && isspace(*p) && *p != '\n') p++;

        // Detectar fin de archivo
        if (!*p) break;

        // Encontrar el final de la palabra
        char *start = p;
        while (*p && !isspace(*p)) p++;
        int word_len = p - start;

        // Escribir la palabra en el archivo correspondiente
        fwrite(start, 1, word_len, output_files[current_part]);
        words_in_part++;

        // Escribir el espacio o salto de línea original
        while (*p && isspace(*p)) {
            fputc(*p, output_files[current_part]);
            if (*p == '\n') ; // Mantener saltos de línea
            p++;
            // Si hay varios espacios, los mantiene
        }

        // Cambiar de archivo si se llegó al límite de palabras para esa parte
        if (words_in_part >= part_words[current_part] && current_part < 2) {
            current_part++;
            words_in_part = 0;
        }
    }

    // Cerrar archivos y liberar memoria
    for (int i = 0; i < 3; i++) fclose(output_files[i]);
    free(file_content);

    // Mensajes en español sobre la segmentación
    printf("Archivo segmentado exitosamente por palabras:\n");
    printf("- %s (%d palabras)\n", output_names[0], part_words[0]);
    printf("- %s (%d palabras)\n", output_names[1], part_words[1]);
    printf("- %s (%d palabras)\n", output_names[2], part_words[2]);
}