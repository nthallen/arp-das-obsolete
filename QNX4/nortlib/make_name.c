/* nl_make_name() provides a general-purpose approach to finding other
 */
#include <stdlib.h>
#include <string.h>
#include "company.h"
#include "nortlib.h"
char rcsid_make_name_c[] =
  "$Header$";

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
/*
=Name nl_make_name(): Build a standard name for qnx_name_locate()
=Subject Nortlib
=Synopsis

#include "nortlib.h"
char *nl_make_name(const char *base, int global);

=Description

  nl_make_name() returns a string of the form: huarp/exp/base
  where exp is the current value of the
  environment variable "Experiment" and base is the input
  argument string. If global is non-zero, the string gets a
  leading '/'. This is the standard means of building names
  within the ARP Data Acquisition System architecture which will
  be used with qnx_name_locate() and qnx_name_attach().<P>

=Returns

  A pointer to a static buffer containing the expanded name.
  You must save the string if it is needed for long.

=SeeAlso

  =Nortlib= functions.

=End
*/
