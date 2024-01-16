#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static const char base64_table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";

char *decode_base64(char *input)
{
    size_t input_len = strlen(input);
    if (input_len % 4 != 0) {
        printf("Invalid base64 input length.\n");
        return NULL;
    }

    size_t decoded_len = (input_len / 4) * 3;
    if (input[input_len - 1] == '=')
        decoded_len--;
    if (input[input_len - 2] == '=')
        decoded_len--;

    char *output = (char *)malloc(decoded_len + 1);
    if (output == NULL) {
        printf("Failed to allocate memory for decoded output.\n");
        return NULL;
    }

    size_t i, j;
    unsigned int value;
    for (i = 0, j = 0; i < input_len; i += 4, j += 3) {
        value = (strchr(base64_table, input[i]) - base64_table) << 18;
        value |= (strchr(base64_table, input[i + 1]) - base64_table) << 12;
        value |= (strchr(base64_table, input[i + 2]) - base64_table) << 6;
        value |= (strchr(base64_table, input[i + 3]) - base64_table);

        output[j] = (value >> 16) & 0xFF;
        if (input[i + 2] != '=')
            output[j + 1] = (value >> 8) & 0xFF;
        if (input[i + 3] != '=')
            output[j + 2] = value & 0xFF;
    }

    output[decoded_len] = '\0';
    return output;
}

