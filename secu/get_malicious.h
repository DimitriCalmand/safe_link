#ifndef GET_MALICIOUS_H
#define GET_MALICIOUS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <stdbool.h>
#include <ctype.h>
#include <json-c/json.h>
#define RESPONSE_SIZE 4096

struct ResponseBuffer {
    char data[RESPONSE_SIZE];
    size_t pos;
};
struct ResponseData {
    char *data;
    size_t size;
};

struct ResponseData get_mallicious(char* file);

#endif
