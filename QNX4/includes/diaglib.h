/* diaglib.h Include file for Diagnostic Library
 * $Log$
 * $Id$
 */

void check_rack(unsigned short address);
void check_wack(unsigned short address, unsigned short oval);
void check_ack(unsigned short address, unsigned short oval);
void check_rw(unsigned short address, unsigned short omask);
extern int dl_skip_checks;
void diag_error(char *fmt, ...);
