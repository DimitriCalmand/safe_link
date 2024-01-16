#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <sys/time.h>
#include "model_tensorflow.h"
#include "./black_list/client.h"
#include "model.h"

int is_spam(char* ip, char* port, char* content, char* sender, model_spam* model)
{
    //Black list algo
    char* list_mail = get_spam(ip, port);
    char* split = strtok(list_mail, "\n");
    int find = 0;
    while (split != NULL)
    {
        if (strcmp(split, sender) == 0)
        {
            find = 1;
            break;
        }
        split = strtok(NULL, "\n");
    }
    free(list_mail);
    if (find)
    {
        return 1;
    }

    TF_Tensor* input_tensor;

    create_tensors_test(&input_tensor, content, model->word_index);
    int num_examples = 1;
    int output_length = 2;
    float* predictions = run_model(model->session, model->input_op, model->output_op, input_tensor, num_examples, output_length);
    float res = 0;
    if (predictions == NULL) 
    {
        return 0;
    }
    else
    {
        if (predictions[0] > 0.5)
        {
            int len_sender = strlen(sender);
            char* string_to_send = malloc(len_sender+2);
            memcpy(string_to_send+1, sender, len_sender);
            string_to_send[0] = '\n';
            string_to_send[len_sender+1] = '\0';
            // send_spam(ip, port, string_to_send);
            free(string_to_send);
            res = 1;
        }
    }
    free(predictions);
    TF_DeleteTensor(input_tensor);
    
    return res;
}


model_spam* init_model(char* path_model, char* path_index_word)
{
    setenv("TF_CPP_MIN_LOG_LEVEL", "2", 1);
    TF_Graph* graph = TF_NewGraph();
    TF_Status* status = TF_NewStatus();
    TF_Session* session = load_model(path_model, graph, status);
    if (session == NULL) 
    {
        errx(1, "Failed to load model");
    }
    TF_Output input_op;
    TF_Output output_op;
    load_input_output_model(graph, &input_op, &output_op);
    WordIndex* word_index = read_word_index(path_index_word);
    model_spam* res = malloc(sizeof(model_spam));
    res->graph = graph;
    res->status = status;
    res->session = session;
    res->output_op = output_op;
    res->input_op = input_op;
    res->word_index = word_index;
    return res;
}
void model_free(model_spam* model)
{
    TF_DeleteGraph(model->graph);
    TF_DeleteSession(model->session, model->status);
    TF_DeleteStatus(model->status);
    free(model->word_index);
    free(model);
}


