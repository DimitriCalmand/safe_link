#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <curl/curl.h>
#include "access_token.h"
#include <err.h>
#include "body_parser.h"
#include "get_mail.h"
#include "id_parser.h"
#include "../secu/get_malicious.h"
#include "../secu/get_json.h"

// This split function will split a string into an array of strings.
char** split(char* input, char* delimiter)
{
    // This is the array of strings that will be returned by the function.
    char** output = malloc(1);

    // This is the token that will be used to split the string.
    char* token = strtok(input, delimiter);

    // This is the index of the array of strings.
    int index = 0;

    // This while loop will keep running until the token is NULL.
    while (token != NULL)
    {
        // This is the array of strings that will be returned by the function.
        output = realloc(output, (index + 2) * sizeof(char*));
        output[index] = malloc(strlen(token) + 1);
        strcpy(output[index], token);
        output[index + 1] = NULL;

        // This is the token that will be used to split the string.
        token = strtok(NULL, delimiter);

        // Increment the index of the array of strings.
        index++;
    }

    // Return the array of strings.
    return output;
}

// This get_id function will return the different IDs of the emails.
char** get_id(const char* token)
{
    // This is the URL that will be used to get the emails.
    char* url = "https://www.googleapis.com/gmail/v1/users/me/messages";

    // This is the string that will be used to store the data received from the Gmail API.
    char* data = malloc(8192);

    // This is the string that will updated with the authorization header.
    char* authorization;

    // asprintf will update the authorization string with the authorization header.
    int a = asprintf(&authorization, "Authorization: Bearer %s", token);
    if (a == -1)
    {
        // Free any memory that is not needed anymore.
        free(data);

        // Return an error message.
        errx(1, "asprintf() failed: %s\n", curl_easy_strerror(a));
    }

    // This is the linked list that will be used to store the headers.
    struct curl_slist* headers = NULL;

    // This is the curl_easy_init function that will initialize a libcurl easy session.
    CURL* curl = curl_easy_init();

    if (curl)
    {
        // This is the curl_easy_setopt function that will set the URL to get.
        curl_easy_setopt(curl, CURLOPT_URL, url);

        // This is the curl_easy_setopt function that will set the function that will be called by libcurl as soon as there is data received that needs to be saved.
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);

        // This is the curl_easy_setopt function that will set the data that will be passed to the write_data function.
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, data);

        // This is the curl_easy_setopt function that will set the linked list of headers.
        headers = curl_slist_append(headers, authorization);

        // This is the curl_easy_setopt function that will set the linked list of headers.
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        // This is the curl_easy_perform function that will perform the entire request.
        curl_easy_perform(curl);

        // This is the curl_easy_cleanup function that will cleanup the curl session.
        curl_easy_cleanup(curl);

        // This is the curl_slist_free_all function that will free all the memory used by the linked list of headers.
        curl_slist_free_all(headers);

        // The data string is a JSON string. The JSON string will be parsed to get the different IDs of the emails that are in the "messages" array.
        // The "messages" array is an array of objects. Each object has an "id" key. The value of the "id" key is the ID of the email.
        // For example, the JSON string is {"messages": [{"id": "18613a0ca79aafaa""}]}.
        // We want to get the value of the "id" key for each object in the "messages" array.
        char** messages = split(data, "\"");

        // This is the array of strings that will be returned by the function and will contain the different IDs of the emails.
        char** ids = malloc(1);

        // This is the index of the array of strings.
        int index = 0;

        // This for loop will loop through the messages array.
        for (int i = 0; messages[i] != NULL; i++)
        {
            // If the current string is "id" then move to two strings ahead.
            if (strcmp(messages[i], "id") == 0)
            {
                // This is the array of strings that will be returned by the function and will contain the different IDs of the emails.
                ids = realloc(ids, (index + 2) * sizeof(char*));
                ids[index] = malloc(strlen(messages[i + 2]) + 1);
                strcpy(ids[index], messages[i + 2]);
                ids[index + 1] = NULL;

                // Increment the index of the array of strings.
                index++;
            }
        }

        // Free any memory that is not needed anymore.
        free(data);
        // Free the all the strings in the messages array.
        for (int i = 0; messages[i] != NULL; i++)
        {
            free(messages[i]);
        }
        free(messages);
        free(authorization);

        // Return the array of strings.
        return ids;
    }

    // Return NULL.
    return NULL;
}

