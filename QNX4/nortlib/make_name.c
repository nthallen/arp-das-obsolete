/* nl_make_name() provides a general-purpose approach to finding other
 * $Log$
 */
#include <stdlib.h>
#include <string.h>
#include "company.h"
#include "nortlib.h"
#pragma off (unreferenced)
  static char rcsid[] =
	"$Id$";
#pragma on (unreferenced)

#define NAME_LENGTH 32

char *nl_make_name(char *base) {
  static char name[NAME_LENGTH+1];
  char *exp;
  int length;
  
  length = strlen(COMPANY) + strlen(base) + 1;
  exp = getenv("Experiment");
  if (exp != NULL) {
	length += strlen(exp) + 1;
  }
  if (length > NAME_LENGTH) {
	if (nl_response)
	  nl_error(nl_response, "Constructed name for %s is too long", base);
	return(NULL);
  }
  if (exp == NULL) sprintf(name, "%s/%s", COMPANY, base);
  else sprintf(name, "%s/%s/%s", COMPANY, exp, base);
  return(name);
}
