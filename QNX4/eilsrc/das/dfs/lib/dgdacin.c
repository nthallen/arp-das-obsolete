/* dgdacin.c Contains DG_dac_in()
 $Log$
 */
#include <stdio.h>
#include <unistd.h>
#include "dbr_utils.h"
#include "msg.h"

/* returns a non-zero value if there are errors reading from the
   dac file.
     0 Everything was fine
     2 Header record size was incorrect
	 3 Checksum was invalid
	 4 No header was found in the dac file
*/
int DG_dac_in(int argcc, char **argvv) {
  char *filename;
  int c, rv;

  /* error handling intialisation if the client code didnt */
  if (!msg_initialised())
  msg_init(DG_NAME,0,1,0,0,1);

  opterr = 0;
  optind = 0;

  filename = NULL;
  do {
	c=getopt(argcc,argvv,opt_string);
	switch (c) {
	  case 'f': filename = optarg; break;
	  case '?':
		msg(MSG_EXIT_ABNORM, "Invalid option -%c", optopt);
	  default : break;
	}
  } while (c != -1);
  optind = 0;
  opterr = 1;

  if (filename == NULL)
	msg(MSG_EXIT_ABNORM,"Must specify dac file");
  if (dac_open(filename))
    msg(MSG_EXIT_ABNORM,"Can't open dac file %s",filename);

  for (;;) {
    switch (dac_next_rec()) {
      case OFF_HDR:
        rv = dac_rec(&dbr_info.tm, sizeof(tm_info_type));
        if (rv != sizeof(tm_info_type)) rv = 2;
        else rv = 0;
        if (dac_checksum()) rv = 3;
        dac_close();
        if (rv) return rv;
        break;
      case EOF:
        dac_close();
        return 4;
      default:
        continue;
    }
    break;
  }

  /* Perform sanity checks: */
  if (tmi(nbminf) == 0 ||
      tmi(nbrow) == 0 ||
      tmi(nrowmajf) == 0 ||
      tmi(nrowsec) == 0 ||
      tmi(mfc_lsb) == tmi(mfc_msb) ||
      tmi(mfc_lsb) >= tmi(nbrow) ||
      tmi(mfc_msb) >= tmi(nbrow) ||
      tmi(nbminf) < tmi(nbrow) ||
      tmi(nbminf) % tmi(nbrow) != 0)
	msg(MSG_EXIT_ABNORM,"Sanity checks failed in DG_dac_in");

  return(0);
}
