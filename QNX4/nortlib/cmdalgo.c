/* cmdalgo.c contains command_algo.c
 * $Log$
 */
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "msg.h"
#include "dbr.h"
#include "nortlib.h"
#include "cmdalgo.h"
#include "nl_cons.h"
#include "cmdlex.h"
static char rcsid[] = "$Id$";

static int pass;
static struct {
  char *name;
  long int pos;
} mode[MAX_MODES];
int cur_mode;
static int n_modes;
static long int mode_time;
static int cmd_queued = 0;
static int cmd_recorded = 0;
static int cmd_terminated = 0;
long int time_next, time_mode, time_run, time_now, time_prev;
int holding;

static void mode_name(char *name) {
  for (cur_mode = 0; cur_mode < n_modes; cur_mode++)
	if (stricmp(name, mode[cur_mode].name) == 0) break;
  if (cur_mode == n_modes) {
	if (n_modes == MAX_MODES) nl_error(4, "Mode buffer overflow");
	mode[cur_mode].name = strdup(name);
	n_modes++;
  }
}

select_mode(char *s) {
  mode_name(s);
  if (cmd_queued) command_exec("\033");
  cmd_queued = 0;
  time_mode = mode_time = time_next = 0;
  if (pass != 0) {
	yyseek(mode[cur_mode].pos);
	timeline_text(*s=='\0'?"Initialize":s, 24);
	read_mode();
  }
}

signed long read_time(void) {
  signed long t;
  
  expecting(TK_INT);
  t = yyval.intval;
  while (yylex() == ':') {
	expecting(TK_INT);
	t = t * 60 + yyval.intval;
  }
  yyunlex();
  return(t);
}

read_mode(void) {
  int token;
  long int t;

  if (cmd_terminated) return;
  for (;;) {
	if (cmd_queued) {
	  cmd_queued = 0;
	  if (command_exec("\n")) {
		cmd_terminated = 1; /*{*/
		expecting('}');
		return;
	  }
	}
	token = yylex();

	if /*{*/ (token == '}') {
	  if (pass != 0) {
		time_next = 0;
		break;
	  } else return;
	}

	if (token == TK_STRING) {
	  select_mode(yyval.string);
	  if (pass == 0) { /* to match { */
		expecting('}');
		return;
	  }
	}

	if (token == '+' || token == TK_INT) {
	  if (token == TK_INT) yyunlex();
	  t = read_time();
	  if (token == '+') t += mode_time;
	  if (pass == 0) {
		if (t < mode_time) nl_error(3, "Mode time is not monotonic");
	  } else time_next = t - time_mode;
	  mode_time = t;
	  token = yylex();
	  if (cur_mode == 0 && cmd_recorded == 0) {
		command_record(pass == 0);
		cmd_recorded = 1;
	  } else if (command_check())
		nl_error(3, "Invalid Command State");
	}
	if (token != TK_COMMAND)
	  nl_error(3, "Expecting a Command");
	if (command_exec(yyval.string))
	  nl_error(4, "Unexpected termination from command_exec");
	cmd_queued = 1;
	if (pass != 0 && time_next > 0) break;
  }
}

static void mode_def(void) {
  expecting(TK_STRING);
  mode_name(yyval.string);
  expecting('{');
  mode[cur_mode].pos = ftell(yyin);
  mode_time = 0;
  read_mode();
}

static void read_algo(char *filename) {
  int token;
  
  yyin = fopen(filename, "r");
  if (yyin == NULL)
	nl_error(3, "Unable to open input file %s", filename);
  clex_error = nl_error;
  nl_error = clex_err;
  pass = 0;
  command_init();
  command_record(1);
  expecting(KW_INIT);
  expecting('{');
  mode_name("");
  mode[cur_mode].pos = ftell(yyin);
  mode_time = 0;
  read_mode();
  command_record(1);
  while ((token = yylex()) != EOF) {
	if (cmd_terminated)
	  nl_error(3, "Expecting EOF");
	if (token == KW_MODE) mode_def();
	else nl_error(3, "Expecting Mode or EOF");
  }
  if (!cmd_terminated)
	nl_error(3, "Termination instruction required");
  command_init();
  command_record(0);
  cmd_recorded = cmd_terminated = 0;
  pass = 1;
  nl_error = clex_error;
}

void command_algo(int argc, char **argv) {
  int c;

  msg_init_options("Alg", argc, argv);
  Con_init_options(argc, argv);
  timeline_init();
  do c=getopt(argc,argv,opt_string); while (c!=-1);
  if (optind >= argc)
	nl_error(3, "Must specify an algorithm file");
  read_algo(argv[optind]);
  optind=0; opterr=1;
  DC_init_options(argc, argv);
  BEGIN_MSG;
  select_mode("");
  DC_operate();
  DONE_MSG;
}

/* displays without moving cursor */
#define CDWIDTH 162
static void nlcons_display(int offset, char *s, char attr) {
  struct _console_ctrl *con_ctrl;
  int index;
  
  if (nlcons_defined) {
	char buf[CDWIDTH];
	int i;

	for (i = 0; *s != '\0' && i < CDWIDTH; s++) {
	  buf[i++] = *s;
	  buf[i++] = attr;
	}
	for (index = 0; index < MAXCONS; index++) {
	  if (nlcon_ctrl(index, &con_ctrl))
		console_write(con_ctrl, 0, offset, buf, i, NULL, NULL, NULL);
	}
  }
}

/* Display functions
Run: 00:00:00             Mode Turn_on:  00:00:00   Holding  Next: 00:00:00
*/
#define TIMELINE_ROW 24
#define TIMELINE_ATTR 0x70

timeline_text(char *s, int c) {
  nlcons_display(TIMELINE_ROW*80*2 + c*2, s, TIMELINE_ATTR);
}

timeline_init(void) {
  timeline_text("                                        "
				"                                        ", 0);
  timeline_text("Run:", 0);
  timeline_text("Next:", 62);
}

timeline_time(long int t, int c) {
  char ts[9];
  int hh, h;
  
  if (nlcons_defined) {
	ts[8] = '\0';
	hh = t % 60;
	h = hh % 10;
	ts[7] = h+'0';
	ts[6] = hh/10 + '0';
	ts[5] = ':';
	t /= 60;
	hh = t % 60;
	h = hh % 10;
	ts[4] = h+'0';
	ts[3] = hh/10 + '0';
	ts[2] = ':';
	t /= 60;
	hh = t % 100;
	h = hh % 10;
	ts[1] = h+'0';
	ts[0] = hh/10 + '0';
	timeline_text(ts, c);
  }
}
