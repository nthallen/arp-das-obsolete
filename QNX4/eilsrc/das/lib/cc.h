
#ifndef _CC_INIT_H_INCLUDED
#define _CC_INIT_H_INCLUDED

#include <cmdctrl.h>

#define OPT_CC_INIT "DRSTO"

/* returns pid of cmdctrl if connects, else 0 */
int cc_init(unsigned char min_dasc, unsigned char max_dasc,
	    unsigned char min_msg, unsigned char max_msg,
	    quit_type how_to_quit, death_type how_to_die,
	    char *task_start);

/* returns pid of cmdctrl if connects, else 0 */
int cc_init_options(int argcc, char *argvv[], unsigned char min_dasc,
		    unsigned char max_dasc, unsigned char min_msg,
		    unsigned char max_msg, quit_type how_to_quit);

#endif
