/* nl_make_name() provides a general-purpose approach to finding other
 * $Log$
 * Revision 1.2  1994/02/16  02:11:08  nort
 * Fixes
 *
 * Revision 1.1  1993/09/15  19:25:31  nort
 * Initial revision
 *
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

char *nl_make_name(const char *base, int global) {
  static char name[NAME_LENGTH+1];
  char *exp, *p;
  int length;
  
  length = strlen(COMPANY) + strlen(base) + 1;
  exp = getenv("Experiment");
  if (exp != NULL) {
	length += strlen(exp) + 1;
  } else global = 0;
  if (global) length++;
  if (length > NAME_LENGTH) {
	if (nl_response)
	  nl_error(nl_response, "Constructed name for %s is too long", base);
	return(NULL);
  }
  p = name;
  if (global) *p++ = '/';
  if (exp == NULL) sprintf(p, "%s/%s", COMPANY, base);
  else sprintf(p, "%s/%s/%s", COMPANY, exp, base);
  return(name);
}
