/* cgrecv.c defines a default nlcg_receive() function
 * $Log$
 * Revision 1.1  1993/02/18  02:29:45  nort
 * Initial revision
 *
 */
#include "nl_cons.h"
#include "globmsg.h"
#ifdef __WATCOMC__
  #pragma off (unreferenced)
	static char rcsid[] =
	  "$Id$";
  #pragma on (unreferenced)
#endif

void nlcg_receive(pid_t who) { reply_byte(who, DAS_UNKN); }
