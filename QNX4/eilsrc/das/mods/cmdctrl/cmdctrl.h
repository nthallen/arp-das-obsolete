/*                     
 * Include file for cmdctrl.c
 * Structures and Enumerated types. 
 * Written by David Stahl. 1991
 * Modified Aug 21 1991 by Eil, to change enumerated result_type
 * Modified 4/22/92 by Eil for QNX 4.
 */

#ifndef CMDCTRL_H
#define CMDCTRL_H

#include <globmsg.h>

typedef struct {
    msg_hdr_type dasc_type; /* First byte indicates that it is a DAS command */
    dascmd_type dascmd;
} dasc_msg_type;

typedef unsigned char quit_type;
#define NOTHING_ON_QUIT     0   /* do nothing on DASCmd quit */
#define FORWARD_QUIT        1   /* Foward the Dascmd quit message */
#define SET_BREAK           2   /* Set break exception on shut_down */
#define TERM_ON_QUIT        3   /* terminate on shut_down */
#define QUIT_ON_QUIT	    4   /* send a SIGQUIT on shut_down */
#define MAX_QUIT_TYPE       4   /* maximum quit type */

/* actions when anybody dies before DASCmd QUIT */
typedef unsigned char death_type;
#define NOTHING_ON_DEATH    0	/* ignore */
#define REBOOT		    1	/* reboot */
#define DAS_RESTART	    2	/* restart DAS using cmdctrl's command line option */
#define TASK_RESTART	    3	/* restart task that died using given command at registration */
#define SHUTDOWN	    4	/* shutdown cmdctrl and all tasks registered */
#define MAX_DEATH_TYPE	    4

typedef struct {
    msg_hdr_type ccreg_byt;         /* First byte indicates that command is CCReg */
    unsigned char min_dasc;         /* Minimum DASCmd to have relayed to id that sent msg */
    unsigned char max_dasc;         /* Maximum DASCmd to have relayed to id that sent msg */
    unsigned char min_msg;          /* Minimum message type to have relayed to id that sends msg */
    unsigned char max_msg;          /* Maximum message type to have relayed to id that sends msg */
    quit_type how_to_quit;	    /* Action to take on DASCmd quit */
    death_type how_to_die;	    /* Action to take on death of registeree */
    char task_start[MAX_MSG_SIZE-7];	/* command to restart task  */
} ccreg_type;

#define CMD_CTRL "Cmdctrl"

#endif
