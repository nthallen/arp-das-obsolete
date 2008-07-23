/* ringtap.c monitors ring performance and reports contents.
   $Log$
 * Revision 1.3  1996/03/21  14:24:48  nort
 * *** empty log message ***
 *
 * Revision 1.2  1993/09/15  19:29:13  nort
 * *** empty log message ***
 *
 * Revision 1.1  1992/08/07  13:43:30  nort
 * Initial revision
 *
 * Begun June 10, 1992
*/
#include <stdio.h>
#include <time.h>
#include "dbr.h"
#include "msg.h"
#pragma off (unreferenced)
  static char rcsid[] =
	"$Id$";
#pragma on (unreferenced)

char *opt_string = OPT_MSG_INIT OPT_DC_INIT;

unsigned int minf_row;

void main(int argc, char **argv) {
  msg_init_options("TAP", argc, argv);
  DC_init_options(argc, argv);
  msg(MSG, "  TMID is \"%s\"", tmi(tmid).ident);
  msg(MSG, "  %d rows, %d cols", tmi(nrowmajf), tmi(nbrow));
  msg(MSG, "  %d rows/minor frame", dbr_info.nrowminf);
  minf_row = 0;
  msg(MSG, "  %d/%d rows/sec", tmi(nrowsper), tmi(nsecsper));
  msg(MSG, "  Synch %04X%s", tmi(synch), tmi(isflag) ? " Inverted" : "");
  BEGIN_MSG;
  DC_operate();
  DONE_MSG;
}

void DC_data(dbr_data_type *dr_data) {
  unsigned int i, j, k, l, u;
  union {
	unsigned char b[2];
	unsigned int mfc;
  } mfc;
  char buf[256], *p;
  
  msg(MSG, "Data: %d rows", dr_data->n_rows);
  for (i = k = 0; i < dr_data->n_rows; i++) {
	if (minf_row == 0) {
	  mfc.b[0] = dr_data->data[k+tmi(mfc_lsb)];
	  mfc.b[1] = dr_data->data[k+tmi(mfc_msb)];
	  msg(MSG, "MFC %u", mfc.mfc);
	}
	for (j = 0, p = buf; j < tmi(nbrow); j++, k++) {
	  *p++ = ' ';
	  l = dr_data->data[k];
	  u = l >> 4;
	  l &= 0xF;
	  *p++ = u + (u >= 10 ? 'A' - 10 : '0');
	  *p++ = l + (l >= 10 ? 'A' - 10 : '0');
	}
	*p = '\0';
	msg(MSG, buf);
	if (++minf_row == dbr_info.nrowminf) minf_row = 0;
  }
}

void DC_tstamp(tstamp_type *tstamp) {
  char *ttext;
  
  ttext = ctime((signed long *)&tstamp->secs);
  ttext[24] = '\0';
  msg(MSG, "TStamp: %ld,%u = %s", tstamp->secs, tstamp->mfc_num, ttext);
}

void DC_DASCmd(unsigned char type, unsigned char number) {
  msg(MSG, "DASCmd: %d %d", type, number);
}

void DC_other(unsigned char *msg_ptr, int sent_tid) {
  msg(MSG_EXIT_ABNORM, "other: Type %d from %u", *msg_ptr, sent_tid);
}
