#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_WORD_LENGTH 100  
#define MAX_WORDS 10000   // Cambiar si el archivo de entrada es muy grande

// Estructura para almacenar una palabra y su cantidad de apariciones
typedef struct {
    char word[MAX_WORD_LENGTH];
    int count;
} Word;

// Busca la palabra en la lista, devuelve su índice, o -1 si no se encuentra 
int find_word(Word words[], int total, const char *target) {
    for (int i = 0; i < total; i++) {
        if (strcmp(words[i].word, target) == 0) {
            return i;
        }
    }
    return -1;
}

// Función de comparación para ordenar:
// - Descendente (mayor a menor) por cantidad
// - Ascendente alfabéticamente si los conteos son iguales
int compare_words(const void *a, const void *b) {
    Word *w1 = (Word *)a;
    Word *w2 = (Word *)b;

    if (w1->count != w2->count)
        return w2->count - w1->count;
    else
        return strcmp(w1->word, w2->word);
}

// Función principal para contar palabras en un archivo de entrada y guardar el resultado en un archivo de salida
void count_words(const char *input_file, const char *output_file) {
    FILE *in = fopen(input_file, "r");
    FILE *out = fopen(output_file, "w");

    if (!in || !out) {
        perror("Error al abrir los archivos");
        return;
    }

    Word words[MAX_WORDS];
    int total_words = 0;
    char current_word[MAX_WORD_LENGTH];

    // Leer palabra por palabra del archivo de entrada
    while (fscanf(in, "%99s", current_word) == 1) {
        int index = find_word(words, total_words, current_word);
        if (index >= 0) {
            words[index].count++;
        } else {
            if (total_words >= MAX_WORDS) {
                fprintf(stderr, "Error: se excedió el límite de palabras (%d). Algunas palabras fueron omitidas.\n", MAX_WORDS);
                break; // Salir del ciclo si se sobrepasa el límite
            }
            strcpy(words[total_words].word, current_word);
            words[total_words].count = 1;
            total_words++;
        }
    }

    // Ordenar las palabras por frecuencia y alfabéticamente
    qsort(words, total_words, sizeof(Word), compare_words);

    // Escribir los resultados en el archivo de salida
    for (int i = 0; i < total_words; i++) {
        fprintf(out, "%s: %d\n", words[i].word, words[i].count);
    }

    fclose(in);
    fclose(out);

    printf("El archivo de salida '%s' fue creado exitosamente con %d palabras.\n", output_file, total_words);
}