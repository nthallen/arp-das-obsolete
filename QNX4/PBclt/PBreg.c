/* PBreg is a regulator for PlayBack functions

   Modes of operation:
     Fast forward: DC_data_rows = dbr_info.max_rows
	 Realtime: DC_data_rows = 1;
			   increment on every timer tick,
			   decrement as data comes in.
	 Slow Mo:  realtime at half speed... 
*/
#include <signal.h>
#include <stdio.h>
#include <time.h>
#include <errno.h>
#include <sys/proxy.h>
#include <sys/kernel.h>
#include <string.h>
#include <stdlib.h>
#include "dbr.h"
#include "msg.h"
#include "globmsg.h"
#include "nortlib.h"
#include "oui.h"

#define REG_SIG 254

/* time_prox holds information pertinent to the two timing proxies.
   If proxy == 0, the proxy has not been initialized.
   If proxy != 0, it has been.
*/
struct time_prox {
  pid_t proxy;
  timer_t timer;
  int active;
} main_timer = {0,0,0};

#define RM_PAUSE    0
#define RM_REALTIME 1
#define RM_FASTFWD  2
#define RM_FASTER   3
#define RM_SLOWER   4
#define RM_ONEROW   5

/* The only modes that can actually be assigned to
   Regulation_Mode are _PAUSE, _REALTIME and _FASTFWD.
   All the others masquerade as _REALTIME.
*/
static int Regulation_Mode = RM_REALTIME;
static int Max_Accumulation, Data_rows_inc;

#define NSEC (1000000000)

static void start_timing(long int rn, long int rd,
	  struct time_prox *tp, unsigned char msgtxt) {
  struct sigevent ev;
  struct itimerspec tval;

  if ( tp->proxy <= 0 ) {
	tp->proxy = nl_make_proxy(&msgtxt, 1);
	tp->timer = 0;
  }
  if ( tp->timer <= 0 ) {
	ev.sigev_signo = -tp->proxy;
	tp->timer = timer_create(CLOCK_REALTIME, &ev);
	if (tp->timer == -1)
	  nl_error(MSG_EXIT_ABNORM, "Error %d making timer", errno);
	tp->active = 0;
  }
  tval.it_value.tv_sec = rn/rd;
  tval.it_value.tv_nsec = (rn%rd) * (NSEC/rd);
  tval.it_interval = tval.it_value;
  if (timer_settime(tp->timer, 0, &tval, NULL) == -1)
	nl_error(MSG_EXIT_ABNORM, "Error %d in timer_settime", errno);
  tp->active = 1;
}

static void suspend_timing( struct time_prox *tp ) {
  struct itimerspec tval;

  if ( ! tp->active ) return;
  tval.it_value.tv_sec = 0;
  tval.it_value.tv_nsec = 0;
  tval.it_interval = tval.it_value;
  if (timer_settime(tp->timer, 0, &tval, NULL) == -1 )
	nl_error(MSG_EXIT_ABNORM, "Error %d in timer_settime", errno);
  tp->active = 0;
}

/* Only detaches proxy if timing is internal */
static void stop_timing(struct time_prox *tp) {
  if (tp->timer != 0) {
	timer_delete(tp->timer);
	tp->timer = 0;
	tp->active = 0;
  }
  if (tp->proxy != 0) {
	qnx_proxy_detach(tp->proxy);
	tp->proxy = 0;
  }
}

extern unsigned char DC_data_rows;

static void set_regulation( int mode ) {
  static long int num=1, den=1;
  switch ( mode ) {
	case RM_FASTFWD:
	  msg( -2, "Fast Forward" );
	  DC_data_rows = dbr_info.max_rows;
	  suspend_timing( &main_timer );
	  break;
	case RM_PAUSE:
	  msg( -2, "Pause" );
	  suspend_timing( &main_timer );
	  DC_data_rows = 0;
	  break;
	case RM_ONEROW:
	  msg( -2, "One Row" );
	  suspend_timing( &main_timer );
	  DC_data_rows = 1;
	  mode = RM_PAUSE;
	  break;
	case RM_REALTIME:
	  Max_Accumulation = 1; /* dbr_info.max_rows; */
	  num = 1; den = 1; break;
	case RM_FASTER:
	  if ( num % 2 == 0 ) num /= 2;
	  else den *= 2;
	  Max_Accumulation *= 2;
	  if ( Max_Accumulation > dbr_info.max_rows )
		Max_Accumulation = dbr_info.max_rows;
	  mode = RM_REALTIME;
	  break;
	case RM_SLOWER:
	  if ( den % 2 == 0 ) den /= 2;
	  else num *= 2;
	  if ( Max_Accumulation > 1 )
		Max_Accumulation /= 2;
	  mode = RM_REALTIME;
	  break;
	default:
	  msg(MSG_WARN, "Unknown mode %d in set_regulation", mode );
  }
  if ( mode == RM_REALTIME ) {
	msg( -2, "Realtime" );
	DC_data_rows = 1;
	Data_rows_inc = Max_Accumulation;
	start_timing(Data_rows_inc*num*tmi(nsecsper), den*tmi(nrowsper),
					&main_timer, REG_SIG);
  }
  Regulation_Mode = mode;
}

