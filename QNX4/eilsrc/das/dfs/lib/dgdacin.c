/* dgdacin.c Contains DG_dac_in()
 $Log$
 * Revision 1.7  1993/01/22  20:46:04  eil
 * rid opt_string
 *
 * Revision 1.6  1992/08/20  15:45:03  nort
 * Change fread to return number of bytes, not number of units.
 *
 * Revision 1.5  1992/08/19  20:32:10  nort
 * Changed to no longer use DAC record format.
 *
 * Revision 1.4  1992/07/16  14:52:44  eil
 * defaulting tm dac filename
 *
 * Revision 1.3  1992/06/09  14:46:07  eil
 * tm dac file defaults to tm.dac
 *
 * Revision 1.2  1992/05/22  19:03:43  eil
 * rid nrowsec.
 *
 * Revision 1.1  1992/05/20  17:26:27  nort
 * Initial revision
 *
 */
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <dbr.h>
#include <eillib.h>

#define TMDBASEFILE "tm.dac"

extern char *opt_string;

/* Aborts on error. */
int DG_dac_in(int argcc, char **argvv) {
  char *filename = NULL;
  int c, rv;
  FILE *fp;

  opterr = 0;
  optind = 0;

  do {
	c=getopt(argcc,argvv,opt_string);
	switch (c) {
	  case 'f': filename = optarg; break;
	  case '?':
		msg(MSG_EXIT_ABNORM, "Invalid option -%c", optopt);
	  default : break;
	}
  } while (c != -1);
  opterr = 1;

  if (filename == NULL) filename = TMDBASEFILE;
  fp = fopen(filename, "rb");
  if (fp == NULL)
    msg(MSG_EXIT_ABNORM,"Can't open dac file %s",filename);
  rv = fread(&dbr_info.tm, 1, sizeof(tm_info_type), fp);
  fclose(fp);
  if (rv != sizeof(tm_info_type))
	msg(MSG_EXIT_ABNORM, "Unable to read %d bytes from dac file %s",
		  sizeof(tm_info_type), filename);
  /* Perform sanity checks: */
  if (tmi(nbminf) == 0 ||
	  tmi(nbrow) == 0 ||
	  tmi(nrowmajf) == 0 ||
	  tmi(nrowsper) == 0 ||
	  tmi(nsecsper) == 0 ||
	  tmi(mfc_lsb) == tmi(mfc_msb) ||
	  tmi(mfc_lsb) >= tmi(nbrow) ||
	  tmi(mfc_msb) >= tmi(nbrow) ||
	  tmi(nbminf) < tmi(nbrow) ||
	  tmi(nbminf) % tmi(nbrow) != 0)
	msg(MSG_EXIT_ABNORM,"Sanity checks failed in DG_dac_in");
  return(0);
}
