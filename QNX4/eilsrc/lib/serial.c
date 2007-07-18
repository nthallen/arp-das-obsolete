/*
    Configure serial ports with default values.
*/
#include <termios.h>
#include <serial.h>
#include <unistd.h>
#include "msg.h"

/* returns 0 on success */
int serial_init(int fd) {
  struct termios termv;
  long dis;

  /* Note: Hardware flow control is by default locked on, although I think it
     can still be turned off by special flag, but not here */

  if (tcgetattr(fd, &termv) == -1)
    msg(MSG_EXIT_ABNORM,"Can't get terminal control settings for descriptor %d",fd);
  termv.c_iflag &= ~(IXOFF);	/* no software flow control */
  termv.c_iflag &= ~(IXON);	/* no software flow control */
  termv.c_iflag &= ~(INPCK);	/* no input parity check */
  termv.c_iflag &= ~(PARMRK);	/* no mark parity errors in input stream */
  termv.c_lflag &= ~(IEXTEN);	/* no QNX extensions */
  termv.c_cflag &= ~(OHFLOW);	/* no hardware flow control */
  termv.c_cflag &= ~(IHFLOW);	/* no hardware flow control */
  termv.c_iflag &= ~(IGNCR);	/* don't ignore CR */
  termv.c_iflag &= ~(ICRNL);	/* don't map CR to NL on input */
  termv.c_iflag &= ~(INLCR);	/* don't map NL to CR on input */
  termv.c_iflag |= (IGNBRK);	/* ignore Break */
  termv.c_lflag &= ~(ECHO);	/* no input echoeing */
  termv.c_lflag &= ~(ECHOE);	/* no input echoeing */
  termv.c_lflag &= ~(ECHOK);	/* no input echoeing */
  termv.c_lflag &= ~(ECHONL);	/* no echo NL even if echo is off */
  termv.c_lflag &= ~(ISIG);	/* no input echoeing */
  termv.c_lflag &= ~(ICANON);	/* no input editing */
  termv.c_oflag &= ~(OPOST);	/* no output processing */
  termv.c_cflag |= (CS8);	/* 8 bits per char */
  termv.c_cflag &= ~(PARENB);	/* no parity */
  termv.c_cflag &= ~(CSTOPB);	/* 1 stop bit */
  termv.c_ospeed = 9600;	/* baud =  9600 */
  termv.c_ispeed = 9600;	/* baud =  9600 */

  dis = fpathconf(fd,_PC_VDISABLE);
  if (dis != -1L) {
    termv.c_cc[VINTR] = dis;
    termv.c_cc[VQUIT] = dis;
    termv.c_cc[VERASE] = dis;
    termv.c_cc[VKILL] = dis;
    termv.c_cc[VEOF] = dis;
    termv.c_cc[VEOL] = dis;
    termv.c_cc[VSUSP] = dis;
  }

  if (tcsetattr(fd,TCSAFLUSH,&termv) == -1)
    msg(MSG_EXIT_ABNORM,"Can't set terminal control settings for descriptor %d",fd);

  return(0);
}
