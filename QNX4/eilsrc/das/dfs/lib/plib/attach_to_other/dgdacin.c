/* dgdacin.c Contains DG_dac_in()
 $Log$
 * Revision 2.1  1994/12/01  21:07:19  eil
 * dfs
 *
 */
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <dfs.h>
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
