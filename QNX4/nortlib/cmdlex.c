/* cmdlex.c
 * $Log$
 * Revision 1.1  1992/10/29  05:59:29  nort
 * Initial revision
 *
 */
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include "cmdlex.h"
#include "nortlib.h"
static char rcsid[] = "$Id$";

FILE *yyin;
yyval_t yyval;
#define YYBUFSIZE 200
static char yybuf[YYBUFSIZE];
static unsigned short yyline = 1;
static int yyungotten;

int yygetc(void) {
  int c;

  if (yyungotten) {
	c = yyungotten;
	yyungotten = 0;
  } else {
	c = getc(yyin);
	if (c == '\n') yyline++;
  }
  return(c);
}

yyungetc(int c) { yyungotten = c; }

#define YYSTF_INIT 256
static stuff_buf(int c) {
  static int i;
  
  if (c == YYSTF_INIT) i = 0;
  else if (i == YYBUFSIZE) nl_error(4, "yybuf overflow");
  else if (c == EOF) nl_error(3, "Unexpected EOF");
  else yybuf[i++] = c;
}

static int yylexl(void) {
  int c;
  
  stuff_buf(YYSTF_INIT);
  do c = yygetc(); while (isspace(c));
  if (c == '>') {
	do c = yygetc(); while (isspace(c) && c != '\n');
	while (c != '\n') {
	  stuff_buf(c);
	  c = yygetc();
	}
	stuff_buf(0);
	yyval.string = yybuf;
	return(TK_COMMAND);
  } else if (isdigit(c)) {
	while (isdigit(c)) {
	  stuff_buf(c);
	  c = yygetc();
	}
	yyungetc(c);
	stuff_buf(0);
	yyval.intval = atoi(yybuf);
	return(TK_INT);
  } else if (isalpha(c) || c == '_') {
	while (isalnum(c) || c == '_') {
	  stuff_buf(c);
	  c = yygetc();
	}
	yyungetc(c);
	stuff_buf(0);
	if (stricmp(yybuf, "Mode") == 0) return(KW_MODE);
	else if (stricmp(yybuf, "Initialize") == 0) return(KW_INIT);
	else {
	  yyval.string = yybuf;
	  return(TK_STRING);
	}
  } else return(c);
}

static int yyunlexed = 0, yylasttok;

int yylex(void) {
  if (yyunlexed) yyunlexed = 0;
  else yylasttok = yylexl();
  return(yylasttok);
}

void yyunlex(void) { yyunlexed = 1; }

void yyseek(long int pos) {
  fseek(yyin, pos, 0);
  yyunlexed = 0;
}

void expecting(int token) {
  char *s, buf[5];
  
  if (yylex() != token) {
	switch (token) {
	  case KW_MODE: s = "\"Mode\""; break;
	  case KW_INIT: s = "\"Initialize\""; break;
	  case TK_STRING: s = "a String"; break;
	  case TK_COMMAND: s = "a Command"; break;
	  case TK_INT: s = "an integer"; break;
	  default: sprintf(buf, "'%c'", token); s = buf; break;
	}
	nl_error(3, "Syntax error: expecting %s", s);
  }
}

/* clex_error holds the previous value of nl_error while we're
   reading in the algorithm for the first time. nl_error is
   pointed to clex_err in order to add line-number information
   to the error list.
*/
int (* clex_error)(unsigned int level, char *s, ...);
int clex_err(unsigned int level, char *s, ...) {
  char buf[160], *p;
  va_list arg;
  
  va_start(arg, s);
  sprintf(buf, "%u: ", yyline);
  p = buf + strlen(buf);
  vsprintf(p, s, arg);
  va_end(arg);
  clex_error(level, "%s", buf);
  return(level);
}
