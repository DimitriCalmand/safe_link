#ifndef PREPARE_DATA_H
#define PREPARE_DATA_H
struct Token
{
    float* token;
    int length;
};
typedef struct Token Token;
#define MAX_WORDS 5000
#define MAX_WORD_LEN 100
#define MAX_LEN 3000

struct WordIndex 
{
    char word[MAX_WORD_LEN];
    int index;
};
typedef struct WordIndex WordIndex;
Token* tokenize(WordIndex* word_index, char* string);
WordIndex* read_word_index(char* filename) ;
#endif