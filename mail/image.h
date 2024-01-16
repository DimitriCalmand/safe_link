#pragma once

#include <stdlib.h>
#include <string.h>

void base64url_decode(const char* input, unsigned char** output, size_t* output_length);
