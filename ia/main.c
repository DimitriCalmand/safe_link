#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <sys/time.h>
#include "model_tensorflow.h"
#include "./black_list/client.h"
#include "./model.h"


int main(int argc, char *argv[])
{   
    char* content = "Unbelievable offer just for you! Dear friend, you have been selected for an exclusive offer! Upgrade your phone with our latest smartphone model at an unbeatable price! But that's not all, for a limited time only, we are offering you a FREE 55 inch 4K TV if you act now! Don’t wait, stocks are limited! Click here to claim your offer! To unsubscribe from future emails, click here.";
    // content = "Bonjour";
    char* sender = "maxiswdemssw.cdedswswamdboudd@epita.fr";
    model_spam* model = init_model("./ia/model", "./ia/word_index.sv");
    clock_t start_time = clock();
    int result = is_spam(argv[1], argv[2], content, sender, model);
    clock_t end_time = clock();
    double execution_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;
    model_free(model);
    argc++;
    return 0;
}




















// =================== THE SPAM DATASET ======================== 
/*  
    Model *model = Init_Model((int[]){512}, 1);
    model->Add(model, Init_Embedding(8194, 128));
    model->Add(model, Init_Lstm(256, 1));
    model->Add(model, Init_Lstm(128, 0));
    model->Add(model, Init_Dense(32));
    model->Add(model, Init_Sigmoid());
    model->Add(model, Init_Dense(16));
    model->Add(model, Init_Sigmoid());
    model->Compile(model, Init_Bce(), 0.001);
    model->Summary(model);

    Matrix* X_train = open_data("/home/dimitri/documents/dataset/spam/Xtrain.csv", (int[]){2259, 512});
    Matrix* X_test = open_data("/home/dimitri/documents/dataset/spam/Xtest.csv", (int[]){399, 512});
    Matrix* y_train = open_data("/home/dimitri/documents/dataset/spam/ytrain.csv", (int[]){2259, 1});
    Matrix* y_test = open_data("/home/dimitri/documents/dataset/spam/ytest.csv", (int[]){399,1});

    model->Fit(model, X_train, y_train, X_test, y_test, 10 , 300, 1);
    Save(model, "ia/model.h5");
    
    model->Free(model); 

    X_train->Free(X_train);
    X_test->Free(X_test);
    y_train->Free(y_train);
    y_test->Free(y_test);
    return 0;
*/

