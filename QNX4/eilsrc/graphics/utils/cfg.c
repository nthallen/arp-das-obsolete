#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef __QNX__
#include <lat.h>
#endif

int init_attrs(char *fname, char *attributes, int max) {
  FILE *fp;
  int i, c;

  i = 0;
  fp = fopene(fname, "rb",0);
  if (fp != NULL) {
    for (; i < max; i++) {
      c = getc(fp);
      if (c == EOF) break;
      attributes[i] = c;
    }
    fclose(fp); return(1);
  }
  while (i < max) attributes[i++] = 7;
  return(0);
}


int save_attrs(char *name, char *attributes, int max) {
  FILE *fp;
  int i;
  fp = fopen(name, "wb");
  if (fp == NULL) return(0);
  for (i = 0; i < max; i++) putc(attributes[i], fp);
  fclose(fp);
  return(1);
}
