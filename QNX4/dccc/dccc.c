/*
	Discrete command card controller program.
	Modified by Eil July 1991 for QNX.
	Ported to QNX 4 by Eil 4/15/92.
*/

/* header files */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/kernel.h>
#include <sys/name.h>
#include <conio.h>
#include <sys/types.h>
#include <das_utils.h>
#include <cmdctrl.h>
#include <globmsg.h>
#include <scdc.h>
#include <dccc.h>
#include <subbus.h>
#include "disc_cmd.h"

/* defines */
#define HDR "dccc"
#define OPT_MINE "f:"

/* functions */
int init_cards(void);
void set_line(int port, int mask), reset_line(int port, int mask);
void read_commands(void);
int get_type(char *buf, int *type), get_line(FILE *fp, char *buf);

/* global variables */
struct cfgcmd {
  unsigned int address;
  unsigned int data;
} *cfg_cmds;

struct port {
  int sub_addr;
  int defalt;
  int value;
} *ports;

struct cmd {
  int type;
  int port;
  int mask;
} *cmds;

char *opt_string=OPT_MSG_INIT OPT_BREAK_INIT OPT_CC_INIT OPT_MINE;
static int dcc_nset = 1, sb_syscon = 0;
static char cmdfile[FILENAME_MAX] = "dccc_cmd.txt";
static int n_cfgcmds = 0, n_ports = 0, n_cmds = 0;
static int strobe_set = 0;
static int in_strobe_cmd = 0;
static int str_cmd;


void main(int argc, char **argv) {

/* getopt variables */
extern char *optarg;
extern int optind, opterr, optopt;

/* local variables */	
char name[FILENAME_MAX+1];
reply_type replycode;
char buf[MAX_MSG_SIZE];
int i,j,mult,inc;
int cmd_idx, value;
pid_t recv_id;

    /* initialise msg options from command line */
    msg_init_options(HDR,argc,argv);
    BEGIN_MSG;
    break_init_options(argc,argv);

    /* process args */
    opterr = 0;
    do {
	i=getopt(argc,argv,opt_string);
	switch (i) {
	    case 'f': strncpy(cmdfile, optarg, FILENAME_MAX-1);  break;
	    case '?': msg(MSG_EXIT_ABNORM,"Invalid option -%c",optopt);
	    default:  break;
	}
    } while (i!=-1);

    /* subbus */
    if (!load_subbus()) msg(MSG_EXIT_ABNORM,"Subbus lib not resident");
    if (subbus_subfunction == SB_SYSCON) sb_syscon = 1;
  
    /* register yourself */
    if ((qnx_name_attach(getnid(),LOCAL_SYMNAME(DCCC,name)))==-1)
	msg(MSG_EXIT_ABNORM,"Can't attach name %s",name);

    read_commands();
  
    if (!init_cards())  msg(MSG_WARN,"Card Initialisation Error");

    cc_init_options(argc,argv,DCT_DCCC,DCT_DCCC,DC_MULTCMD,DC_MULTCMD,FORWARD_QUIT);

    while (1) {	

	/* loop initialisation */		
	mult = 0;
	in_strobe_cmd = 0;
	buf[MAX_MSG_SIZE-1] = 0xFF;
	replycode=DAS_OK;
	inc = 1;

	while ( (recv_id = Receive(0, buf, sizeof(buf) ))==-1)
	    msg(MSG_WARN, "error recieving messages");

	/* check out msg structure */
	switch (buf[0]) {
	    case DASCMD: 
		switch (buf[1]) {
		    case DCT_DCCC: cmd_idx=buf[2]; break;
		    case DCT_QUIT:
			if (buf[2]==DCV_QUIT) {
			    Reply(recv_id,&replycode,sizeof(reply_type));
			    DONE_MSG;
			}
		    default: replycode=DAS_UNKN;
		}
		break;
	    case DC_MULTCMD:
		/* check commands are all of same type */
		cmd_idx=buf[1];
		if (cmds[cmd_idx].type == SET) inc = 2;
		for (i=1, value=buf[2]; buf[i] != 0xFF; i+=inc)
		    if (cmds[buf[i]].type != cmds[cmd_idx].type) {
			msg(MSG_WARN,"Bad DC_MULTCMD command recieved from task %d",recv_id);
			replycode=DAS_UNKN;
			break;
		    }
		i = mult = 1;
		break;
	    default: replycode=DAS_UNKN;
	}

	
	if (replycode==DAS_OK)
	    do {
		switch (cmds[cmd_idx].type) {
		    case STRB:
			in_strobe_cmd++;
			if (strobe_set) {
			    msg(MSG,"PORT %03X mask %04X",ports[cmds[cmd_idx].port].sub_addr, cmds[cmd_idx].mask);
			    reset_line(cmds[str_cmd].port, cmds[str_cmd].mask);
			} else {
			    msg(MSG,"port %03X mask %04X",ports[cmds[cmd_idx].port].sub_addr, cmds[cmd_idx].mask);
			    set_line(cmds[cmd_idx].port, cmds[cmd_idx].mask);
			}
			break;
		    case STEP:
			set_line(cmds[cmd_idx].port, cmds[cmd_idx].mask);
			reset_line(cmds[cmd_idx].port, cmds[cmd_idx].mask);
			break;
		    case SET:
			if (cmds[cmd_idx].mask)
			    if (value) set_line(cmds[cmd_idx].port, cmds[cmd_idx].mask);
			    else reset_line(cmds[cmd_idx].port, cmds[cmd_idx].mask);
			else set_line(cmds[cmd_idx].port, value);
			break;
		    case SPARE:
		    default: replycode = DAS_UNKN;
		} /* switch */

		Reply(recv_id,&replycode,sizeof(reply_type));

		/* update command and value */				
		if (mult) {
		    i+=inc;
		    cmd_idx = buf[i];
		    if (cmd_idx == 0xFF) mult = 0;					
		    else value = buf[i+1];
		}
		/* strobe if required */
		if (!mult && in_strobe_cmd)
		    if (strobe_set) {
			if (sb_syscon) outp(0x30E, 2);
			else  reset_line(cmds[n_cmds-1].port,cmds[n_cmds-1].mask);
			strobe_set = 0;
			if (cmd_idx != str_cmd) msg(MSG_WARN, "Bad strobe sequence");
		    } else {
			if (sb_syscon) outp(0x30E, 3);
			else set_line(cmds[n_cmds-1].port, cmds[n_cmds-1].mask);
			strobe_set = 1;
			str_cmd = cmd_idx;
		    }
	    } /* do */  while ( mult );
	/* if */
	else Reply(recv_id, &replycode ,sizeof(reply_type));

    } /* while */
}  /* main */


