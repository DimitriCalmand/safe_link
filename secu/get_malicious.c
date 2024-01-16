#include "get_malicious.h"
#define API_KEY "f291e4234f778dc9cbc6dee3c4ecbc83d582af90e5b97dc7bf8a9846d96d5bf3"



size_t write_callback(char *ptr, size_t size, size_t nmemb, void *userdata)
{
    size_t total_size = size * nmemb;
    struct ResponseBuffer *buffer = (struct ResponseBuffer *)userdata;

    for (size_t i = 0; i < total_size; i++) {
        if ((unsigned char)ptr[i] < 128 || ((unsigned char)ptr[i] & 0xC0) == 0xC0) {
            if (buffer->pos < RESPONSE_SIZE - 1) {
                buffer->data[buffer->pos] = ptr[i];
                buffer->pos++;
            }
        }
    }

    return total_size;
}

size_t write_data_callback(void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t total_size = size * nmemb;
    struct ResponseData *mem = (struct ResponseData *)userp;

    char *tmp = realloc(mem->data, mem->size + total_size + 1);
    if(tmp == NULL) {
        /* out of memory! */ 
        printf("Not enough memory (realloc returned NULL)\n");
        return 0;
    }

    mem->data = tmp;
    memcpy(&(mem->data[mem->size]), contents, total_size);
    mem->size += total_size;
    mem->data[mem->size] = 0; /* null-terminator */

    return total_size;
}



struct ResponseData get_mallicious(char* file)
{
    CURL *curl;
    CURLcode res;
    struct ResponseBuffer response_buffer;
    response_buffer.pos = 0;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    struct ResponseData response_data;
    response_data.data = NULL;
    response_data.size = 0;

    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, "https://www.virustotal.com/api/v3/files");

        struct curl_slist *headers = NULL;
        char auth_header[100];
        snprintf(auth_header, sizeof(auth_header), "x-apikey: %s", API_KEY);
        headers = curl_slist_append(headers, auth_header);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        curl_mime *form = curl_mime_init(curl);
        curl_mimepart *part = curl_mime_addpart(form);
        curl_mime_name(part, "file");
        curl_mime_filedata(part, file);
        curl_easy_setopt(curl, CURLOPT_MIMEPOST, form);

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_buffer);

        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
            perror("erreyr");
        }

        curl_mime_free(form);
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);

        response_buffer.data[response_buffer.pos] = '\0';

        json_object *root = json_tokener_parse(response_buffer.data);
        if (root == NULL) {
            fprintf(stderr, "Failed to parse JSON response\n");
            perror("erreyr");
        }

        json_object *data;
        if (!json_object_object_get_ex(root, "data", &data)) {
            fprintf(stderr, "Invalid JSON response format\n");
            json_object_put(root);
            perror("erreyr");
        }

        json_object *links;
        if (!json_object_object_get_ex(data, "links", &links)) {
            fprintf(stderr, "Invalid JSON response format\n");
            json_object_put(root);
            perror("erreyr");
        }

        json_object *self_url;
        if (!json_object_object_get_ex(links, "self", &self_url)) {
            fprintf(stderr, "Invalid JSON response format\n");
            json_object_put(root);
            perror("erreyr");
        }

        const char *self_url_str = json_object_get_string(self_url);
        size_t self_url_length = strlen(self_url_str);
        char sanitized_url[self_url_length + 1];
        size_t sanitized_url_length = 0;
        for (size_t i = 0; i < self_url_length; i++) {
            if (isprint((unsigned char)self_url_str[i]) && !iscntrl((unsigned char)self_url_str[i])) {
                sanitized_url[sanitized_url_length] = self_url_str[i];
                sanitized_url_length++;
            }
        }
        sanitized_url[sanitized_url_length] = '\0';

        curl = curl_easy_init();
        if (curl) {
            struct curl_slist *headers = NULL;
            char auth_header[100];
            snprintf(auth_header, sizeof(auth_header), "x-apikey: %s", API_KEY);
            headers = curl_slist_append(headers, auth_header);
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
            curl_easy_setopt(curl, CURLOPT_URL, sanitized_url);

            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data_callback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&response_data);

            res = curl_easy_perform(curl);
            if (res != CURLE_OK) {
                fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
                perror("erreyr");
            }
            curl_slist_free_all(headers);   
            curl_easy_cleanup(curl);
        }

        json_object_put(root);

        curl_global_cleanup();

        return response_data;
    }

    return response_data;
}



