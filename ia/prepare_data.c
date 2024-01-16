#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "prepare_data.h"
char* clean_string(char* string)
{
    //remove all non-alphanumeric characters
    char* res = malloc((1+strlen(string)) * sizeof(char));
    int k = 0;
    for(size_t i = 0; i < strlen(string); i++)
    {
        if (isalpha(string[i]))
        {
            res[k++] = tolower(string[i]);
        }
        if (string[i] == ' ')
        {
            res[k++] = ' ';
        }
    }
    res[k] = '\0';
    return res;
}



WordIndex word_index[MAX_WORDS];
int num_words = 0;

WordIndex* read_word_index(char* filename) 
{
    WordIndex* word_index = malloc(sizeof(struct WordIndex)*MAX_WORDS);
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        printf("Could not open file %s\n", filename);
        return NULL;
    }
    while (fscanf(file, "%s\t%d\n", word_index[num_words].word, &word_index[num_words].index) != EOF) {
        num_words++;
        if (num_words >= MAX_WORDS) {
            break;
        }
    }
    fclose(file);
    return word_index;
}
int find_index(WordIndex* word_index, char* word)
{
    for (int i = 0; i < num_words; i++)
    {
        if (strcmp(word_index[i].word, word) == 0)
        {
            return word_index[i].index;
        }
    }
    return 1;
}

Token* tokenize(WordIndex* word_index, char* string)
{
    char* clean = clean_string(string);
    char* copy = strdup(clean);  // copier la chaîne pour compter les mots
    char* split = strtok(copy, " ");
    int total_words = 0;
    while (split != NULL)
    {
        total_words++;
        split = strtok(NULL, " ");
    }
    free(copy);  // libérer la copie

    split = strtok(clean, " ");
    float* token = calloc(MAX_LEN, sizeof(float));  // initialiser avec des zéros
    int start_index = MAX_LEN - total_words;  // décaler les tokens vers la fin
    if (start_index < 0) {
        start_index = 0;
    }
    int i = start_index;
    while (split != NULL && i < MAX_LEN)
    {
        int index = find_index(word_index, split);
        token[i++] = (float)index;
        split = strtok(NULL, " ");
    }
    free(clean);
    Token* res = malloc(sizeof(struct Token));
    res->token = token;
    // printf("token[0]: %f\n", token[0]);
    res->length = MAX_LEN;
    return res;
}
// int main()
// {
//     read_word_index("/home/dimitri/documents/c/string/word_index.sv");
//     for (int i = 0; i < num_words; i++) {
//         //printf("Word: %s, Index: %d\n", word_index[i].word, word_index[i].index);
//     }
//     int* token = tokenize(word_index, "Hello, world! verify"); 
//     for (int i = 0; i < 3; i++)
//     {
//         printf("%d\n", token[i]);
//     }
//     free(token); 
//     return 0;
// }
