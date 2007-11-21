/* attribut.c handles attribute configuration.
   Written December 6, 1989
*/
#include <stdio.h>
#ifdef __QNX__
#include <lat.h>
#endif
#include "attribut.h"

unsigned char attributes[MAX_ATTRS];
void init_attrs(char *fname) {
  FILE *fp;
  char name[65];
  int i, c;

  i = 0;
  fp = fopene(fname, "rb", name);
  if (fp != NULL) {
    for (; i < MAX_ATTRS; i++) {
      c = getc(fp);
      if (c == EOF) break;
      attributes[i] = c;
    }
    fclose(fp);
  }
  while (i < MAX_ATTRS) attributes[i++] = 7;
}
