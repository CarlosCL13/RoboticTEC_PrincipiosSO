#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <wchar.h>
#include <locale.h>
#include <wctype.h>

// Function to remove accents from UTF-8 characters
wint_t remove_accent(wint_t c) {
    switch (c) {
        case L'á': case L'Á': return L'a';
        case L'é': case L'É': return L'e';
        case L'í': case L'Í': return L'i';
        case L'ó': case L'Ó': return L'o';
        case L'ú': case L'Ú': return L'u';
        case L'ü': case L'Ü': return L'u';
        // The ñ is retained but converted to lower case by towlower
        case L'ñ': case L'Ñ': return L'ñ'; 
        default: return c;
    }
}

void preprocess_file(const char* input_file, const char* output_file) {
    // Set localization to handle wide characters
    setlocale(LC_ALL, "en_US.UTF-8");
    
    FILE *in = fopen(input_file, "r");
    FILE *out = fopen(output_file, "w");

    if (!in || !out) {
        perror("Error opening file");
        return;
    }

    wint_t c;
    while ((c = fgetwc(in)) != WEOF) {
        // Convert to lowercase (including Ñ → ñ)  
        c = towlower(c);
        c = remove_accent(c);
        
        // Writing letters (a-z, ñ) and spaces
        if ((c >= L'a' && c <= L'z') || c == L'ñ' || c == L' ' || c == L'\n') {
            fputwc(c, out);
        }
    }

    fclose(in);
    fclose(out);
    printf("Processed text stored in %s\n", output_file);
}