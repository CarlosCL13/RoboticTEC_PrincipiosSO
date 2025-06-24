#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <wchar.h>
#include <locale.h>
#include <wctype.h>

// Función para eliminar acentos de caracteres UTF-8
wint_t remove_accent(wint_t c) {
    switch (c) {
        case L'á': case L'Á': return L'a';
        case L'é': case L'É': return L'e';
        case L'í': case L'Í': return L'i';
        case L'ó': case L'Ó': return L'o';
        case L'ú': case L'Ú': return L'u';
        case L'ü': case L'Ü': return L'u';
        // La ñ se mantiene, pero se convierte a minúscula con towlower
        case L'ñ': case L'Ñ': return L'ñ'; 
        default: return c;
    }
}

// Función principal de preprocesamiento: elimina acentos, convierte a minúsculas y filtra caracteres
void preprocess_file(const char* input_file, const char* output_file) {
    // Configura la localización para manejar caracteres anchos (UTF-8)
    setlocale(LC_ALL, "en_US.UTF-8");
    
    FILE *in = fopen(input_file, "r");
    FILE *out = fopen(output_file, "w");

    if (!in || !out) {
        perror("Error al abrir los archivos");
        return;
    }

    wint_t c;
    // Leer carácter por carácter del archivo de entrada
    while ((c = fgetwc(in)) != WEOF) {
        // Convertir a minúscula (incluyendo Ñ → ñ)  
        c = towlower(c);
        c = remove_accent(c);
        
        // Escribir solo letras (a-z, ñ), espacios y saltos de línea
        if ((c >= L'a' && c <= L'z') || c == L'ñ' || c == L' ' || c == L'\n') {
            fputwc(c, out);
        }
    }

    fclose(in);
    fclose(out);
    printf("Texto procesado guardado en %s\n", output_file);
}