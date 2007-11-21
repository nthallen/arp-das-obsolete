/* diaglib.h Include file for Diagnostic Library
 * $Log$
 * Revision 1.1  1992/08/20  21:00:59  nort
 * Initial revision
 *
 * $Id$
 */

void check_rack(unsigned short address);
void check_wack(unsigned short address, unsigned short oval);
void check_ack(unsigned short address, unsigned short oval);
void check_rw(unsigned short address, unsigned short omask);
void check_nack( unsigned short addr );
void check_read( unsigned short addr );
extern int dl_skip_checks;
void diag_error(char *fmt, ...);