/* functions */

/* return 1 on success, 0 on error */
int init_cards(void) {

  int i, dcc_nset;

  dcc_nset = 1;
  for (i = 0; i < n_cfgcmds; i++)
    if (!write_ack(0,cfg_cmds[i].address, cfg_cmds[i].data)) break;
  if (i == n_cfgcmds) { /*   Establish default values for all commands    */
    for (i = 0; i < n_ports; i++)
      if (!write_ack(0,ports[i].sub_addr, ports[i].defalt)) break;
    if (i == n_ports) dcc_nset = 0;
  }
  /* Enable Commands */  
  set_cmdenbl(1);
  return(!dcc_nset);
}

void set_line(int port, int mask) {
  ports[port].value |= mask;
  write_subbus(0,ports[port].sub_addr, ports[port].value);
}

void reset_line(int port, int mask) {
  ports[port].value &= ~mask;
  write_subbus(0,ports[port].sub_addr, ports[port].value);
}

/* No special commands for the time being */

void read_commands(void) {
  FILE *fp;
  char buf[128], *p;
  int i;

  if ((fp = fopen(cmdfile, "r")) == NULL)
  	msg(MSG_EXIT_ABNORM,"error opening %s",cmdfile);

  if (get_line(fp, buf) || (sscanf(buf, "%d", &n_cfgcmds) != 1))
        	msg(MSG_EXIT_ABNORM,"error getting number of config commands");

  cfg_cmds = (struct cfgcmd *) malloc(n_cfgcmds * sizeof(struct cfgcmd));

  /* Configuration commands */
  for (i = 0; i < n_cfgcmds; i++)
    if (get_line(fp, buf) ||
        (sscanf(buf, "%x,%x", &cfg_cmds[i].address, &cfg_cmds[i].data) != 2))
        	msg(MSG_EXIT_ABNORM, "error getting configuration command %d",i);

  if (get_line(fp, buf) || (sscanf(buf, "%d", &n_ports) != 1))
         	msg(MSG_EXIT_ABNORM, "error getting number of ports");

  ports = (struct port *) malloc(n_ports * sizeof(struct port));

  /* Port information */
  for (i = 0; i < n_ports; i++) {
    if ((get_line(fp, buf)) ||
        (sscanf(buf, "%x,%x", &ports[i].sub_addr, &ports[i].defalt) != 2))
        	msg(MSG_EXIT_ABNORM, "error getting port %d info",i);
    ports[i].value = 0;
  }
  
  if (get_line(fp, buf) || (sscanf(buf, "%d", &n_cmds) != 1))
         	msg(MSG_EXIT_ABNORM, "error getting number of commands");

  cmds = (struct cmd *) malloc(n_cmds * sizeof(struct cmd));
  
  /* Commands */ 
  for (i = 0; i < n_cmds; i++) {
    if (get_line(fp,buf) || get_type(buf, &cmds[i].type))
    	msg(MSG_EXIT_ABNORM, "error getting command %d type", i);

    for (p = &buf[0]; *p != ','; p++);
    if (sscanf(p, ",%d,%x", &cmds[i].port, &cmds[i].mask) != 2)
    	msg(MSG_EXIT_ABNORM, "error getting command %d info");
}

  /* dont read special commands */

  fclose(fp);
}
