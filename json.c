/*
 * Copyright Ted Meyer 2015
 *
 * see LICENSE for details
 *
 */

#include <stdlib.h>
#include <stdio.h>

#include "json.h"
#include "dmap.h"

char purge_whitespace(charstream *stream) {
    while(1) {
        switch(seq(stream)) {
            case ' ':
            case '\t':
            case '\n':
                next(stream);
                break;
            default:
                return seq(stream);
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

dmap *array(charstream *str) {
    dmap *result = map_new();
    int ctr = 0;
    while(seq(str) != ']') {
        purge_whitespace(str);
        char *name = itoaa(ctr);
        put(result, name, read_json(str));
        (void)next(str); // purge comma
        ctr++;
    }
    return result;
}

char *string(charstream *str) {
    int max = 10;
    int size = 0;
    char *result = calloc(sizeof(char), max);
    while(seq(str) && seq(str) != '"') {
        if (seq(str) == '\\') {
            (void) next(str);
        }
        result[size++] = seq(str);
        result[size+1] = 0;
        if (size == max-1) {
            max *=2;
            result = realloc(result, max);
        }
        (void) next(str);
    }
    (void) next(str); //remove quote
    return result;
}

dmap *object(charstream *str) {
    dmap *result = map_new();
    while(purge_whitespace(str) != '}') {
        char *name = read_json(str)->string;
        purge_whitespace(str);
        (void)next(str); // purge colon
        put(result, name, read_json(str));
    }
    (void)next(str); //remove curly
    return result;
}

int is_numeric(char c) {
    return c<='9' && c>='0';
}

unsigned int number(charstream *str) {
    unsigned int result = 0;
    while(is_numeric(seq(str))) {
        result *= 10;
        result += (seq(str) - '0');
        (void)next(str);
    }
    return result;
}

unsigned char boolean(charstream *str) {
    switch(seq(str)) {
        case 'T': case 't':
            skip(str, 4);
            return 1;
        case 'F': case 'f':
            skip(str, 5);
            return 0;
    }
    perror("how did you even");
    return 0;
}

json *read_json(charstream *j) {
    unsigned int n;
    unsigned char b;
    while(seq(j)) {
        purge_whitespace(j);
        switch(seq(j)) {
            case '[':
                next(j);
                return typr(0, array(j));
            case '{':
                next(j);
                return typr(0, object(j));
            case '"':
                next(j);
                return typr(2, string(j));
            case '1': case '2': case '3': case '4': case '5':
            case '6': case '7': case '8': case '9': case '0':
            case '-': case '.':
                n = number(j);
                return typr(1, &n);
            case 't': case 'f':
            case 'T': case 'F':
                b = boolean(j);
                return typr(3, &b);
            case ',':
                next(j);
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

