/* dacoff.c Defines the high-level interface to the object file output.
   Written May 21, 1991 by NTA
*/
#include <stdio.h>
#include <assert.h>
#include "dac.h"

static FILE *ofp = NULL;
static unsigned int rec_active = 0, rec_count, rec_sum;

/* Returns TRUE if the specified file cannot be opened */
int of_open(char *name) {
  assert(ofp == NULL);
  ofp = fopen(name, "w+b"); 
  return(ofp == NULL);
}

void of_close(void) {
  assert(ofp != NULL);
  fclose(ofp);
  ofp = NULL;
}

void of_rec_beg(unsigned int rectype) {
  assert(ofp != NULL);
  assert(rec_active == 0);
  rec_active = 1;
  rec_sum = rec_count = 0;
  fputc(rectype, ofp);
  fwrite(&rec_count, 2, 1, ofp);
}

void of_rec_data(unsigned char *buf, unsigned int n_bytes) {
  unsigned int i;

  for (i = 0; i < n_bytes; i++) rec_sum += buf[i];
  fwrite(buf, 1, n_bytes, ofp);
  rec_count += n_bytes;
}

void of_rec_end(void) {
  fputc(rec_sum, ofp);
  fseek(ofp, -(long)(rec_count+3), SEEK_CUR);
  fwrite(&rec_count, 2, 1, ofp);
  fseek(ofp, 0L, SEEK_END);
  rec_active = 0;
}
