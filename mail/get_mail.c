#include "get_mail.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <json-c/json.h>
#include "decodeb64.h"

char* skip_bytes(char* content)
{
    int i = 0;
    while (content[i] != '{')
    {
        i++;
    }
    return content + i;
}

char *get_data(char *image)
{
    skip_bytes(image);
    // Get the data field.
    char *data = strstr(image, "data");
    // I want to isale each string contained by " " ".
    char **data_split = split(data, "\"");
    
    char* data_return = data_split[2];
    for (int i = 0; data_split[i] != NULL; i++)
    {
        if (i == 2)
        {
            continue;
        }
        free(data_split[i]);
    }
    free(data_split);
    return data_return;
}

//get the snippet of the mail
char *get_snippet(char* value, struct json_object *obj) 
{
    // Obtenir la valeur du champ "snippet"
    struct json_object *snippet_obj;
    json_object_object_get_ex(obj, value, &snippet_obj);
    char *snippet = (char*) json_object_get_string(snippet_obj);

    // Retourner la valeur du champ "snippet"
    return snippet; 
}

//get the body of the mail
char *get_body(struct json_object *obj) 
{
    // Obtenir la valeur du champ "body" dans le champ "payload"
    struct json_object *payload_obj;
    json_object_object_get_ex(obj, "payload", &payload_obj);
    struct json_object *parts_obj;
    json_object_object_get_ex(payload_obj, "parts", &parts_obj);

    long unsigned int i = 0;
    while (i < json_object_array_length(parts_obj))
    {
        struct json_object *part_obj = json_object_array_get_idx(parts_obj, i);
        struct json_object *mimeType_obj;
        json_object_object_get_ex(part_obj, "mimeType", &mimeType_obj);
        char *mimeType = (char*) json_object_get_string(mimeType_obj);
        if (strcmp(mimeType, "text/plain") == 0)
        {
            struct json_object *body_obj;
            json_object_object_get_ex(part_obj, "body", &body_obj);
            struct json_object *data_obj;
            json_object_object_get_ex(body_obj, "data", &data_obj);
            char *data = (char*) json_object_get_string(data_obj);

            // Decode the data.
            char *decoded_data = decode_base64(data);
            return decoded_data;
        }
        else if (strcmp(mimeType, "multipart/alternative") == 0)
        {
            struct json_object *parts_obj;
            json_object_object_get_ex(part_obj, "parts", &parts_obj);
            long unsigned int j = 0;
            while (j < json_object_array_length(parts_obj))
            {
                struct json_object *part_obj = json_object_array_get_idx(parts_obj, j);
                struct json_object *mimeType_obj;
                json_object_object_get_ex(part_obj, "mimeType", &mimeType_obj);
                char *mimeType = (char*) json_object_get_string(mimeType_obj);
                if (strcmp(mimeType, "text/plain") == 0)
                {
                    struct json_object *body_obj;
                    json_object_object_get_ex(part_obj, "body", &body_obj);
                    struct json_object *data_obj;
                    json_object_object_get_ex(body_obj, "data", &data_obj);
                    char *data = (char*) json_object_get_string(data_obj);

                    // Decode the data.
                    char *decoded_data = decode_base64(data);
                    return decoded_data;
                }
                j++;
            }
        }
        i++;
    }
    return NULL;
}

//get the sender of the mail in the headers field then get the value field
char *get_sender(char* value, struct json_object *obj) 
{
    // Obtenir la valeur du champ "headers" dans le champ "payload"
    struct json_object *payload_obj;
    json_object_object_get_ex(obj, "payload", &payload_obj);
    struct json_object *headers_obj;
    json_object_object_get_ex(payload_obj, "headers", &headers_obj);

    // Parcourir les "name" du champ "headers" et si le nom est "To" alors on récupère la valeur du champ "value"
    long unsigned int i = 0;
    while (i < json_object_array_length(headers_obj))
    {
        struct json_object *header_obj = json_object_array_get_idx(headers_obj, i);
        struct json_object *name_obj;
        json_object_object_get_ex(header_obj, "name", &name_obj);
        char *name = (char*) json_object_get_string(name_obj);
        if (strcmp(name, "To") == 0)
        {
            struct json_object *value_obj;
            json_object_object_get_ex(header_obj, value, &value_obj);
            char *value = (char*) json_object_get_string(value_obj);
            return value;
        }
        i++;
    }
    return NULL;
}

//get the receiver of the mail in the headers field then get the value field
char *get_receiver(char* value, struct json_object *obj) 
{
    // Obtenir la valeur du champ "headers" dans le champ "payload"
    struct json_object *payload_obj;
    json_object_object_get_ex(obj, "payload", &payload_obj);
    struct json_object *headers_obj;
    json_object_object_get_ex(payload_obj, "headers", &headers_obj);

    // Parcourir les "name" du champ "headers" et si le nom est "From" alors on récupère la valeur du champ "value"
    long unsigned int i = 0;
    while (i < json_object_array_length(headers_obj))
    {
        struct json_object *header_obj = json_object_array_get_idx(headers_obj, i);
        struct json_object *name_obj;
        json_object_object_get_ex(header_obj, "name", &name_obj);
        char *name = (char*) json_object_get_string(name_obj);
        if (strcmp(name, "From") == 0)
        {
            struct json_object *value_obj;
            json_object_object_get_ex(header_obj, value, &value_obj);
            char *value = (char*) json_object_get_string(value_obj);

            // Parse the value to get only the email address
            int sizeofemail = 0;
            int index = 0;
            while (value[index] != '<')
            {
                index++;
            }
            index++;
            int saveindex = index;
            while (value[index] != '>')
            {
                sizeofemail++;
                index++;
            }
            char *email = malloc(sizeofemail * sizeof(char));
            memcpy(email, value + saveindex, sizeofemail);
            return email;
        }
        i++;
    }
    return NULL;
}

