#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void Segmentation_Algorithm() {
    const char* input_filename = "output_text.txt";
    const char* output_names[] = {"node1_txt", "node2_txt", "node3_txt"};
    
    FILE *input_file = fopen(input_filename, "r");
    if (!input_file) {
        perror("Error opening input file");
        exit(EXIT_FAILURE);
    }

    // Contar el número total de líneas
    int total_lines = 0;
    int ch;
    while ((ch = fgetc(input_file)) != EOF) {
        if (ch == '\n') total_lines++;
    }
    // Sumar la última línea si no termina con \n
    if (ch == EOF && fseek(input_file, -1, SEEK_END)) {
        if (fgetc(input_file) != '\n') total_lines++;
    }
    rewind(input_file);

    // Calcular líneas por parte
    int lines_per_part = total_lines / 3;
    int remainder = total_lines % 3;

    // Abrir archivos de salida
    FILE *output_files[3];
    for (int i = 0; i < 3; i++) {
        output_files[i] = fopen(output_names[i], "w");
        if (!output_files[i]) {
            perror("Error creating output file");
            // Cerrar archivos ya abiertos
            for (int j = 0; j < i; j++) {
                fclose(output_files[j]);
            }
            fclose(input_file);
            exit(EXIT_FAILURE);
        }
    }

    // Variables para el procesamiento
    char buffer[1024];
    int current_line = 0;
    int part_lines[3] = {
        lines_per_part + (remainder > 0 ? 1 : 0),
        lines_per_part + (remainder > 1 ? 1 : 0),
        lines_per_part
    };

    // Leer y escribir líneas
    int current_part = 0;
    int lines_in_current_part = 0;
    
    while (fgets(buffer, sizeof(buffer), input_file) != NULL) {
        fputs(buffer, output_files[current_part]);
        lines_in_current_part++;
        
        // Cambiar al siguiente archivo cuando se complete la parte actual
        if (lines_in_current_part >= part_lines[current_part] && current_part < 2) {
            current_part++;
            lines_in_current_part = 0;
        }
    }

    // Cerrar archivos
    fclose(input_file);
    for (int i = 0; i < 3; i++) {
        fclose(output_files[i]);
    }

    printf("File successfully split:\n");
    printf("- %s (%d lines)\n", output_names[0], part_lines[0]);
    printf("- %s (%d lines)\n", output_names[1], part_lines[1]);
    printf("- %s (%d lines)\n", output_names[2], part_lines[2]);
}

int main() {
    Segmentation_Algorithm();
    return 0;
}