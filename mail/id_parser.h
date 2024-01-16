#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include "../ia/model_tensorflow.h"
#include "../ia/model.h"
#include "../ia/black_list/client.h"
#include "image.h"

// This function is a basic implementation of the split function.
char** split(char* input, char* delimiter);

// This function will give us the differents mail IDs from the access token.
char** get_id(const char* token);

// This function will add the mail IDs to th ids.JSON file.
void add_ids(char** ids, char* path, char* dir_path);

// This function will get the contents of the emails.
void new_email(char** ids, char* path, char** contents, char *server_ip, char *server_port, model_spam* model, const char* token);

