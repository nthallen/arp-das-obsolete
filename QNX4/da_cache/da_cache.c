/* da_cache.c */
#include <stdlib.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/kernel.h>
#include <sys/name.h>
#include "subbus.h"
#include "nortlib.h"
#include "da_cache.h"
#include "da_cache_int.h"
#include "oui.h"
#include "cltsrvr.h"

static pid_t quit_proxy;

/* *_max_addr is outside the range. This is different
   from the command-line option. The adjustment is made
   in main() after oui_init_options();
*/
static unsigned short hw_min_addr, hw_max_addr;
static unsigned short sw_min_addr, sw_max_addr;

static unsigned short *hwcache, *swcache;

char *cache_hw_range, *cache_sw_range;

void cache_hardware( cache_msg *im, cache_rep *rep ) {
  int cache_addr;
  if ( im->address < hw_max_addr &&
	  im->address >= hw_min_addr ) {
	cache_addr = ( im->address - hw_min_addr ) / 2;
	switch ( im->type ) {
	  case CACHE_READ:
		rep->value = hwcache[cache_addr];
		break;
	  case CACHE_WRITE:
		hwcache[cache_addr] = im->value & CACHE_HW_MASK;
		if ( ! write_ack( 0, im->address, im->value ) ) {
		  rep->status = CACHE_E_NACK;
		  hwcache[cache_addr] |= CACHE_NACK_MASK;
		}
		break;
	  default:
		rep->status = CACHE_E_UNKN;
		break;
	}
  } else rep->status = CACHE_E_OOR;
}

void cache_software( cache_msg *im, cache_rep *rep ) {
  int cache_addr;
  if ( im->address < sw_max_addr &&
	  im->address >= sw_min_addr ) {
	cache_addr = im->address - sw_min_addr;
	switch ( im->type ) {
	  case CACHE_READ:
		rep->value = swcache[cache_addr];
		break;
	  case CACHE_WRITE:
		swcache[cache_addr] = im->value;
		break;
	  default:
		rep->status = CACHE_E_UNKN;
		break;
	}
  } else rep->status = CACHE_E_OOR;
}

static void operate( void ) {
  pid_t who;
  cache_msg im;
  cache_rep rep;
  int done = 0;

  while ( !done ) {
	who = Receive(0, &im, sizeof(im));
	rep.status = CACHE_E_OK;
	if ( who == -1 ) {
	  nl_error( 1, "Error receiving" );
	} else if ( who == quit_proxy ) return;
	else if ( im.header != CACHE_MSG ) {
	  rep.status = CACHE_E_UNKN;
	} else {
	  switch ( im.type ) {
		case CACHE_READ:
		case CACHE_WRITE:
		  if ( im.address < sw_min_addr )
			cache_hardware( &im, &rep );
		  else cache_software( &im, &rep );
		  break;
		case CACHE_QUIT:
		  rep.status = CACHE_E_OK;
		  done = 1;
		  break;
		default:
		  rep.status = CACHE_E_UNKN;
		  break;
	  }
	}
	if ( who != -1 ) {
	  rep.header = CACHE_MSG;
	  Reply( who, &rep, sizeof(rep) );
	}
  }
}

void process_range( char *txt,
  unsigned short *min, unsigned short *max ) {
  if ( txt == NULL ) {
	*min = *max = 0;
  } else {
	char *s, *t;
	s = t = txt;
	if ( !isxdigit(*t) )
	  nl_error( 3, "Expected hex digit at start of arg \"%s\"",
		txt );
	while ( isxdigit(*t) ) t++;
	if ( *t++ != '-' )
	  nl_error( 3, "Expected '-' in arg \"%s\"", txt );
	*min = atoh(s);
	s = t;
	if ( !isxdigit(*t) )
	  nl_error( 3, "Expected hex digit after '-': \"%s\"",
		txt );
	while ( isxdigit(*t) ) t++;
	if ( *t != '\0' )
	  nl_error( 3, "Garbage after range: \"%s\"", txt );
	*max = atoh(s);
	if ( *min > *max )
	  nl_error( 3, "Invalid range: \"%s\"", txt );
	if ( *max > 0xFFFD )
	  nl_error( 3, "Range max too high: \"%s\"", txt );
  }
}

void main( int argc, char **argv ) {
  int name_id;

  oui_init_options( argc, argv );
  process_range( cache_hw_range, &hw_min_addr, &hw_max_addr );
  if ( cache_hw_range )	hw_max_addr = (hw_max_addr + 2) & ~1;

  process_range( cache_sw_range, &sw_min_addr, &sw_max_addr );
  if ( cache_hw_range ) sw_max_addr++;
  
  if ( sw_min_addr < hw_max_addr )
	nl_error( 3, "-S range must be above -H range" );
  
  if ( hw_max_addr > hw_min_addr ) {
	int n = (hw_max_addr - hw_min_addr)/2;
	hwcache = new_memory(n*sizeof(unsigned short));
  }
  if ( sw_max_addr > sw_min_addr ) {
	swcache = new_memory(sw_max_addr-sw_min_addr);
  }

  /* register name */
  quit_proxy = cc_quit_request(0);
  name_id =
	qnx_name_attach( 0, nl_make_name( CACHE_NAME, 0 ) );
  if ( name_id == -1 )
	nl_error( 3, "Unable to attach name" );
  nl_error( 0, "Installed" );

  /* Receive loop */
  
  operate();

  /* cleanup: */
  qnx_name_detach( 0, name_id );
  nl_error( 0, "Terminated" );
  exit(0);
}
