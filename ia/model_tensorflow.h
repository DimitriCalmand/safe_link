#ifndef MODEL_TENSORFLOW_H
#define MODEL_TENSORFLOW_H
#include <stdio.h>
#include <tensorflow/c/c_api.h>
#include <err.h>
#include "prepare_data.h"
void load_input_output_model(TF_Graph* graph, TF_Output* input_op, TF_Output* output_op);
TF_Session* load_model(const char* model_dir, TF_Graph* graph, TF_Status* status);
float* run_model(TF_Session* session, TF_Output input_op, TF_Output output_op, TF_Tensor* input_tensor, int num_examples, int output_length);
float calculate_accuracy(float* predictions, float* y_test, int num_examples);
float* open_dataset(char* path, int* shape);
void create_tensors_test( TF_Tensor** input_tensor, char* string, WordIndex* word_index);
#endif