#include "get_json.h"
#include "get_malicious.h"
char* get_answer(struct json_object *obj)
{
    struct json_object *data;
    json_object_object_get_ex(obj, "data", &data);
    struct json_object *attributes;
    json_object_object_get_ex(data, "attributes", &attributes);
    struct json_object *stats;
    json_object_object_get_ex(attributes, "stats", &stats);
    struct json_object *malicious;
    json_object_object_get_ex(stats, "malicious", &malicious);
    char *malicious_string = (char *)json_object_get_string(malicious);
    return malicious_string;
}

int is_malicious(char* file)
{
    struct ResponseData data = get_mallicious(file);
    char* contents = data.data;
    struct json_object *obj = json_tokener_parse(contents);
    char* malicious = get_answer(obj);
    free(contents);
    int res = 0;
    if (malicious[0] != '0')
    {
        res = 1;
    }
    json_object_put(obj);
    return res;
}
