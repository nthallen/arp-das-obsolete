/* gtctdiag.c Xilinx-based Ungated Counter Diagnostic
   June 21, 1996
   Modified for PC/104 Implementation April, 2001

   The gated counter features a trigger counter and 2 pmt 
   channels, each with a 20-bit ungated counter and 2 gated 
   16-bit counters. The 2 channels share 2 programmable gates. 
   Counter collection features the standard auto-synchronization 
   code.
   
   The board is based at I/O Ports anywhere from 0 to 0x3C0 in
   steps of 0x040. With no jumpers installed, the address is
   0x3C0. The board can be identified by reading from offset 2
   for the characteristic identifier (presently programmed as
   0x0010...) (Note the highest 4 bits are the IRQ selection)

   The address map is:
   
      00: Status/Config
	  02: Hardware/Software Identifier
	  04: Adjustable Gate A config MSB=delay,LSB=width
	  06: Adjustable Gate B config
	  08: Trigger Counter
	  10: Counter 1N LSB
	  12: Counter 1N MSN
	  14: Counter 1A (gate A)
	  16: Counter 1B
	  18: Counter 2N LSB
	  1A: Counter 2N MSN
	  1C: Counter 2A
	  1E: Counter 2B   
   
   The status word is at offset 0 and consists of:
   
     D0:  Trigger Counter Overflow
	 D1:  Counter 1N Overflow
	 D2:  Counter 1A Overflow
	 D3:  Counter 1B Overflow
     D4:  Counter 2N Overflow
	 D5:  Counter 2A Overflow
	 D6:  Counter 2B Overflow
	 D7:  Spare
	 D8-11: R/W Integration period select
	 D12: Counter 1N Config
	 D13: Counter 2N Config
	 D14: L2Stat (True if latched twice before reading)
	 D15: Resynch (True if a resynch occurred since last status 
				   read)

  Diagnostic strategy:
  
	Board address is set, report it.
	Set integration period to 4 Hz
	Read at 4 Hz and report counts and status
	Adjust read frequency up and down to cause resynchs
	(report counts to rtg?)
  
  The gated counter is similar to the ungated counter in many 
  respects.

*/
#include <sys/types.h>
#include <signal.h>
#include <time.h>
#include <sys/kernel.h>
#include <sys/proxy.h>
#include <conio.h>
#include <stdlib.h>
#include "nortlib.h"

#define UGCT_ID 0x0111
#define GTCT_ID 0x0110
#define ONE_SEC 1000000000
#define RD_HZ 4
unsigned long ip_period;

static pid_t time_proxy;
static timer_t timer;

static void set_timer( unsigned long nsecs ) {
  struct sigevent ev;
  struct itimerspec tval;

  if ( timer <= 0 ) {
	time_proxy = qnx_proxy_attach( 0, NULL, 0, -1 );
	if ( time_proxy == -1 )
	  nl_error( 3, "Error getting proxy" );
	ev.sigev_signo = -time_proxy;
	timer = timer_create( CLOCK_REALTIME, &ev );
	if ( timer == -1 )
	  nl_error( 3, "Error in timer_create" );
  }
  tval.it_value.tv_sec = tval.it_interval.tv_sec = 0;
  tval.it_value.tv_nsec = tval.it_interval.tv_nsec = nsecs;
  if ( timer_settime( timer, TIMER_ADDREL, &tval, NULL ) == -1 )
	nl_error( 3, "Error in timer_settime" );
}

#ifdef __USAGE
%C	[Port]
	PC/104 Gated Counter Diagnostic
#endif

static int bd_delay, bd_width;
static void set_gates( unsigned short addr, int delay, int width ) {
  unsigned short gateword;
  if ( delay < 0 ) delay = 0;
  if ( delay > 255 ) delay = 255;
  bd_delay = delay;
  if ( width < 0 ) width = 0;
  if ( width > 255 ) width = 255;
  bd_width = width;
  gateword = (delay << 8 ) + width;
  outpw( addr+4, gateword );
  outpw( addr+6, gateword );
}

void main( int argc, char **argv ) {
  unsigned short ctno;
  unsigned short bdaddr = 0x3C0;
  unsigned short data, status;
  unsigned long nsamples = 0, ctval;
  int c, nlct = 0;

  if ( argc > 1 ) bdaddr = atoh( argv[1] );
  printf( "Testing Counter Board at address %04X\n", bdaddr );
  status = inpw( bdaddr + 2);
  printf( "Firmware/IRQ Reg: %04X\n", status );
  
  /* Set integration period to 4 Hz */
  data = ((16/RD_HZ)-1) << 8;
  outpw( bdaddr, data );
  
  /* Set gates to 0 delay max width */
  set_gates( bdaddr, 0, 255 );
  /* outpw( bdaddr+4, 0x00FF ); */
  /* outpw( bdaddr+6, 0x00FF ); */
  
  /* Set up a 4 Hz Proxy */
  set_timer( ip_period = ONE_SEC/RD_HZ );

  /* Read at RD_HZ and report counts and status */
  for (;;) {
	pid_t who;
	who = Receive( 0, NULL, 0 );
	if ( who != time_proxy )
	  nl_error( 3, "Received from unknown sender %04X", who );
	printf( "%8ld: ", nsamples++ );
	status = inpw( bdaddr );
	printf( " %04X", status );
	data = inpw( bdaddr + 0x08 ); /* Trigger Counts */
	printf( " %5u", data );
	for ( ctno = 0; ctno < 2; ctno++ ) {
	  data = inpw( bdaddr + 0x10 + 8*ctno + 2 );
	  ctval = ((unsigned long)data) << 16;
	  data = inpw( bdaddr + 0x10 + 8*ctno );
	  ctval += data;
	  printf( " %7lu", ctval ); /* Ungated */
	  data = inpw( bdaddr + 0x10 + 8*ctno + 4 );
	  printf( " %5u", data ); /* Gate A Counts */
	  data = inpw( bdaddr + 0x10 + 8*ctno + 6 );
	  printf( " %5u", data ); /* Gate B Counts */
	}
	data = inpw( bdaddr + 0x04 );
	printf( " %04X", data ); /* Gate A Config */
	data = inpw( bdaddr + 0x06 );
	printf( " %04X", data ); /* Gate B Config */
	
	if ( status & 0x8000 ) nlct = 3;
	putc( nlct > 0 ? '\n' : '\r', stdout );
	if ( nlct > 0 ) nlct--;
	fflush( stdout );
	if ( kbhit() ) {
	  switch ( c = getch() ) {
		case '>': set_timer( ip_period += (ONE_SEC/RD_HZ)/(10*RD_HZ) ); break;
		case '<': set_timer( ip_period -= (ONE_SEC/RD_HZ)/(10*RD_HZ) ); break;
		case '=': set_timer( ip_period = ONE_SEC/RD_HZ ); break;
		case '+': set_gates( bdaddr, bd_delay+1, bd_width ); break;
		case '-': set_gates( bdaddr, bd_delay-1, bd_width ); break;
		default: break;
	  }
	  while ( kbhit() ) getch();
	  if ( c == '\033' ) break;
	}
  }
  timer_delete( timer );
  qnx_proxy_detach( time_proxy );
  while (kbhit()) getch();
  putc('\n', stdout);
}
