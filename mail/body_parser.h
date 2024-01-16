#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

int check_id(FILE *file, char *id);

int check_dir(char *dir_path, char *id);

int check_dir2(char *dir_path, char *id);

// This function gets the body of each mail and returns it as a string.
char** get_contents(char** ids, const char* token, char* path, char *dir_path);

char** get_images(char *ids, const char* token, char *attachments_id);