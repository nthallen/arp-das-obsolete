/* cmdlex.h defines interface to lexical analyzer for cmd algorithms.
 * $Log$
 */
#ifndef CMDLEX_H_INCLUDED
#define CMDLEX_H_INCLUDED
#include <stdio.h>

extern FILE *yyin;
int yylex(void);
void yyunlex(void);
void yyseek(long int);
void expecting(int token);
extern int (* clex_error)(unsigned int level, char *s, ...);
int clex_err(unsigned int level, char *s, ...);

typedef union {
  short intval;
  char *string;
} yyval_t;
extern yyval_t yyval;

#define KW_MODE 256
#define KW_INIT 257
#define TK_STRING 258
#define TK_COMMAND 259
#define TK_INT 260
#endif
