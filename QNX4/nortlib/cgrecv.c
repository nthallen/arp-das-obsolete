/* cgrecv.c defines a default nlcg_receive() function
 * $Log$
 * Revision 1.2  1993/07/01  15:35:04  nort
 * Eliminated "unreferenced" via Watcom pragma
 *
 * Revision 1.1  1993/02/18  02:29:45  nort
 * Initial revision
 *
 */
#include "nl_cons.h"
#include "globmsg.h"
char rcsid_cgrecv_c[] =
  "$Header$";

void nlcg_receive(pid_t who) { reply_byte(who, DAS_UNKN); }

/*
=Name nlcg_receive(): Receive hook for =nlcon_getch=().
=Subject Nortlib Console Functions
=Synopsis
#include "nl_cons.h"

void nlcg_receive(pid_t who);

=Description

This function is called when =nlcon_getch=() Receives an unrecognized
message. The default behaviour is to return DAS_UNKN, but the
library function may be overridden by a user function of the same
name.<P>

When Receiving, nlcon_getch() is only interested in the PID of
the sender, since it is receiving empty proxies. nlcg_receive()
may extract content from a message by calling Readmsg().<P>

nlcg_receive() is responsible for replying to all messages for
which it is called.

=SeeAlso
=nlcon_getch=().
=End
*/
