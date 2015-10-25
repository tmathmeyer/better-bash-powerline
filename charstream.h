



#ifndef _CHARSTREAM_H_
#define _CHARSTREAM_H_


#define ASSERT(cond, ret) if (!(cond)) { return ret; }


typedef
struct {
    unsigned int type; // 0 is (char *), 1 is (FILE *)
    unsigned int done;
    FILE *ptr;
    char **str;
    char buff;
} charstream;

char next(charstream *cs);
char seq(charstream *cs);

void skip(charstream *cs, unsigned int nt);

charstream *streamfile(FILE *ptr);
charstream *streamstr(char *str);

#endif
