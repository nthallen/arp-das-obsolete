/*
 * Discrete command card controller program.
 * $Log$
 * Revision 1.9  1998/02/14 23:13:46  eil
 * added SELECT capability
 *
 * Revision 1.8  1997/02/05  15:56:30  eil
 * uses new set_cmdenbl available with new subbus
 *
 * Revision 1.7  1996/04/25  20:32:02  eil
 * latest and greatest
 *
 * Revision 1.6  1995/06/14  14:56:16  eil
 * last version before set_cmdstrobe function and new subbus104.
 *
 * Revision 1.5  1993/04/28  15:56:31  eil
 * added MSG_DEBUG's, tabs and check error return of Reply
 *
 * Revision 1.4  1993/03/26  18:16:59  eil
 * before cepex
 *
 * Revision 1.5  1992/10/27  17:29:31  nort
 * Changed to reset cmd_idx not str_cmd in do loop for mult_cmds.
 * Eliminated *inc in one calculation. What is the format anyway?
 *
 * Revision 1.4  1992/10/27  17:13:30  nort
 * Moved to Nort's system.
 * Changed our includes to "" from <>
 * Eliminated unused j, included conio.h for outp prototype.
 * Converted to 4 space tabs
 *
 * Revision 1.3	 1992/08/13	 21:32:37  nort
 * Add revision log.
 *
 * revision 1.2 locked by: nort;
 * date: 1992/08/13 21:29:08;  author: nort;  state: Exp;  lines: +151 -142
 * Altered strobing sequence to reset strobe before command lines.
 * Also strobe once for all MULT_CMDS.
 * Also changed tabs to 4
 *
 * revision 1.1
 * date: 1992/08/13 21:02:12;  author: nort;  state: Exp;
 * Initial revision
 *
 * Modified by Eil July 1991 for QNX.
 * Ported to QNX 4 by Eil 4/15/92.
 */

/* header files */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <conio.h>
#include <sys/kernel.h>
#include <sys/name.h>
#include <sys/types.h>
#include "das.h"
#include "eillib.h"
#include "globmsg.h"
#include "scdc.h"
#include "dccc.h"
#include "subbus.h"
#include "disc_cmd.h"
#include "port_types.h"
static char rcsid[] = "$Id$";

/* defines */
#define HDR "dccc"
#define OPT_MINE "if:"

/* functions */
void init_cards(void);
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

