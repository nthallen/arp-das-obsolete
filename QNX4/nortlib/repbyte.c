/* repbyte.c Provides trivial reply_byte() function.
 * Always returns 0. Could be augmented to check success...
 */
#include <sys/kernel.h>
#include "nortlib.h"
char rcsid_repbyte_c[] =
  "$Header$";

int reply_byte(pid_t sent_tid, unsigned char msg) {
  Reply(sent_tid, &msg, 1);
  return(0);
}