// This function will add all the ids to the ids.txt file.
void add_ids(char** ids, char *path, char *dir_path)
{
    // This is the file that will be used to store the IDs of the emails that have already been read.
    // The file will be opened in append mode and will be created if it does not exist.
    FILE* file = fopen(path, "a");
    if (file == NULL)
    {
        // Return an error message.
        errx(1, "Could not open file: %s\n", path);
    }

    // This for loop will loop through the ids array.
    for (int i = 0; ids[i] != NULL; i++)
    {
        if (check_dir2(dir_path, ids[i]) == 1)
        {
            continue;
        }

        // Append the ID to the file.
        fprintf(file, "%s\n",ids[i]);
    }

    // Close the file.
    fclose(file);
}

// This function will create a .txt file for each email and will store the contents (subject, sender, body, etc.) of the email in the file.
void new_email(char** ids, char *path, char **contents, char *server_ip, char *server_port, model_spam* model, const char* token)
{
    // This function will create a .txt file for each email and will store the contents (subject, sender, body, etc.) of the email in the file.
    // This for loop will loop through the ids array.
    char *file_path2 = NULL;
    for (int i = 0; ids[i] != NULL; i++)
    {
        if (check_dir2(path, ids[i]) == 1)
        {
            continue;
        }

        // We want to get the different parts of the email (subject, sender, body, etc.).
        Tuple *infos = get_info(contents[i]);

        // Use the is_spam() function to check if the email is spam.
        if (is_spam(server_ip, server_port, infos[3].value, infos[0].value, model) > 0.8)
        {
            // Update the path of the file.
            char *file_path = NULL;
            int a = asprintf(&file_path, "%s/spams/%s.txt", path, ids[i]);
            if (a == -1)
            {
                errx(1, "Could not update file path\n");
            }
            
            char *dir_path = NULL;
            int b = asprintf(&dir_path, "%s/spams", path);
            if (b == -1)
            {
                errx(1, "Could not update dir path\n");
            }

            // Use check_dir() to check if the ID.txt file already exists.
            // If the file already exists then skip it.
            if (check_dir(dir_path, ids[i]) == 1)
            {
                free(file_path);
                continue;
            }

            // This is the file that will be used to store the contents of the email.
            // The file will be opened in write mode and will be created if it does not exist.
            FILE* file = fopen(file_path, "a");
            if (file == NULL)
            {
                // Return an error message.
                errx(1, "Could not open file: %s\n", file_path);
            }
            // Write the infos to the file.
            //printf("Sender: %s\n", infos[0].value);
            fprintf(file, "Sender: %s\n", infos[0].value);
            //printf("Date: %s\n", infos[1].value);
            fprintf(file, "Date: %s\n", infos[1].value);
            //printf("Subject: %s\n", infos[2].value);
            fprintf(file, "Subject: %s\n", infos[2].value);
            //printf("Body: %s\n", infos[3].value);
            fprintf(file, "Body: %s\n", infos[3].value);

            int j = 4;
            while (infos[j].value != NULL)
            {
                fprintf(file, "Photo %d \n",j-3);
                //j++;
                // Get the images.
                char** images = get_images(ids[i], token, infos[j].value);
                char *data = get_data(images[0]);
                const char* base64url_string = data;
                unsigned char* decoded_data;
                size_t decoded_length;
                base64url_decode(base64url_string, &decoded_data, &decoded_length);

                // Utilisez les données décodées comme vous le souhaitez
                // Par exemple, enregistrez les données dans un fichier
                int b = asprintf(&file_path2, "%s/%s.%d.png", path, ids[i], j-3);
                if (b == -1)
                {
                    // Return an error message.
                    errx(1, "asprintf() failed: %s\n", curl_easy_strerror(b));
                }
                FILE* file = fopen(file_path2, "wb");
                if (file != NULL) {
                    fwrite(decoded_data, 1, decoded_length, file);
                    // test if the file is malicious
                    if (is_malicious(file_path2) == 1)
                    {
                        //printf("Malicious file detected: %s\n", file_path2);
                        printf("Malicious image detected: %s\n", file_path2);
                    }
                    else
                    {
                        printf("Image %s is not malicious\n", file_path2);
                    }
                    fclose(file);
                }
                for (int k = 0; images[k] != NULL; k++)
                {
                    free(images[k]);
                }
                free(images);
                // Libérez la mémoire allouée pour les données décodées
                free(decoded_data);
                j++;
                free(data);
                
            }
            // === END NEW ===
    
            // Close the file.
            fclose(file);
            // Free any memory that is not needed anymore.
            free(file_path);
        }
        else
        {
            // Update the path of the file.
            char *file_path = NULL;
            int a = asprintf(&file_path, "%s/mails/%s.txt", path, ids[i]);
            if (a == -1)
            {
                errx(1, "Could not update file path\n");
            }

            char *dir_path = NULL;
            int b = asprintf(&dir_path, "%s/mails", path);
            if (b == -1)
            {
                errx(1, "Could not update dir path\n");
            }

            // Use check_dir() to check if the ID.txt file already exists.
            // If the file already exists then skip it.
            if (check_dir(dir_path, ids[i]) == 1)
            {
                free(file_path);
                continue;
            }

            // This is the file that will be used to store the contents of the email.
            // The file will be opened in write mode and will be created if it does not exist.
            FILE* file = fopen(file_path, "a");
            if (file == NULL)
            {
                // Return an error message.
                errx(1, "Could not open file: %s\n", file_path);
            }
            // Write the infos to the file.
            //printf("Sender: %s\n", infos[0].value);
            fprintf(file, "Sender: %s\n", infos[0].value);
            //printf("Date: %s\n", infos[1].value);
            fprintf(file, "Date: %s\n", infos[1].value);
            //printf("Subject: %s\n", infos[2].value);
            fprintf(file, "Subject: %s\n", infos[2].value);
            //printf("Body: %s\n", infos[3].value);
            fprintf(file, "Body: %s\n", infos[3].value);

            int j = 4;
            while (infos[j].value != NULL)
            {
                fprintf(file, "Photo%d \n",j-3 );
                //j++;
                // Get the images.
                char** images = get_images(ids[i], token, infos[j].value);
                char *data = get_data(images[0]);
                const char* base64url_string = data;
                unsigned char* decoded_data;
                size_t decoded_length;
                base64url_decode(base64url_string, &decoded_data, &decoded_length);

                // Utilisez les données décodées comme vous le souhaitez
                // Par exemple, enregistrez les données dans un fichier
                int b = asprintf(&file_path2, "%s/%s.%d.png", path, ids[i], j-3);
                if (b == -1)
                {
                    // Return an error message.
                    errx(1, "asprintf() failed: %s\n", curl_easy_strerror(b));
                }
                FILE* file = fopen(file_path2, "wb");
                if (file != NULL) {
                    fwrite(decoded_data, 1, decoded_length, file);
                    // test if the file is malicious
                    if (is_malicious(file_path2) == 1)
                    {
                        //printf("Malicious file detected: %s\n", file_path2);
                        printf("Malicious image detected: %s\n", file_path2);
                    }
                    else
                    {
                        printf("Image %s is not malicious\n", file_path2);
                    }
                    fclose(file);
                }
                for (int k = 0; images[k] != NULL; k++)
                {
                    free(images[k]);
                }
                free(images);

                // Libérez la mémoire allouée pour les données décodées
                free(decoded_data);
                j++;
                free(data);
                
            }
            // === END NEW ===
    
            // Close the file.
            fclose(file);
            // Free any memory that is not needed anymore.
            free(file_path);
        }
        for (int i = 0; infos[i].value != NULL; i++)
        {
            free(infos[i].value);
        }
        free(infos);

    }

}
