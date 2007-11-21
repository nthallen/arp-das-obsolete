
#ifndef _CC_INIT_H_INCLUDED
#define _CC_INIT_H_INCLUDED
#ifdef __cplusplus
extern "C" {
#endif

#include <sys/types.h>
#include "cmdctrl.h"

#define OPT_CC_INIT "DRSTO"

/* returns pid of cmdctrl if connects, else 0 */
int cc_init(unsigned char min_dasc, unsigned char max_dasc,
	    unsigned char min_msg, unsigned char max_msg,
	    quit_type how_to_quit, death_type how_to_die,
	    pid_t prox,
	    char *task_start);

/* returns pid of cmdctrl if connects, else 0 */
int cc_init_options(int argcc, char *argvv[], unsigned char min_dasc,
		    unsigned char max_dasc, unsigned char min_msg,
		    unsigned char max_msg, quit_type how_to_quit, ...);

#ifdef __cplusplus
};
#endif

#if defined __386__
#  pragma library (das3r)
#elif defined __SMALL__
#  pragma library (dass)
#elif defined __COMPACT__
#  pragma library (dasc)
#elif defined __MEDIUM__
#  pragma library (dasm)
#elif defined __LARGE__
#  pragma library (dasl)
# endif

#endif

