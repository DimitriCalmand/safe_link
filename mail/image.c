#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/bio.h>
#include <openssl/evp.h>

void base64url_decode(const char* input, unsigned char** output, size_t* output_length) {
    // Convertir la chaîne base64url en base64 standard
    char* base64_string = malloc(strlen(input) + 1);
    strcpy(base64_string, input);
    size_t len = strlen(base64_string);

    for (size_t i = 0; i < len; i++) {
        if (base64_string[i] == '-')
            base64_string[i] = '+';
        else if (base64_string[i] == '_')
            base64_string[i] = '/';
    }

    size_t padding_length = 4 - (len % 4);
    for (size_t i = 0; i < padding_length; i++) {
    base64_string = (char*)realloc(base64_string, strlen(base64_string) + 2); // +2 pour "=" et la fin de la chaîne '\0'
    strcat(base64_string, "=");
    }

    // Décoder la chaîne base64
    BIO* bio = BIO_new(BIO_f_base64());
    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
    BIO* mem = BIO_new_mem_buf(base64_string, -1);
    BIO_push(bio, mem);

    *output = (unsigned char*)malloc(len);
    *output_length = BIO_read(bio, *output, len);

    BIO_free_all(bio);
    free(base64_string); // Libérer la mémoire allouée pour base64_string
}