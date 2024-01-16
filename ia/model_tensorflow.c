#include "model_tensorflow.h"

float* open_dataset(char* path, int* shape) 
{
    FILE *fp;
    fp = fopen(path, "r");
    if (fp == NULL) 
    {
        errx(1, "Erreur lors de l'ouverture du fichier\n");
        return 0;
    }
    else 
    {
        int* data = malloc(sizeof(int)*shape[0]*shape[1]);
        int i, j;

        float* res = calloc(shape[0]*shape[1], sizeof(float));
        for (i = 0; i < shape[0]; i++) 
        {
            for (j = 0; j < shape[1]; j++) 
            {
                if (fscanf(fp, "%d", data+i*shape[1]+j) != 1) 
                {
                    printf("Erreur lors de la lecture des données\n");
                    exit(1);
                }
                res[i*shape[1]+j] = (float)data[i*shape[1]+j];         
            }
        }
        free(data);
        fclose(fp);
        return res;
    }    
}

void free_tensor_data(void* data, size_t length __attribute__((unused)), void* arg __attribute__((unused))) 
{
  free(data);
}
void fill_input_tensor(float* data, int64_t* shape, int num_dims, TF_Tensor** tensor) {
  int64_t num_elements = 1;
  for (int i = 0; i < num_dims; ++i) 
  {
    num_elements *= shape[i];
  }


  size_t data_size = num_elements * sizeof(float);
  float* tensor_data = (float*)malloc(data_size);
  memcpy(tensor_data, data, data_size);
  *tensor = TF_NewTensor(TF_FLOAT, shape, num_dims, tensor_data, data_size, &free_tensor_data, NULL);
}

TF_Session* load_model(const char* model_dir, TF_Graph* graph, TF_Status* status) 
{
    // Charger le modèle TensorFlow SavedModel
    TF_SessionOptions* session_options = TF_NewSessionOptions();
    TF_Buffer* run_options = TF_NewBufferFromString("", 0);
    const char* tags[] = {"serve"};
    TF_Session* session = TF_LoadSessionFromSavedModel(session_options, run_options, model_dir, tags, 1, graph, NULL, status);

    if (TF_GetCode(status) != TF_OK) 
    {
        printf("Error loading SavedModel: %s\n", TF_Message(status));
        TF_DeleteSession(session, status);
        TF_DeleteSessionOptions(session_options);
        TF_DeleteStatus(status);
        TF_DeleteGraph(graph);

        return NULL;
    }
    //free
    TF_DeleteSessionOptions(session_options);
    TF_DeleteBuffer(run_options);
    return session;
}
void load_input_output_model(TF_Graph* graph, TF_Output* input_op, TF_Output* output_op)
{
    *input_op = (TF_Output){TF_GraphOperationByName(graph, "serving_default_inputs"), 0};
    *output_op = (TF_Output){TF_GraphOperationByName(graph, "StatefulPartitionedCall"), 0};
}
void create_tensors_test(TF_Tensor** input_tensor, char* string, WordIndex* word_index) {
    // Créer les tenseurs d'entrée et de sortie pour le modèle
    // Remplacez les dimensions et les types appropriés pour votre modèle spécifique

    // int64_t input_dims[] = {399, 512};
    // float* val_input = open_dataset("/home/dimitri/documents/dataset/spam/Xtest.csv", (int[]){399, 512});
    Token* token = tokenize(word_index, string);
    fill_input_tensor(token->token, (int64_t[]){1, token->length}, 2, input_tensor); 
    free(token->token);     
    free(token);  
}

float* run_model(TF_Session* session, TF_Output input_op, TF_Output output_op, TF_Tensor* input_tensor, int num_examples, int output_length)
{
    // Exécuter le modèle
    TF_Status* status = TF_NewStatus();
    TF_Tensor* output_tensor = NULL;
    TF_SessionRun(session, NULL,
                  &input_op, &input_tensor, 1,
                  &output_op, &output_tensor, 1,
                  NULL, 0, NULL, status);

    if (TF_GetCode(status) != TF_OK) 
    {
        printf("Error running session: %s\n", TF_Message(status));
        TF_DeleteStatus(status);
        return NULL;
    }

    float* output_data = (float*)TF_TensorData(output_tensor);
    float* result = calloc(num_examples, sizeof(float));

    for (int i = 0; i < num_examples; ++i) 
    {
        result[i] = 1.;
        if (output_data[i * output_length] > output_data[i * output_length + 1]) {
            result[i] = 0.;
        }
        result[i] = output_data[i * output_length];
    }
    TF_DeleteStatus(status);
    TF_DeleteTensor(output_tensor);
    return result;
}


float calculate_accuracy(float* predictions, float* y_test, int num_examples) 
{
    float acc = 0;
    for (int i = 0; i < num_examples; ++i) 
    {
        if (predictions[i] == y_test[i]) 
        {
            acc += 1;
        }
    }

    return acc / num_examples;
}
