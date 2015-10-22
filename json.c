

#include <stdlib.h>
#include <stdio.h>

#include "json.h"
#include "dmap.h"

char purge_whitespace(char **str) {
    while(1) {
        switch(**str) {
            case ' ':
            case '\t':
            case '\n':
                (void)((*str)++);
                break;
            default:
                return **str;
        }
    }
    return 0;
}

json *typr(int type, void *insrt) {
    json *result = calloc(sizeof(json), 1);
    switch(type) {
        case 0:
            result->obj = insrt;
            break;
        case 1:
            result->number = *(unsigned int *)insrt;
            break;
        case 2:
            result->string = insrt;
            break;
        case 3:
            result->boolean  = *(unsigned char *)insrt;
            break;
        default:
            free(result);
            return NULL;
    }
    result->type = type;
    return result;
}

dmap *array(char **str) {
    dmap *result = map_new();
    int ctr = 0;
    while(**str != ']') {
        purge_whitespace(str);
        char *name = itoaa(ctr);
        put(result, name, read_json(str));
        (void)(*str)++; // purge comma
        ctr++;
    }
    return result;
}

char *string(char **str) {
    int max = 10;
    int size = 0;
    char *result = calloc(sizeof(char), max);
    while(**str && **str != '"') {
        if (**str == '\\') {
            (void)(*str)++;
        }
        result[size++] = **str;
        result[size+1] = 0;
        if (size == max-1) {
            max *=2;
            result = realloc(result, max);
        }
        (void)(*str)++;
    }
    (void)(*str)++; //remove quote
    return result;
}

dmap *object(char **str) {
    dmap *result = map_new();
    while(purge_whitespace(str) != '}') {
        char *name = read_json(str)->string;
        purge_whitespace(str);
        (void)(*str)++; // purge colon
        put(result, name, read_json(str));
    }
    (void)(*str)++; //remove curly
    return result;
}

int is_numeric(char c) {
    return c<='9' && c>='0';
}

unsigned int number(char **str) {
    unsigned int result = 0;
    while(is_numeric(**str)) {
        result *= 10;
        result += (**str - '0');
        (void)(*str)++;
    }
    return result;
}

unsigned char boolean(char **str) {
    switch(**str) {
        case 'T': case 't':
            (*str) += 4;
            return 1;
        case 'F': case 'f':
            (*str) += 5;
            return 0;
    }
    perror("how did you even");
    return 0;
}

json *read_json(char **str) {
    unsigned int n;
    unsigned char b;
    while(**str) {
        purge_whitespace(str);
        switch(**str) {
            case '[':
                (void)(*str)++;
                return typr(0, array(str));
            case '{':
                (void)(*str)++;
                return typr(0, object(str));
            case '"':
                (void)(*str)++;
                return typr(2, string(str));
            case '1': case '2': case '3': case '4': case '5':
            case '6': case '7': case '8': case '9': case '0':
            case '-': case '.':
                n = number(str);
                return typr(1, &n);
            case 't': case 'f':
            case 'T': case 'F':
                b = boolean(str);
                return typr(3, &b);
            case ',':
                (void)(*str)++;
                break;
            case 0:
                return NULL;
        }
    }
    return NULL;
}

void free_json(json *j) {
    char *name;
    json *nxt;
    if (j) {
        switch(j->type) {
            case 2:
                free(j->string);
                return;
            case 0:
                map_each(j->obj, name, nxt) {
                    free(name);
                    free_json(nxt);
                }
        }
        free(j);
    }
}

void print_json(json *j) {
    char *name;
    json *nxt;
    if (j) {
        switch(j->type) {
            case 2:
                puts(j->string);
                return;
            case 1:
                printf("%i\n", j->number);
                return;
            case 3:
                puts(j->boolean?"TRUE":"FALSE");
                return;
            case 0:
                puts("{");
                map_each(j->obj, name, nxt) {
                    printf("\"%s\":", name);
                    print_json(nxt);
                }
                puts("}");
        }
    }
}

char *itoaa(unsigned int i) {
    unsigned int tmp = i;
    unsigned int digits = 0;
    while(tmp) {
        digits++;
        tmp /= 10;
    }
    char *result = calloc(sizeof(char), digits+1);
    sprintf(result, "%i", i);
    return result;
}

json *jarray_get(json *j, int i) {
    char *c = itoaa(i);
    json *res = get((j->obj), c);
    free(c);
    return res;
}

