/* cgrecv.c defines a default nlcg_receive() function
 * $Log$
 */
#include "nl_cons.h"
#include "globmsg.h"

void nlcg_receive(pid_t who) { reply_byte(who, DAS_UNKN); }
