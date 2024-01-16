#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <curl/curl.h>
#include <err.h>
#include "id_parser.h"

void save_refresh_token(char* token)
{
    char* path = "./mail/refresh_token";
    int fd = open(path, O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if (fd == -1)
    {
        perror("error open\n");
    }
    int a = write(fd, token, strlen(token));
    if (a == -1)
    {
        perror("error write\n");
    }
    close(fd);
}
// This is the callback function will be called by libcurl as soon as there is data received that needs to be saved.
size_t write_data(char* ptr, size_t size, size_t nmemb, void *userdata)
{
    // Append the data to the end of the string.
    strncat(userdata, ptr, size * nmemb);

    // Return the number of bytes received, indicating to curl that it should not stop the download.
    return size * nmemb;
}

// This authorization_code function will return the authorization code by making sure the user has entered the right code (provided by the Gmail API).
void authorization_code()
{
    // This is the URL that will be used to get the authorization code.
    // char* url = "open -a safari https://urlz.fr/kF2i";
    char* url = "firefox https://urlz.fr/kF2i";
    if (system(url) == -1)
    {
        perror("system");
    }
}

// This access_token function will return the access token by making a POST request to the Gmail API.
char* access_token(char* code, char* client_id, char* client_secret)
{
    // This is the access token that will be returned by the function, it must be initialized with a big enough size.
    char* data = malloc(5192);
    data[0] = '\0';

    // This is the curl_easy_init function that will initialize the curl object.    
    CURL* curl = curl_easy_init();
    CURLcode res = CURLE_OK;

    if (curl)
    {
        // This is the URL that will be used to get the access token.
        char* url = "https://www.googleapis.com/oauth2/v4/token";

        // asprintf will create a string that will be used to store the POST data.
        char* post_url = NULL;
        int a = asprintf(&post_url, "code=%s&client_id=%s&client_secret=%s&redirect_uri=urn:ietf:wg:oauth:2.0:oob&grant_type=authorization_code", code, client_id, client_secret);
        if (a == -1)
        {
            // Free any memory that is not needed anymore.
            // free(url);
            free(post_url);
            free(code);

            // Return an error message.
            errx(1, "asprintf() failed: %s\n", curl_easy_strerror(a));
        }
        // This is the curl_easy_setopt function that will set the URL.
        curl_easy_setopt(curl, CURLOPT_URL, url);

        // This is the curl_easy_setopt function that will set the POST data.
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_url);

        // This is the curl_easy_setopt function that will set the callback function.
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);

        // This is the curl_easy_setopt function that will set the data that will be passed to the callback function.
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, data);

        // This is the curl_easy_perform function that will perform the request.
        res = curl_easy_perform(curl);

        // Check for errors.
        if (res != CURLE_OK)
        {
            // Free any memory that is not needed anymore.
            // free(url);
            free(post_url);
            free(code);

            // Return an error message.
            errx(1, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        }

        // This is the curl_easy_cleanup function that will cleanup the curl object.
        curl_easy_cleanup(curl);

        // We only want to return the access token.
        // The acces token is in a JSON format and appears like these : "access_token": "the_access_token".
        // We will parse the JSON string to get the access token only using the split function.
        char** split_data = split(data, "\"");
        char* token = split_data[3];
        save_refresh_token(split_data[9]);
        // Free any memory that is not needed anymore.
        // free(url);
        free(post_url);
        free(code);
        free(data);
        for (int i = 0; split_data[i] != NULL; i++) {
            if (i == 3)
                continue;
            free(split_data[i]);
        }
        free(split_data);
        
        // Return the access token.
        return token;
    }

    // Return an error message.
    errx(1, "curl_easy_init() failed: %s\n", curl_easy_strerror(res));
}
