/* read_cmd.c handles the top level input from a solenoid format file.
   $Log$
   Written March 24, 1987
   Modified July 1991 for QNX.
*/
#include "tokens.h"
#include "solfmt.h"
static char rcsid[] = "$Id$";

void read_cmd(void) {
  int token;
  extern int open_char, close_char, res_num, res_den;

  for (token = get_token(); token != TK_EOF; token = get_token()) {
    switch (token) {
      case TK_SOLENOID:
        read_sol();
        break;
      case TK_OPEN:
      case TK_CLOSE:
        if (get_token() != TK_EQUAL || get_token() != TK_CHAR_CONSTANT)
          filerr("Open/Close Statement Syntax Error\n");
        if (token == TK_OPEN) open_char = gt_number;
        else close_char = gt_number;
        break;
      case TK_RESOLUTION:
        if (get_token() != TK_EQUAL) filerr("Expected '=' after 'resolution'");
        if (get_token() != TK_NUMBER)
          filerr("Expected <number> after 'resolution ='");
        res_num = gt_number;
        if (get_token() != TK_SLASH) filerr("Need Slash in resolution");
        if (get_token() != TK_NUMBER) filerr("Need Number after slash");
        res_den = gt_number;
        break;
      case TK_MODE:
        read_mode();
        break;
      case TK_ROUTINE:
        read_routine();
        break;
      case TK_STATUS_BYTES:
        read_status_addr();
        break;
      case TK_DTOA:
        read_dtoa();
        break;
	  case TK_PROXY:
		read_proxy();
		break;
      default: filerr("Syntax Error in read_cmd\n");
    }
  }
}
