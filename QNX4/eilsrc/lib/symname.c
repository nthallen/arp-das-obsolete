/*
    constructs symbolic names.
*/

#include <stdlib.h>
#include <string.h>
#include <symname.h>
#include <msg.h>

#define NAME_LENGTH 32

char *symname( char *company, char *base, int globalflag, char *env_varname) {
static char name[NAME_LENGTH+1];
char *exp;
int length;
char glob[2] = {'\0'};

  length = strlen(company) + strlen(base) + 1;
  exp = getenv(env_varname);
  if (exp != NULL) {
	length += strlen(exp) + 1;
  } else globalflag = 0;
  if (globalflag) length++;
  if (length > NAME_LENGTH)
	  msg(MSG_EXIT_ABNORM, "Constructed name for %s is too long", base);

  if (globalflag) strcpy(glob,"/");
  if (exp == NULL) sprintf(name, "%s%s/%s", glob, company, base);
  else sprintf(name, "%s%s/%s/%s", glob, company, exp, base);
  return(name);
}
