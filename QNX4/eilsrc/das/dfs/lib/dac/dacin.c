/* dacin.c provides routines for reading in a .dac file.
   Written April 24, 1991
   Modified Jan 10 1992, bug fix.
*/
#include <stdio.h>
#include <assert.h>
#include "dac.h"

static FILE *dacfp = NULL;
static unsigned char checksum;
static unsigned long int nbrec, nbfile;
static unsigned int in_rec;

/* returns TRUE if the file is not found */
int dac_open(char *filename) {
  assert(dacfp == NULL);
  dacfp = fopen(filename, "rb");
  in_rec = 0;
  if (dacfp) {
    fseek(dacfp, 0L, SEEK_END);
    nbfile = ftell(dacfp);
    rewind(dacfp); /* 1/10/92 */
  }
  return(dacfp == NULL);
}

void dac_close(void) {
  assert(dacfp != NULL);
  fclose(dacfp);
  dacfp = NULL;
}

/* skips the previous record if necessary.
   Returns EOF on EOF, else returns the record type
   (which is between 0 and 255).
*/
int dac_next_rec(void) {
  int rectype;
  unsigned int n_bytes, br;

  assert(dacfp != NULL);
  if (in_rec) {
    in_rec = 0;
    if (nbfile > nbrec) fseek(dacfp, nbrec+1, SEEK_CUR);
    else return(EOF);
  }

  if (nbfile >= 4) {
    rectype = getc(dacfp);
    assert(rectype != EOF);
    br = fread(&n_bytes, 2, 1, dacfp);
    assert(br == 1); /* 1/10/92 */
    nbfile -= 3;
    nbrec = n_bytes;
    assert(nbfile > nbrec);
    in_rec = 1;
    checksum = 0;
  } else rectype = EOF;

  return rectype;
}

/* returns the number of bytes of data left in the current record.
   If no records have been read, returns 0.
*/
unsigned int dac_rec_size(void) {
  assert(dacfp != NULL);
  return(in_rec ? (unsigned int) nbrec : 0);
}

/* returns the number of bytes read. If the record size exceeds
   the maxsize specified or if there is a read error, a
   zero value is returned.
*/
unsigned int dac_rec(void *buf, unsigned int maxsize) {
  if (!in_rec || nbrec > maxsize) return 0;
  return dac_read(buf, (unsigned int)nbrec);
}

/* returns the number of bytes read. Returns 0 at the end of
   the record.
*/
unsigned int dac_read(void *buf, unsigned int n_bytes) {
  unsigned int nb;
  char *bp;

  assert(dacfp != NULL);
  assert(buf != NULL);
  if (n_bytes > nbrec) n_bytes = nbrec;
  nb = fread(buf, 1, n_bytes, dacfp); /* 1/13/92 */
  assert(nb == n_bytes);
  for (bp = buf; nb > 0; nb--) checksum += *bp++;
  nbfile -= n_bytes;
  nbrec -= n_bytes;
  return n_bytes;
}

/* reads the remainder of the current record and returns TRUE if
   the checksum doesn't match.
*/
int dac_checksum(void) {
  int c;
  unsigned int i;

  assert(dacfp != NULL);
  if (!in_rec) return 0;
  for (i = nbrec; i > 0; i--) {
    c = getc(dacfp);
    assert(c != EOF);
    checksum += c;
  }
  c = getc(dacfp);
  nbfile -= nbrec + 1;
  in_rec = 0;
  return(c != checksum);
}

