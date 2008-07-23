/* GC functions: gc.c */
#include <conio.h>
#include <i86.h>
#include "ttdriver.h"
#include "ttintern.h"
#include "nortlib.h"

static int gc_is_configured = 0;

/* GC reset
	CFG  <- 0x18 (standard read cfg)
	CTRL <- 9
	ADDR <- TT_GC_BASE
	CTRL <- 1
	[wait for reset delay]
	CTRL <- 9
	[wait for hold delay]
	CTRL <- 8
	[look for ACK (CTRL & 0x10)==0 ]
	CTRL <- 9
	ADDR <- TT_DUMMY_ADDR
*/
unsigned short tti_gc_reset( void ) {
  outp( TT_CFG_PORT, TT_READ_CFG );
  outp( TT_CTRL_PORT, 9 );
  outp( TT_ADDR_PORT, TT_GC_BASE );
  outp( TT_CTRL_PORT, 1 );
  delay( 10 ); /* wait for reset delay */
  outp( TT_CTRL_PORT, 9 );
  delay( 100 ); /* wait for hold delay */
  outp( TT_CTRL_PORT, 8 );
  /* look for ACK (CTRL & 0x10)==0 */
  start_timer();
  while ( (inp( TT_CTRL_PORT ) & 0x10 ) != 0 && ! timed_out ) ;
  stop_timer();
  /* outp( TT_CTRL_PORT, 9 ); */
  outp( TT_ADDR_PORT, TT_DUMMY_ADDR );
  if ( timed_out ) return ~0;
  gc_is_configured = 1;
  return 0;
}

static unsigned short gc_byte( void ) {
  unsigned short byte;

  if ( ! gc_is_configured )
	return 0;
  outp( TT_CTRL_PORT, 8 );
  start_timer();
  while ( !timed_out && ( inp( TT_CTRL_PORT ) & 0x10 ) != 0 );
  byte = inp( TT_DATA_PORT ) & 0xFF;
  outp( TT_CTRL_PORT, 9 );
  while ( !timed_out && ( inp( TT_CTRL_PORT ) & 0x10 ) == 0 );
  stop_timer();
  if ( timed_out )
	gc_is_configured = 0;
  return byte;
}

unsigned short tti_gc_byte( void ) {
  unsigned short byte;

  if ( ! gc_is_configured ) return ~0;
  outp( TT_CFG_PORT, TT_READ_CFG );
  outp( TT_CTRL_PORT, 9 );
  outp( TT_ADDR_PORT, TT_GC_BASE );
  byte = gc_byte();
  outp( TT_ADDR_PORT, TT_DUMMY_ADDR );
  return byte;
}

#define GCFREQREF (100.0 * 50000000.0)

static unsigned long read_gc_counter( void ) {
  union {
	unsigned long longval;
	unsigned char chval[ sizeof( long ) ];
  } u;
  
  u.chval[3] = 0;
  u.chval[2] = gc_byte();
  u.chval[1] = gc_byte();
  u.chval[0] = gc_byte();
  /* if ( u.longval == 0L ) u.longval = 1L; */
  /* u.longval = (unsigned long) (GCFREQREF / u.longval ); */
  return u.longval;
}

/* returns the actual number of samples read. If an error
occurs (timeout) then the first offset code is set to 0xFFFF
*/
unsigned short tti_gc_read( gc_data_buf *gcbuf ) {
  int i, samples = 0;

  if ( ! gc_is_configured ) {
	gcbuf->end_offset[0] = ~0;
    return 0;
  }
  outp( TT_CFG_PORT, TT_READ_CFG );
  outp( TT_CTRL_PORT, 9 );
  outp( TT_ADDR_PORT, TT_GC_BASE );
  for ( i = 0; i < N_GC_CHANNELS; i++) {
	int bytes;
	bytes = gc_byte();
	if ( bytes == 0xFF ) bytes = 53;
	if ( bytes > 53 )
	  nl_error( 4, "bytes to read was %u", bytes );
	while ( bytes >= 3 ) {
	  gcbuf->samples[ samples++ ] = read_gc_counter();
	  bytes -= 3;
	}
	while ( bytes-- > 0 )
	  gc_byte();
	gcbuf->end_offset[ i ] = samples;
  }
  outp( TT_ADDR_PORT, TT_DUMMY_ADDR );
  return samples;
}

/* tt_gc_chan() function masks all of the negotiation with the
   gc board and simply returns the latest value from the specified
   channel. It reads all the counters if this channel hasn't been
   latched since last being read.
   
   I will buffer up to two counter values for each channel, but
   no more, assuming that the caller will call at approximately
   the correct frequency.
*/
/* gc_chan structure:
   head == -1   --> queue is empty
   else q[head] is the next item to be dequeued
   q[tail] is where to enqueue the next item.
   if head == tail, queue is full
*/
typedef struct {
  int head, tail;
  int is_latched;
  unsigned long q[4];
} gc_chan;
static gc_chan chans[ N_GC_CHANNELS ] = {
  -1, 0, 0, 0L, 0L,
  -1, 0, 0, 0L, 0L,
  -1, 0, 0, 0L, 0L,
  -1, 0, 0, 0L, 0L
};

static void gc_enqueue( gc_chan *gc, unsigned long val ) {
  gc->q[gc->tail] = val;
  if ( gc->head == -1 ) gc->head = gc->tail;
  else if ( gc->head == gc->tail )
	gc->head = (gc->head + 1) & 3;
  gc->tail = (gc->tail+1) & 3;
}

static unsigned long gc_dequeue( gc_chan *gc ) {
  unsigned long val;

  gc->is_latched = 0;
  if ( gc->head == -1 ) return 0L;
  val = gc->q[ gc->head ];
  gc->head = (gc->head+1) & 3;
  if ( gc->head == gc->tail )
	gc->head = -1;
  return val;
}

static void gc_chans_read( void ) {
  gc_data_buf gcb;
  int i, j;

  for ( i = 0; i < N_GC_CHANNELS; i++ )
	chans[ i ].is_latched = 1;
  tti_gc_read( &gcb );
  if ( gcb.end_offset[0] != ~0 ) {
	for ( i = j = 0; i < N_GC_CHANNELS; i++ ) {
	  for ( ; j < gcb.end_offset[i]; j++ ) {
		gc_enqueue( &chans[ i ], gcb.samples[ j ] );
	  }
	}
  }
}

unsigned long tti_gc_chan( unsigned short chan ) {
  gc_chan *gc;

  if ( chan >= N_GC_CHANNELS || ! gc_is_configured )
	return ~0L;
  gc = &chans[ chan ];
  if ( ! gc->is_latched )
	gc_chans_read();
  return gc_dequeue( gc );
}
