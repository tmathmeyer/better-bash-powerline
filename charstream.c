
#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "charstream.h"


#ifdef NOCODE
typedef
struct {
    unsigned int type; // 0 is (char *), 1 is (FILE *)
    unsigned int done;
    FILE *ptr;
    char **str;
    char buff;
} charstream;
#endif


char next(charstream *cs) {
    ASSERT(cs, -1);
    if (cs->done) {
        cs->buff = 0;
    } else {
        switch(cs->type) {
            case 0:
                if (**cs->str) {
                    (void)(*(cs->str))++;
                } else {
                    cs->done = 1;
                }
                cs->buff = **cs->str;
                break;
            case 1:
                cs->buff = fgetc(cs->ptr);
                if (cs->buff == EOF) {
                    cs->done = 1;
                    cs->buff = 0;
                }
        }
    }
    return cs->buff;
}

char seq(charstream *cs) {
    ASSERT(cs, -1);
    return cs->buff;
}

void skip(charstream *cs, unsigned int nt) {
    for(unsigned int i=0; i<nt && !cs->done; i++) {
        (void) next(cs);
    }
}

charstream *streamfile(FILE *ptr) {
    charstream *res = calloc(sizeof(charstream), 1);
    res->type = 1;
    res->done = 0;
    res->ptr = ptr;
    res->str = 0;
    res->buff = fgetc(ptr);
    return res;
}

charstream *streamstr(char *str) {
    charstream *res = calloc(sizeof(charstream), 1);
    res->type = 0;
    res->done = 0;
    res->ptr = 0;
    res->str = calloc(sizeof(char *), 1);
    (res->str)[0] = strdup(str);
    res->buff = *str;
    return res;
}