//get the Subject of the mail in the headers field then get the value field
char *get_subject(char* value, struct json_object *obj) 
{
    // Obtenir la valeur du champ "headers" dans le champ "payload"
    struct json_object *payload_obj;
    json_object_object_get_ex(obj, "payload", &payload_obj);
    struct json_object *headers_obj;
    json_object_object_get_ex(payload_obj, "headers", &headers_obj);

    // Parcourir les "name" du champ "headers" et si le nom est "Subject" alors on récupère la valeur du champ "value"
    long unsigned int i = 0;
    while (i < json_object_array_length(headers_obj))
    {
        struct json_object *header_obj = json_object_array_get_idx(headers_obj, i);
        struct json_object *name_obj;
        json_object_object_get_ex(header_obj, "name", &name_obj);
        char *name = (char*) json_object_get_string(name_obj);
        if (strcmp(name, "Subject") == 0)
        {
            struct json_object *value_obj;
            json_object_object_get_ex(header_obj, value, &value_obj);
            char *value = (char*) json_object_get_string(value_obj);
            return value;
        }
        i++;
    }
    return NULL;
}

//get the date of the mail in the headers field then get the value field
char *get_date(char* value, struct json_object *obj) 
{
    // Obtenir la valeur du champ "headers" dans le champ "payload"
    struct json_object *payload_obj;
    json_object_object_get_ex(obj, "payload", &payload_obj);
    struct json_object *headers_obj;
    json_object_object_get_ex(payload_obj, "headers", &headers_obj);

    // Parcourir les "name" du champ "headers" et si le nom est "Date" alors on récupère la valeur du champ "value"
    long unsigned int i = 0;
    while (i < json_object_array_length(headers_obj))
    {
        struct json_object *header_obj = json_object_array_get_idx(headers_obj, i);
        struct json_object *name_obj;
        json_object_object_get_ex(header_obj, "name", &name_obj);
        char *name = (char*) json_object_get_string(name_obj);
        if (strcmp(name, "Date") == 0)
        {
            struct json_object *value_obj;
            json_object_object_get_ex(header_obj, value, &value_obj);
            char *value = (char*) json_object_get_string(value_obj);
            return value;
        }
        i++;
    }
    return NULL;
}

char **get_attachment_ids(struct json_object *obj)
{
    struct json_object *payload_obj;
    json_object_object_get_ex(obj, "payload", &payload_obj);
    struct json_object *parts_obj;
    json_object_object_get_ex(payload_obj, "parts", &parts_obj);

    int num_parts = json_object_array_length(parts_obj);
    char **attachment_ids = (char **)malloc((num_parts + 1) * sizeof(char *));

    int idx = 0;
    for (int i = 0; i < num_parts; i++)
    {
        struct json_object *part_obj = json_object_array_get_idx(parts_obj, i);
        struct json_object *mimeType_obj;
        json_object_object_get_ex(part_obj, "mimeType", &mimeType_obj);
        char *mimeType = (char *)json_object_get_string(mimeType_obj);

        if (strstr(mimeType, "image/") != NULL)
        {
            struct json_object *body_obj;
            json_object_object_get_ex(part_obj, "body", &body_obj);
            struct json_object *attachmentId_obj;
            json_object_object_get_ex(body_obj, "attachmentId", &attachmentId_obj);
            char *attachmentId = (char *)json_object_get_string(attachmentId_obj);

            attachment_ids[idx++] = strdup(attachmentId);
        }
    }

    attachment_ids[idx] = NULL;
    return attachment_ids;
}

//get the data 

//get all info of the mail
Tuple *get_info(char *json_string) 
{
    //Analyser la chaîne de caractères JSON
    char* json_str= skip_bytes(json_string);
    struct json_object *obj = json_tokener_parse(json_str);

    // Créer un tableau de pointeurs de type Tuple
    Tuple *infos = malloc(20 * sizeof(Tuple));

    // Obtenir la valeur des champs.
    char *sender = get_sender("value", obj);
    char *date = get_date("value", obj);
    char *subject = get_subject("value", obj);
    char *body = get_body(obj);

    // === NEW ===
    char **attachment_ids = get_attachment_ids(obj);
    int i = 0;
    while (attachment_ids[i] != NULL)
    {
        //Allouer de la mémoire pour les valeurs des champs.
        infos[4 + i].value = malloc(strlen(attachment_ids[i]) + 1);
        //Copier les valeurs des champs dans le tableau de pointeurs de type Tuple.
        strcpy(infos[4 + i].value, attachment_ids[i]);
        i++;
    }
    infos[4 + i].value = NULL;
    // === END NEW ===

    // Allouer de la mémoire pour les valeurs des champs.
    infos[0].value = malloc(strlen(sender) + 1);
    infos[1].value = malloc(strlen(date) + 1);
    infos[2].value = malloc(strlen(subject) + 1);
    infos[3].value = malloc(strlen(body) + 1);

    // Copier les valeurs des champs dans le tableau de pointeurs de type Tuple.
    strcpy(infos[0].value, sender);
    strcpy(infos[1].value, date);
    strcpy(infos[2].value, subject);
    strcpy(infos[3].value, body);

    json_object_put(obj);
    for (int i =0; attachment_ids[i] != NULL; i++)
    {
        free(attachment_ids[i]);
    }
    free(attachment_ids);
    free(body);

    // Retourner le tableau de pointeurs de type Tuple.
    return infos;
}
