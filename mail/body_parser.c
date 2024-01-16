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

// This function  will check if a file already exists in a directory.
int check_dir(char *dir_path, char *id)
{
    // The path of the file that will be checked.
    char *path;

    // asprintf will update the path string with the path of the file that will be checked.
    int a = asprintf(&path, "%s/%s.txt", dir_path, id);
    if (a == -1)
    {
        // Return an error message.
        err(1, "asprintf() failed: %s\n", curl_easy_strerror(a));
    }
    // Check if the file exists within the directory.
    if (access(path, F_OK) == 0)
    {
        // Free any memory that is not needed anymore.
        free(path);
        return 1;
    }
    // Free any memory that is not needed anymore.
    free(path);
    return 0;
}

// This function  will check if a file already exists in a directory.
int check_dir2(char *dir_path, char *id)
{
    // The path of the file that will be checked.
    char *path;
    char *path2;

    // asprintf will update the path string with the path of the file that will be checked.
    int a = asprintf(&path, "%s/mails/%s.txt", dir_path, id);
    if (a == -1)
    {
        // Return an error message.
        err(1, "asprintf() failed: %s\n", curl_easy_strerror(a));
    }
    int b = asprintf(&path2, "%s/spams/%s.txt", dir_path, id);
    if (b == -1)
    {
        // Return an error message.
        err(1, "asprintf() failed: %s\n", curl_easy_strerror(b));
    }
    // Check if the file exists within the directory.
    if (access(path, F_OK) == 0 || access(path2, F_OK) == 0)
    {
        // Free any memory that is not needed anymore.
        free(path);
        free(path2);
        return 1;
    }
    // Free any memory that is not needed anymore.
    free(path);
    free(path2);
    return 0;
}

// This function will be used to get the contents of the emails (by using the IDs).
char** get_contents(char** ids, const char* token, char* path, char *dir_path)
{
    // This is the array of strings that will be returned by the function.
    char** contents = malloc(1);

    // This is the index of the array of strings.
    int index = 0;
    
    // Open the file that will be used to store the IDs.
    FILE* file = fopen(path, "a+");
    if (file == NULL)
    {
        // Return an error message.
        err(1, "fopen() failed");
    }

    // This while loop will keep running until the token is NULL.
    while (ids[index] != NULL)
    {
        // use check_dir to check if the file already exists in the directory.
        if (check_dir2(dir_path, ids[index]) == 1)
        {
            // The file already exists in the directory, so we will skip this iteration.
            contents[index] = NULL;
            index++;
            continue;
        }

        // This is the URL that will be used to get the emails contents. It will be updated with the current id.
        char* url;
        int a = asprintf(&url, "https://www.googleapis.com/gmail/v1/users/me/messages/%s", ids[index]);
        if (a == -1)
        {
            // Free any memory that is not needed anymore.
            free(contents);

            // Return an error message.
            err(1, "asprintf() failed: %s\n", curl_easy_strerror(a));
        }

        // This string will be used to store the data of the current id, and will be reset after each iteration.
        char* data = malloc(300000);

        // This is the string that will updated with the authorization header.
        char* authorization;

        // asprintf will update the authorization string with the authorization header, and will be reset after each iteration.
        int b = asprintf(&authorization, "Authorization: Bearer %s", token);
        if (b == -1)
        {
            // Free any memory that is not needed anymore.
            free(contents);
            free(data);

            // Return an error message.
            errx(1, "asprintf() failed: %s\n", curl_easy_strerror(b));
        }

        // This is the curl object that will be used to get the emails.
        CURL* curl = curl_easy_init();

         // This is the linked list that will be used to store the headers.
        struct curl_slist* headers = NULL;

        // This if statement will check if the curl object was initialized.
        if (curl)
        {
            // This is the curl object that will be used to get the emails.
            curl_easy_setopt(curl, CURLOPT_URL, url);
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, data);
            headers = curl_slist_append(headers, authorization);
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
            curl_easy_perform(curl);
            curl_easy_cleanup(curl);
            curl_slist_free_all(headers);
        }

        // Add the whole data string to contents.
        contents = realloc(contents, (index + 2) * sizeof(char*));
        contents[index] = malloc(strlen(data) + 1);
        printf("%ld\n", strlen(data) + 1);
        strcpy(contents[index], data);
        contents[index + 1] = NULL;

        // Increment the index of the array of strings.
        index++;

        // Reset the data string.
        free(data);

        // Reset the authorization string.
        free(authorization);

        // Reset the url string.
        free(url);
    }

    // Free any memory that is not needed anymore.
    fclose(file);

    // Return the array of strings.
    return contents;
}

// This function will get the image of the email by using attachment ID.
char** get_images(char *ids, const char* token, char *attachments_id)
{
    // This is the array of strings that will be returned by the function.
    char** contents = malloc(1);

    // This is the index of the array of strings.
    int index = 0;

    // This while loop will keep running until the token is NULL.

    // This is the URL that will be used to get the emails contents. It will be updated with the current id.
    char* url;
    
    //check if the attachment id is null
    if (attachments_id == NULL)
    {
        free(contents);
        return NULL;
    }
    int a = asprintf(&url, "https://www.googleapis.com/gmail/v1/users/me/messages/%s/attachments/%s", ids, attachments_id);
    if (a == -1)
    {
        // Free any memory that is not needed anymore.
        free(contents);
        
        err(1, "asprintf() failed: %s\n", curl_easy_strerror(a));
    }
    // Print the current authorization string.

    // This string will be used to store the data of the current id, and will be reset after each iteration.
    char* data = malloc(1000000);
    // This is the string that will updated with the authorization header.
    char* authorization;
    // asprintf will update the authorization string with the authorization header, and will be reset after each iteration.
    int b = asprintf(&authorization, "Authorization: Bearer %s", token);
    if (b == -1)
    {
        // Free any memory that is not needed anymore.
        free(contents);
        free(data);
        // Return an error message.
        errx(1, "asprintf() failed: %s\n", curl_easy_strerror(b));
    }
    // This is the curl object that will be used to get the emails.
    CURL* curl = curl_easy_init();
    // This is the linked list that will be used to store the headers.
    struct curl_slist* headers = NULL;
    // This if statement will check if the curl object was initialized.
    if (curl)
    {
        // This is the curl object that will be used to get the emails.
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, data);
        headers = curl_slist_append(headers, authorization);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
    }
    // Add the whole data string to contents.
    contents = realloc(contents, (index + 2) * sizeof(char*));
    contents[index] = malloc(strlen(data) + 1);
    strcpy(contents[index], data);
    contents[index + 1] = NULL;
    // Increment the index of the array of strings.
    index++;
    // Reset the data string.
    free(data);
    // Reset the authorization string.
    free(authorization);
    // Reset the url string.
    free(url);    
    // Return the array of strings.
    return contents;
}


