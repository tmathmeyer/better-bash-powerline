
#define _DEFAULT_SOURCE

#include <sys/types.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "json.h"

#define SOLID_RIGHT ""
#define LINE_RIGHT ""
#define SOLID_LEFT ""
#define LINE_LEFT ""

typedef
struct psg {
    char *parsed;
    json *conf;
    struct psg *next;
} psg;

static dmap *map = NULL;

char *proc_module() {
    pid_t ppid = getppid();
    char *cmd = malloc(40);
    snprintf(cmd, 40, "cat /proc/%i/status | grep PPid", ppid);
    FILE *fp = popen(cmd, "r");
    if (fp == NULL) {
        return NULL;
    }
    int realppid;
    fscanf(fp, "PPid: %i", &realppid);
    memset(cmd, 0, 40);
    snprintf(cmd, 40, "ps h --ppid %i -o pid | wc -l", realppid);
    fclose(fp);
    fp = popen(cmd, "r");
    if (fp == NULL) {
        return NULL;
    }
    int i;
    fscanf(fp, "%i", &i);
    fclose(fp);
    memset(cmd, 0, 40);
    i-=1;
    if (i <= 0) {
        free(cmd);
        return NULL;
    }
    return itoaa(i);
}

char *cwd_module(json *opts) {
    char *result = NULL;
    if (get(map, "path")) {
        result = get(map, "path");
    } else {
        char *cwd = malloc(1024);
        if (getcwd(cwd, 1024) != NULL) {
            char *home = getenv("HOME");
            int home_len = strlen(home);
            char *pos = strstr(cwd, home);
            if (pos == NULL) {
                result = cwd;
            } else {
                cwd += home_len-1;
                cwd[0] = '~';
                result = cwd;
            }
        }
    }

    json *delimit = get(opts->obj, "delimit");
    if (strcmp("powerline", delimit->string)) {
        return result;
    }

    if (result) {
        char *res = calloc(sizeof(char), strlen(result)*2);
        char *tmp = res, *del = NULL;
        while((del=strstr(result, "/"))) {
            del[0] = 0;
            tmp += sprintf(tmp, "%s " LINE_RIGHT " ", result);
            result = del+1;
        }
        sprintf(tmp, "%s", result);
        return res;
    }
    return NULL;
}

char *user_module() {
    if (get(map, "user")) {
        return get(map, "user");
    }
    return strdup(getenv("USER"));
}

char *git_module(dmap *bl) {
    char *path = malloc(256);
    memset(path, 0, 256);
    snprintf(path, 256, "/bin/bash %s/.config/psg/git.sh 2>/dev/null", getenv("HOME"));
    FILE *git_info = popen(path, "r");
    if (git_info == NULL) {
        return NULL;
    }

    char *branch = calloc(sizeof(char), 64);
    int pos = 0; char c = 0;
    while((c = fgetc(git_info)) != '\n' && c != EOF && pos < 64) {
        branch[pos++] = c;
    }
    fclose(git_info);
    if (pos < 1) {
        return NULL;
    }

    if (strstr(branch, ":")) {
        json *opts = get(bl, "options");
        if (opts) {
            dmap *omap = opts->obj;
            
            json *cfg = get(omap, "changes_forground");
            json *cbg = get(omap, "changes_background");

            if (cfg) {
                put(bl, "forground", cfg);
            }

            if (cbg) {
                put(bl, "background", cbg);
            }
        }
    }

    return branch;
}

void print(char *str, json *conf, json *next) {
    if (!str) {
        return;
    }
    json *forground = get(conf->obj, "forground");
    json *background = get(conf->obj, "background");

    printf("\\[\\033[38;5;%im\\]", forground->number);
    printf("\\[\\033[48;5;%im\\]", background->number);
    printf(" %s ", str);

    printf("\\[\\033[38;5;%im\\]", background->number);
    if (next) {
        json *nb = get(next->obj, "background");
        printf("\\[\\e[48;5;%im\\]", nb->number);
    } else {
        printf("\\[\\e[0m\\]");
        printf("\\[\\e[38;5;%im\\]", background->number);
    }

    printf(SOLID_RIGHT);
}

char *gen_module(char *src) {
    return get(map, src);
}

void parse_psg(psg *ps) {
    while(ps) {
        json *src = get(ps->conf->obj, "source");
        if (!(strcmp("jobs", src->string))) {
            ps->parsed = proc_module();
        } else if (!(strcmp("user", src->string))) {
            ps->parsed = user_module();
        } else if (!(strcmp("path", src->string))) {
            ps->parsed = cwd_module(get(ps->conf->obj, "options"));
        } else if (!(strcmp("git", src->string))) {
            ps->parsed = git_module(ps->conf->obj);
        } else {
            ps->parsed = gen_module(src->string);
        }
        ps = ps->next;
    }
}

void print_psg(psg *ps) {
    while(ps) {
        print(ps->parsed, ps->conf, ps->next&&ps->next->parsed?ps->next->conf:NULL);
        ps = ps->next;
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
    // TODO: real flags check here
    parse_arg(map, argv[pl]);
}

int main(int argc, char **argv) {

    FILE *fp;
    long lSize;
    char *buffer, *backup;


    char *path = calloc(sizeof(char), 256);
    memset(path, 0, 256);
    snprintf(path, 256, "%s/.config/psg/psg.json", getenv("HOME"));

    fp = fopen(path, "r");
    if (!fp) {
        perror("cannot read file");
        perror(path);
        exit(1);
    }

    fseek(fp, 0L, SEEK_END);
    lSize = ftell(fp);
    rewind(fp);

    buffer = calloc(sizeof(char), lSize+1);
    if(!buffer) {
        fclose(fp);
        perror("memory alloc fails");
        exit(1);
    }

    if(fread(buffer, lSize, 1, fp) != 1) {
        fclose(fp);
        free(buffer);
        perror("entire read fails");
        exit(1);
    }

    backup = buffer;
    json *each, *j = read_json(&buffer);
    psg *head = NULL;
    psg *tail = NULL;

    jarray_each(j, each) {
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
    print_psg(head);

    printf("\\[\\e[0m\\]");
    fclose(fp);
    free(backup);
}

