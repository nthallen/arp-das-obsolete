/*
    Configure serial ports with default values.
*/
#include <termios.h>
#include <serial.h>
#include <msg.h>

/* returns 0 on success */
int serial_init(int fd) {
struct termios termv;

    if (tcgetattr(fd, &termv) == -1)
	msg(MSG_EXIT_ABNORM,"Can't get terminal control settings for descriptor %d",fd);
    termv.c_iflag &= ~(IXOFF);	/* no software flow control */
    termv.c_iflag &= ~(IXON);	/* no software flow control */
    termv.c_lflag &= ~(IEXTEN);	/* no QNX extensions */
    termv.c_cflag &= ~(OHFLOW);	/* no hardware flow control */
    termv.c_cflag &= ~(IHFLOW);	/* no hardware flow control */
    termv.c_iflag &= ~(IGNCR);	/* don't ignore CR */
    termv.c_iflag |= (IGNBRK);	/* ignore Break */
    termv.c_lflag &= ~(ECHO);	/* no input echoeing */
    termv.c_lflag &= ~(ICANON);	/* no input editing */
    termv.c_oflag &= ~(OPOST);	/* no output processing */
    termv.c_cflag |= (CS8);	/* 8 bits per char */
    termv.c_cflag &= ~(PARENB);	/* no parity */
    termv.c_cflag &= ~(CSTOPB);	/* 1 stop bit */
    termv.c_ospeed = 9600;	/* baud =  9600 */
    termv.c_ispeed = 9600;	/* baud =  9600 */

    if (tcsetattr(fd,TCSAFLUSH,&termv) == -1)
     	msg(MSG_EXIT_ABNORM,"Can't set terminal control settings for descriptor %d",fd);

    return(0);
}
