#ifndef MODEL_H
#define MODEL_H
#include "model_tensorflow.h"
struct model_spam
{
    TF_Session* session;
    TF_Output input_op;
    TF_Output output_op;
    TF_Status* status;
    TF_Graph* graph;
    WordIndex* word_index;
};
typedef struct model_spam model_spam;
model_spam* init_model(char* path_model, char* path_index_word);
int is_spam(char* ip, char* port, char* content, char* sender, model_spam* model);
void model_free(model_spam* model);

#endif