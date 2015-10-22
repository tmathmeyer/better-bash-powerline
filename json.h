
#ifndef _JSON_H_
#define _JSON_H_

#include "dmap.h"

typedef
struct json {
    union {
        dmap *obj;
        unsigned int number;
        char *string;
        unsigned char boolean;
    };
    int type;
} json;

json *read_json(char **string);
void print_json(json *j);
void free_json(json *j);
char *itoaa(unsigned int i);
json *jarray_get(json *j, int i);

#define jarray_each(json, o) \
    for(unsigned int i=0;i<(json)->obj->size && ((o)=jarray_get((json), i));i++)

#endif

