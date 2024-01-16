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
#include "access_token.h"

char* is_token_already_use()
{
    int fd = open("./mail/refresh_token", O_RDONLY);
    if (fd == -1)
    {
        perror("open");
    }
    char buffer[1024];
    int r = read(fd, buffer, 1024);
    if (r == -1)
    {
        perror("read");
    }
    buffer[r] = '\0';
    if (buffer[0] == '.')
    {
        printf("enter\n");
        return "0";
    }
    char* res = malloc(strlen(buffer) + 1);
    strcpy(res, buffer);
    res[strlen(buffer)] = '\0';
    return res;
}
char* get_mail()
{
    int fd = open("./mail/email", O_RDONLY);
    if (fd == -1)
    {
        perror("open");
    }
    char buffer[1024];
    int r = read(fd, buffer, 1024);
    if (r == -1)
    {
        perror("read");
    }
    buffer[r] = '\0';
    char* res = malloc(strlen(buffer) + 1);
    strcpy(res, buffer);
    if (res[strlen(buffer) - 1] == '\n')
    {
        res[strlen(buffer) - 1] = '\0';
    }
    res[strlen(buffer)] = '\0';
    return res;
}

void save_mail(char* mail)
{
    char* path = "./mail/email";
    int fd = open(path, O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if (fd == -1)
    {
        perror("error open\n");
    }
    int a = write(fd, mail, strlen(mail));
    if (a == -1)
    {
        perror("error write\n");
    }
    close(fd);

}
char* refresh_token(char* refresh_token, char* client_id, char* client_secret)
{
    // This is the access token that will be returned by the function, it must be initialized with a big enough size.
    char* data = malloc(5192);
    data[0] = '\0';

    // This is the curl_easy_init function that will initialize the curl object.    
    CURL* curl = curl_easy_init();
    CURLcode res = CURLE_OK;
// curl -s --request POST --data "client_id=391645126883-hi7agjilffcrck31lcl740mljfk2defi.apps.googleusercontent.com&
// client_secret=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&refresh_token=<REFRESH_TOKEN>&
// grant_type=refresh_token" https://oauth2.googleapis.com/token

    if (curl)
    {
        // This is the URL that will be used to get the access token.
        char* url = "https://oauth2.googleapis.com/token";

        // asprintf will create a string that will be used to store the POST data.
        char* post_url = NULL;
        int a = asprintf(&post_url, "client_id=%s&client_secret=%s&refresh_token=%s&grant_type=refresh_token", client_id, client_secret, refresh_token);
        if (a == -1)
        {
            // Free any memory that is not needed anymore.
            // free(url);
            free(post_url);

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
            free(refresh_token);
            free(post_url);
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

        free(post_url);
        free(data);

        for (int i = 0; split_data[i] != NULL; i++) 
        {
            if (i == 3)
                continue;
            free(split_data[i]);
        }
        free(split_data);
        printf("data: %s\n", token);
        // Return the access token.
        return token;
    }

    // Return an error message.
    errx(1, "curl_easy_init() failed: %s\n", curl_easy_strerror(res));
}