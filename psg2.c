/*
 * Copyright Ted Meyer 2015
 *
 * see LICENSE for details
 *
 */

#define _DEFAULT_SOURCE

#include <sys/types.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "json.h"
#include "charstream.h"

#define SOLID_RIGHT ""
#define LINE_RIGHT ""
#define SOLID_LEFT ""
#define LINE_LEFT ""

#define bool unsigned char
#define true 1
#define false 0

typedef
struct psg {
    char *parsed;
    json *conf;
    struct psg *next;
} psg;

static dmap *map = NULL;
static bool colors = true;

char *matchreplace(char *str, json *tofrom) {
    if (!tofrom || !(tofrom->obj)) {
        return str;
    }

    json *j_to = get(tofrom->obj, "to");
    json *j_from = get(tofrom->obj, "from");

    if (!j_to || !j_from) {
        return str;
    }

    char *to = j_to->string;
    char *from = j_from->string;

    if (!to || !from) {
        return str;
    }

    int maxsize = strlen(str) * strlen(to) / strlen(from);
    
    char *res = calloc(sizeof(char), maxsize);
    char *tmp = res, *del = NULL;
    while((del=strstr(str, from))) {
        del[0] = 0;
        tmp += sprintf(tmp, "%s%s", str, to);
        str = del+1;
    }
    sprintf(tmp, "%s", str);
    return res;
}

void print_powerline_full(char *str, json *conf, json *next) {
    if (!str) {
        return;
    }
    json *forground = get(conf->obj, "forground");
    json *background = get(conf->obj, "background");
    
    if (colors) {
        printf("\\[\\033[38;5;%im\\]", forground->number);
        printf("\\[\\033[48;5;%im\\]", background->number);
    }
    printf(" %s ", str);

    if (colors) {
        printf("\\[\\033[38;5;%im\\]", background->number);
        if (next) {
            json *nb = get(next->obj, "background");
            printf("\\[\\e[48;5;%im\\]", nb->number);
        } else {
            printf("\\[\\e[0m\\]");
            printf("\\[\\e[38;5;%im\\]", background->number);
        }
    }

    printf(SOLID_RIGHT);
}

void print_powerline_light(char *str, json *conf) {
    if (!str) {
        return;
    }
    json *forground = get(conf->obj, "background");
    if (colors) {
        printf("\\[\\033[38;5;%im\\]", forground->number);
    }
    printf(" %s ", str);
    printf(LINE_RIGHT);
}

char *optsprocess(char *src, json *opts) {
    if (!opts) {
        return src;
    }
    
    char *name;
    json *nxt;

    map_each(opts->obj, name, nxt) {
        if (!strcmp("match", name)) {
            src = matchreplace(src, nxt);
        }
    }

    return src;
}


void parse_psg(psg *ps) {
    while(ps) {
        json *src = get(ps->conf->obj, "name");
        json *val = get(ps->conf->obj, "default");

        char *srcval = src?src->string:NULL;
        char *defval = val?val->string:NULL;
        ps->parsed = defval;
        
        char *override = get(map, srcval);
        if (override) {
            ps->parsed = override;
        }

        ps->parsed = optsprocess(ps->parsed, get(ps->conf->obj, "options"));

        if (ps->parsed && strlen(ps->parsed) == 0) {
            ps->parsed = NULL;
        }

        ps = ps->next;
    }
}

void print_psg(psg *ps, json *style) {
    json *powerline = get(style->obj, "powerline");
    char *pltype = powerline->string;
    if (!strncmp(pltype, "full", 4)) {
        while(ps) {
            print_powerline_full(ps->parsed
                    ,ps->conf
                    ,ps->next&&ps->next->parsed?ps->next->conf:NULL);
            ps = ps->next;
        }
    } else
    if (!strncmp(pltype, "minimal", 7)) {
        while(ps) {
            print_powerline_light(ps->parsed, ps->conf);
            ps = ps->next;
        }
        printf(" ");
    }
}

void parse_arg(dmap *map, char *arg) {
    char *eq = strstr(arg, "=");
    if (!eq) {
        fprintf(stderr, "error on [%s]; args are in form [name]=[value]", arg);
        exit(1);
    }
    eq[0] = 0;
    put(map, arg, eq+1);
}

void parse_args(dmap *map, int pl, char **argv) {
    if (!strcmp(argv[pl], "--no-colors")) {
        colors = false;
    } else {
        parse_arg(map, argv[pl]);
    }
}

int main(int argc, char **argv) {
    json *each, *j = read_json(streamfile(stdin));
    json *elements = get(j->obj, "elements");
    json *style = get(j->obj, "style");

    if (!elements || !style) {
        puts("cannot find a style or elements block in json");
        return 1;
    }
    
    psg *head = NULL;
    psg *tail = NULL;

    jarray_each(elements, each) {
        psg *p = calloc(sizeof(psg), 1);
        p->next = NULL;
        p->conf = each;
        if (tail) {
            tail->next = p;
            tail = p;
        }
        if (!head) {
            head = p;
            tail = p;
        }
    }

    map = map_new();
    for(int i=1; i<argc; i++) {
        parse_args(map, i, argv);
    }

    parse_psg(head);
    print_psg(head, style);

    if (colors) {
        printf("\\[\\e[0m\\]");
    } else {
        putchar('\n');
    }
}

