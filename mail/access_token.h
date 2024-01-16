#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

// This is the callback function that libcurl will call when it has data for us.
size_t write_data(void *ptr, size_t size, size_t nmemb, void *userdata);

// This function will get the authorization code from the user.
char* authorization_code();

// This function will get the access token from the authorization code.
char* access_token(const char* code, char* client_id, char* client_secret);
