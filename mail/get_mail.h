#ifndef GET_MAIL_H
#define GET_MAIL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <curl/curl.h>
#include "access_token.h"
#include "id_parser.h"
#include "body_parser.h"
#include <err.h>
#include <json-c/json.h>

struct Tuple
{
    char* key;
    char* value;
}; typedef struct Tuple Tuple;

Tuple *get_info(char *json_sting) ;

char *get_snippet(char* value, struct json_object *obj) ;

char *get_data(char *image) ;

#endif