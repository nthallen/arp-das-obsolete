/* hint.c provides all the interrupt functionality (except the
   handler itself.
*/
#include <conio.h>
#include <i86.h>
#include <sys/irqinfo.h>
#include <sys/proxy.h>
#include "nortlib.h"
#include "intserv.h"
#include "internal.h"
#include "subbus.h"

static int tmr_iid = -1;
pid_t expint_proxy = 0;
int expint_irq = 9;

#define MAX_IRQ_104 12
static unsigned short irq104[ MAX_IRQ_104 ] = {
  0, 0, 0, 0x21, 0x22, 0x23, 0x24, 0x25, 0, 0x20, 0x26, 0x27
};

void expint_init(void) {

  if ( subbus_subfunction != 0 ) {

	/* Get a proxy for the handler */
	expint_proxy = qnx_proxy_attach( 0, NULL, 0, -1 );
	if ( expint_proxy == -1 )
	  nl_error( 3, "Unable to attach proxy for expint" );

	/* Attach the irq */
#ifndef ATTACHIT
	tmr_iid = qnx_hint_attach( expint_irq, expint_handler, 
								FP_SEG( &expint_proxy ) );
	if (tmr_iid == -1)
	  nl_error( 3, "Unable to attach IRQ %d", expint_irq);
#endif

	if ( subbus_subfunction == SB_SYSCON104 ) {
	  unsigned short cfg_word;
	
	  if ( expint_irq < 0 || expint_irq >= MAX_IRQ_104
						  || irq104[ expint_irq ] == 0 )
		nl_error( 3, "Specified IRQ %d is out of range or invalid",
						expint_irq );
	  cfg_word = ( inpw( 0x312 ) & ~0x3F ) | irq104[ expint_irq ];
	  nl_error( -2, "SC104 writing CPA word %04X", cfg_word );
	  outpw( 0x312, cfg_word );
	}
  } else nl_error( -2, "No Subbus Library Present" );
}

void expint_reset(void) {
  if ( tmr_iid != -1 ) qnx_hint_detach( tmr_iid );
  if ( subbus_subfunction == SB_SYSCON104 ) {
	unsigned short cfg_word;
	
	cfg_word = ( inpw( 0x312 ) & ~0x3F );
	outpw( 0x312, cfg_word );
  }
}
