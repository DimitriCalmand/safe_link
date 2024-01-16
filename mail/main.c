#include "access_token.h"
#include "id_parser.h"
#include "body_parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "get_mail.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "decodeb64.h"
#include <sys/stat.h>
#include "image.h"

int main(void)
{
    // We want to print all the mail IDs of the user.

    // Set the client ID and client secret.
    char* client_id = "391645126883-hi7agjilffcrck31lcl740mljfk2defi.apps.googleusercontent.com";
    char* client_secret = "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";

    // Ask the user to enter his email address (this will be used to create a directory).
    char email[100];
    printf("Please enter your email address: ");
    scanf("%s", email);
    // Take the first part of the email address.

    // We want to create a directory with the name of the email address.
    char *path = NULL;
    int c = asprintf(&path, "database/%s", email);
    if (c == -1)
    {
        // Return an error message.
        err(1, "asprintf() failed: %s\n", curl_easy_strerror(c));
    }
    // Create a directory with the name of the email address. If the directory already exists, it will not be created.
    mkdir(path, 0777);

    // Save the path of the directory.
    char *path2 = NULL;
    int d = asprintf(&path2, "database/%s", email);
    if (d == -1)
    {
        // Return an error message.
        err(1, "asprintf() failed: %s\n", curl_easy_strerror(d));
    }

    // Add ids.txt to the path.
    path = realloc(path, strlen(path) + strlen("/ids.txt") + 1);
    strcat(path, "/ids.txt");

    // First, we need to get the authorization code.
    char* code = authorization_code();

    // Next, we need to get the access token.
    char* token = access_token(code, client_id, client_secret);

    // Next, we need to get the ID of the emails.
    char** id = get_id(token);
    // Use the add_ids function to add the IDs to the file.
    add_ids(id, path, path2);

    // Print the contents of the emails.
    char** contents = get_contents(id, token, path, path2);
    
    //new_email(id, path2, contents);

    // Free the memory of the infos array.

    // Free the entire memory (including the memory of the array of strings).
    free(token);
    // free(code);
    int i = 0;
    while (id[i] != NULL)
    {
        free(id[i]);
        i++;
    }
    free(id);
    i = 0;
    /*while (contents[i] != NULL)
    {
        free(contents[i]);
        i++;
    }
    free(contents);*/

    return 0;
}