typedef struct quit_proxy_s {
  struct quit_proxy_s *next;
  pid_t proxy;
} quit_proxy;
static quit_proxy *quit_proxies = NULL;

static void add_quit_proxy( pid_t proxy ) {
  quit_proxy *qp;
  qp = malloc(sizeof(quit_proxy));
  if (qp == 0) msg(MSG_EXIT_ABNORM, "Out of memory for quit_proxy");
  qp->next = quit_proxies;
  qp->proxy = proxy;
  quit_proxies = qp;
}

void main(int argc, char **argv) {
  int name_id;
  quit_proxy *qp;
  
  oui_init_options( argc, argv );
  name_id = qnx_name_attach( 0, nl_make_name( "PBreg", 0 ) );
  if (name_id == -1)
	msg(MSG_EXIT_ABNORM, "Unable to attach name" );
  BEGIN_MSG;
  DC_data_rows = 1;
  DC_operate();
  for ( qp = quit_proxies; qp != 0; qp = qp->next )
	Trigger( qp->proxy );
  DONE_MSG;
}

void DC_data(dbr_data_type *dr_data) {
  switch ( Regulation_Mode ) {
	case RM_PAUSE:
	  DC_data_rows = 0;
	  break;
	case RM_REALTIME:
	  if ( DC_data_rows > dr_data->n_rows )
		DC_data_rows -= dr_data->n_rows;
	  else DC_data_rows = 0;
	  break;
	case RM_FASTFWD:
	  DC_data_rows = dbr_info.max_rows;
	  break;
	default:
	  msg( 1, "Unexpected Regulation_Mode %d", Regulation_Mode );
	  break;
  }
}

void DC_tstamp(tstamp_type *tstamp) { tstamp = tstamp; }

#define MKDCTV(x,y) ((x<<8)|y)
#define DCTV(x,y) MKDCTV(DCT_##x,DCV_##y)

void DC_DASCmd(unsigned char type, unsigned char val) {
  switch (MKDCTV(type, val)) {
	case DCTV(TM,TM_START):
	  set_regulation( RM_REALTIME );
	  break;
	case DCTV(TM,TM_END):
	  suspend_timing( &main_timer );
	  break;
	case DCTV(QUIT,QUIT):
	  msg( 0, "I got Quit, DC_data_rows = %d", (int)DC_data_rows );
	  set_regulation( RM_FASTFWD );
	  stop_timing( &main_timer );
	  msg( 0, "Now DC_data_rows = %d", (int)DC_data_rows );
	  break;
	default:
	  break;
  }
}

/* Handle other commands */
void DC_other(unsigned char *msg_ptr, int sent_tid) {
  if ( sent_tid == main_timer.proxy ) {
	if ( DC_data_rows < Max_Accumulation - Data_rows_inc )
	  DC_data_rows += Data_rows_inc;
	else DC_data_rows = Max_Accumulation;
	return; /* Don't reply to the proxy */
  } else if (strcmp( msg_ptr, "pbFF" ) == 0 ) {
	set_regulation( RM_FASTFWD );
  } else if (strcmp( msg_ptr, "pbRT" ) == 0 ) {
	set_regulation( RM_REALTIME );
  } else if (strcmp( msg_ptr, "pbPS" ) == 0 ) {
	set_regulation( RM_PAUSE );
  } else if (strcmp( msg_ptr, "pb*2" ) == 0 ) {
	set_regulation( RM_FASTER );
  } else if (strcmp( msg_ptr, "pb/2" ) == 0 ) {
	set_regulation( RM_SLOWER );
  } else if (strcmp( msg_ptr, "pb1R" ) == 0 ) {
	set_regulation( RM_ONEROW );
  } else if (strncmp( msg_ptr, "pbQQ", 4 ) == 0 ) {
	add_quit_proxy( *(pid_t *)(msg_ptr+4) );
  } else reply_byte( sent_tid, DAS_UNKN );
  reply_byte( sent_tid, DAS_OK );
}