char *opt_string=OPT_MSG_INIT OPT_CC_INIT OPT_MINE;
int init_fail = 0;
static sb_syscon = 0;
static use_cmdstrobe = 0;
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
  reply_type replycode;
  char buf[MAX_MSG_SIZE];
  int i,mult,inc;
  int cmd_idx, value, num_mult_cmds;
  pid_t recv_id;

  /* initialise msg options from command line */
  msg_init_options(HDR,argc,argv);
  BEGIN_MSG;

  /* process args */
  opterr = 0;
  optind = 0;
  do {
    i=getopt(argc,argv,opt_string);
    switch (i) {
    case 'f': strncpy(cmdfile, optarg, FILENAME_MAX-1); break;
    case 'i': init_fail = 1; break;
    case '?': msg(MSG_EXIT_ABNORM,"Invalid option -%c",optopt);
    default:  break;
    }
  } while (i!=-1);

  /* subbus */
  if (!load_subbus()) msg(MSG_EXIT_ABNORM,"Subbus lib not resident");
  if (subbus_subfunction == SB_SYSCON || 
      subbus_subfunction == SB_SYSCON104) sb_syscon = 1;
  if (subbus_features & SBF_CMDSTROBE) use_cmdstrobe = 1;
  else if (sb_syscon) 
    msg(MSG_WARN,"Out of date resident subbus library: please upgrade");

  /* register yourself */
  if (qnx_name_attach(getnid(),LOCAL_SYMNAME(DCCC))==-1)
    msg(MSG_EXIT_ABNORM,"Can't attach symbolic name for %s",DCCC);

  read_commands();
  init_cards();
  cc_init_options(argc,argv,DCT_DCCC,DCT_DCCC,DC_MULTCMD,DC_MULTCMD,FORWARD_QUIT);

  while (1) {

    /* loop initialisation */
    mult = 0;
    num_mult_cmds = 0;
    in_strobe_cmd = 0;
    replycode=DAS_OK;
    inc = 1;
    value = 0;
    strnset(buf,0,MAX_MSG_SIZE);
    buf[0] = DEATH;

    while ( (recv_id = Receive(0, buf, sizeof(buf) ))==-1)
      msg(MSG_WARN, "error recieving messages");

    /* check out msg structure */
    switch (buf[0]) {
    case DASCMD:
      switch (buf[1]) {
      case DCT_DCCC: 
	cmd_idx=buf[2]; 
	value = *( (UBYTE2 *)(buf+3) );
	break;
      case DCT_QUIT:
	if (buf[2]==DCV_QUIT) {
	  Reply(recv_id,&replycode,sizeof(reply_type));
	  DONE_MSG;
	}
      default:
	msg(MSG_WARN,"unknown DASCMD type %d received",buf[1]);
	replycode=DAS_UNKN;
      }
      break;
    case DC_MULTCMD:
      /* check commands are all of same type */
      cmd_idx=buf[2];
      num_mult_cmds = buf[1];
      inc = 1;
      if (cmds[cmd_idx].type == SET || cmds[cmd_idx].type == SELECT) inc = 3;
      value = *( (UBYTE2 *)(buf+3) );
      for (i=2; i< num_mult_cmds+2; i+=inc)
	if (cmds[buf[i]].type != cmds[cmd_idx].type) {
	  msg(MSG_WARN,"Bad DC_MULTCMD command recieved from task %d",recv_id);
	  replycode=DAS_UNKN;
	  break;
	}
      i = 2;
      mult = 1;
      break;
    default:
      msg(MSG_WARN,"unknown msg with header %d received",buf[0]);
      replycode=DAS_UNKN;
    }

    if (replycode==DAS_OK) {

      /* reset the strobe if necessary or define str_cmd
	 Don't reset strobe_set here, because we need to
	 check it's value again after the do loop.
	 */
      if (cmds[cmd_idx].type == STRB) {
	in_strobe_cmd++;
	if (strobe_set) {
	  if (sb_syscon) {
	    if (use_cmdstrobe) set_cmdstrobe(0);
	    else outp(0x30E, 2);
	  }
	  else reset_line(cmds[n_cmds-1].port,cmds[n_cmds-1].mask);
	  if (cmd_idx != str_cmd) msg(MSG_WARN, "Bad strobe sequence");
	} else str_cmd = cmd_idx;
      }

      do {
	switch (cmds[cmd_idx].type) {
	case STRB:
	  if (strobe_set) {
	    msg(MSG_DEBUG,"STRB reset: PORT %03X mask %04X command index %d",ports[cmds[cmd_idx].port].sub_addr, cmds[cmd_idx].mask, cmd_idx);
	    reset_line(cmds[cmd_idx].port, cmds[cmd_idx].mask);
	  } else {
	    msg(MSG_DEBUG,"STRB set: port %03X mask %04X command index %d",ports[cmds[cmd_idx].port].sub_addr, cmds[cmd_idx].mask,cmd_idx);
	    set_line(cmds[cmd_idx].port, cmds[cmd_idx].mask);
	  }
	  break;
	case STEP:
	  set_line(cmds[cmd_idx].port, cmds[cmd_idx].mask);
	  reset_line(cmds[cmd_idx].port, cmds[cmd_idx].mask);
	  break;
	case SET:
	  msg(MSG_DEBUG,"SET: PORT %d, mask %04X, value %04X, command index %d",ports[cmds[cmd_idx].port].sub_addr, cmds[cmd_idx].mask, value, cmd_idx);
	  if (cmds[cmd_idx].mask)
	    if (value) set_line(cmds[cmd_idx].port, cmds[cmd_idx].mask);
	    else reset_line(cmds[cmd_idx].port, cmds[cmd_idx].mask);
	  else set_line(cmds[cmd_idx].port, value);
	  break;
	case SELECT:
	  msg(MSG_DEBUG,"SELECT: PORT %d, mask %04X, value %04X, command index %d",ports[cmds[cmd_idx].port].sub_addr, cmds[cmd_idx].mask, value, cmd_idx);
	  sel_line(cmds[cmd_idx].port, cmds[cmd_idx].mask, value);
	  break;
	case SPARE: replycode = DAS_UNKN;
	  msg(MSG_WARN,"command type SPARE received");
	  break;
	default: replycode = DAS_UNKN;
	  msg(MSG_WARN,"unknown command type %d received",cmds[cmd_idx].type);
	}			/* switch */

	/* update command and value */
	if (mult) {
	  i+=inc;
	  if ( (i - 2) >= (num_mult_cmds*inc)) mult = 0;
	  else {
	    cmd_idx = buf[i];
	    value = *( (UBYTE2 *)(buf+i+1) );
	  }
	}
      }	/* do */ while ( mult && replycode==DAS_OK);

      if (in_strobe_cmd) {
	if (strobe_set) strobe_set = 0;
	else {
	  if (sb_syscon) {
	    if (use_cmdstrobe) set_cmdstrobe(1);
	    else outp(0x30E, 3);
	  }
	  else set_line(cmds[n_cmds-1].port, cmds[n_cmds-1].mask);
	  strobe_set = 1;
	}
      }
    }

    if (Reply(recv_id, &replycode ,sizeof(reply_type))==-1)
      msg(MSG_WARN,"error replying to task %d",recv_id);

  }	/* while */
}	/* main */


/* functions */

/* return 1 on success, 0 on error */
void init_cards(void) {
  int i;
  for (i = 0; i < n_cfgcmds; i++)
    if (!write_ack(0,cfg_cmds[i].address, cfg_cmds[i].data))
      msg(init_fail ? MSG_EXIT_ABNORM : MSG_WARN,"No ack for configuration command at address %#X",cfg_cmds[i].address);
  for (i = 0; i < n_ports; i++)
    if (!write_ack(0,ports[i].sub_addr, ports[i].defalt))
      msg(init_fail ? MSG_EXIT_ABNORM : MSG_WARN,"No ack at port address %#X",ports[i].sub_addr);
  /* Set Command Strobe Inactive */
  if (sb_syscon) {
    if (use_cmdstrobe) set_cmdstrobe(0);
    else outp(0x30E, 2);
  }
  else reset_line(cmds[n_cmds-1].port,cmds[n_cmds-1].mask);
  /* Enable Commands */
  set_cmdenbl(1);
}

void set_line(int port, int mask) {
  ports[port].value |= mask;
  write_subbus(0,ports[port].sub_addr, ports[port].value);
}

void sel_line(int port, int mask, int value) {
  ports[port].value &= ~mask;
  ports[port].value |= (mask & ~value);
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
