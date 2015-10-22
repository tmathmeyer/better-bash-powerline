
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

static char *cwd = NULL;
static char *usr = NULL;
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

char *cwd_module() {
    char *result = NULL;
    if (get(map, "path")) {
        result = cwd;
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
        return usr;
    }
    return strdup(getenv("USER"));
}

char *git_module() {
	char * path = malloc(256);
	memset(path, 0, 256);
	snprintf(path, 256, "/bin/bash %s/.config/psg/git.sh 2>/dev/null", getenv("HOME"));
	FILE * git_info = popen(path, "r");
	if (git_info == NULL) {
		return NULL;
	}

	char * branch = malloc(64);
	int pos = 0; char c = 0;
	while((c = fgetc(git_info)) != '\n' && c != EOF && pos < 64) {
		if (c != '\\') {
			branch[pos++] = c;
		} else {
			char n = fgetc(git_info);
			if (n == 'e') {
				branch[pos++] = '\e';
			}
		}
	}
	fclose(git_info);
	return pos<14?NULL:branch;
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
            ps->parsed = cwd_module();
        } else if (!(strcmp("git", src->string))) {
            ps->parsed = git_module();
        } else {
            ps->parsed = gen_module(src->string);
        }


        ps = ps->next;
    }
}

void print_psg(psg *ps) {
    while(ps) {
        json *src = get(ps->conf->obj, "source");
        if (!(strcmp("jobs", src->string))) {
            print(proc_module(), ps->conf, ps->next&&ps->next->parsed?ps->next->conf:NULL);
        } else if (!(strcmp("user", src->string))) {
            print(user_module(), ps->conf, ps->next&&ps->next->parsed?ps->next->conf:NULL);
        } else if (!(strcmp("path", src->string))) {
            print(cwd_module(), ps->conf, ps->next&&ps->next->parsed?ps->next->conf:NULL);
        } else if (!(strcmp("git", src->string))) {
            print(git_module(), ps->conf, ps->next&&ps->next->parsed?ps->next->conf:NULL);
        } else {
            json *src = get(ps->conf->obj, "source");
            puts(src->string);
        }

        ps = ps->next;
    }
}

int main() {

    FILE *fp;
    long lSize;
    char *buffer, *backup;


    fp = fopen("sample.json", "r");
    if (!fp) {
        perror("cannot read file");
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

    //TODO parse

    parse_psg(head);
    print_psg(head);

    printf("\\[\\e[0m\\]");
    fclose(fp);
    free(backup);
}


