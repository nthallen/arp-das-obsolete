/* dmmclient.c interfaces to dmmdriver which interfaces to
   Diamond Systems DMM32 A/D, D/A, DIO board.
*/
#include "nortlib.h"
#include "dmmdrv.h"
#include "cltsrvr.h"
static char rcsid_dmm32_c[] =
  "$Header$";

static Server_Def DMM32Def = { DMM_NAME, 1, 0, 1, 0, 0, 0, 0 };

int dmm_init( void ) {
  rcsid_dmm32_c[0] = rcsid_dmm32_c[0];
  return CltInit( &DMM32Def );
}

/* dmm_command returns the appropriate value from the specified
   function or ~0 on error
*/
unsigned short dmm_command( unsigned short function,
                           unsigned short address,
						   unsigned short data ) {
  Send_to_dmm S;
  Reply_from_dmm R;
  S.signature = 'dm';
  S.function = function;
  S.address = address;
  S.data = data;
  if ( CltSend( &DMM32Def, &S, &R,
		sizeof(Send_to_dmm), sizeof(Reply_from_dmm) ) == 0 ) {
	if ( R.signature != 'dm' )
	  nl_error( 4, "Garbled response from dmmdriver" );
	return R.value.shrt;
  } else return ~0;
}

unsigned short dmm_gain( unsigned short address, double gain ) {
  Send_gain_to_dmm S;
  Reply_from_dmm R;
  S.signature = 'dm';
  S.function = DMMMSG_SET_GAIN;
  S.address = address;
  S.data = gain;
  if ( CltSend( &DMM32Def, &S, &R,
		sizeof(Send_gain_to_dmm), sizeof(Reply_from_dmm) ) == 0 ) {
	if ( R.signature != 'dm' )
	  nl_error( 4, "Garbled response from dmmdriver" );
	return R.value.shrt;
  } else return ~0;
}

/* dmm_scan_Mass returns the appropriate value from the specified
   function or ~0 on error
*/
unsigned short dmm_scan_Mass(
		  unsigned short fromMass,
		  unsigned short toMass,
		  unsigned short byMass,
		  unsigned short dwell ) {
  Send_scan_to_dmm S;
  Reply_from_dmm R;
  S.signature = 'dm';
  S.function = DMMMSG_SCAN_MASS;
  S.fromMass = fromMass;
  S.toMass = toMass;
  S.byMass = byMass;
  S.dwell = dwell;
  if ( CltSend( &DMM32Def, &S, &R,
		sizeof(S), sizeof(R) ) == 0 ) {
	if ( R.signature != 'dm' )
	  nl_error( 4, "Garbled response from dmmdriver" );
	return R.value.shrt;
  } else return ~0;
}

/* dmm_scan_Freq returns the appropriate value from the specified
   function or ~0 on error
*/
unsigned short dmm_scan_Freq(
		  unsigned short fromFreq,
		  unsigned short toFreq,
		  unsigned short byFreq,
		  unsigned short dwell ) {
  Send_scan_to_dmm S;
  Reply_from_dmm R;
  S.signature = 'dm';
  S.function = DMMMSG_SCAN_FREQ;
  S.fromMass = fromFreq;
  S.toMass = toFreq;
  S.byMass = byFreq;
  S.dwell = dwell;
  if ( CltSend( &DMM32Def, &S, &R,
		sizeof(S), sizeof(R) ) == 0 ) {
	if ( R.signature != 'dm' )
	  nl_error( 4, "Garbled response from dmmdriver" );
	return R.value.shrt;
  } else return ~0;
}

/*
=Name dmm_init(): Initiate communication with DMM32 Driver
=Subject DMM32 Analog I/O Driver
=Subject Startup
=Name dmm_command(): Send command to DMM32 Driver
=Subject Driver Interfaces
=Name dmm_shutdown(): Ask resident DMM32 Driver to quit
=Subject DMM32 Analog I/O Driver
=Subject Shutdown
=Name dmm_read(): Read from DMM32 Driver
=Subject DMM32 Analog I/O Driver
=Name dmm_write(): Write to DMM32 Driver
=Subject DMM32 Analog I/O Driver
=Name dmm_scdc_command(): Send an SCDC command directly to DMM32 Driver
=Subject DMM32 Analog I/O Driver
=Name dmm_quit(): Synonym for dmm_shutdown()
=Subject DMM32 Analog I/O Driver
=Subject Shutdown
=Name NCAR_read_rf_a2d(): Read A/D on NCAR RF Board
=Subject DMM32 Analog I/O Driver
=Name NCAR_read_rf_d2a(): Read D/A on NCAR RF Board
=Subject DMM32 Analog I/O Driver
=Name NCAR_write_rf_d2a(): Write to D/A on NCAR RF Board
=Subject DMM32 Analog I/O Driver
=Name NCAR_read_500V_a2d(): Read A/D on NCAR 500V Board
=Subject DMM32 Analog I/O Driver
=Name NCAR_read_500V_d2a(): Read D/A on NCAR 500V Board
=Subject DMM32 Analog I/O Driver
=Name NCAR_write_500V_d2a(): Write to D/A on NCAR 500V Board
=Subject DMM32 Analog I/O Driver
=Name NCAR_set_mass(): Select Mass on NCAR RF Board
=Subject DMM32 Analog I/O Driver
=Name NCAR_set_gain(): Set Gains for NCAR_set_mass() Function
=Subject DMM32 Analog I/O Driver

=Synopsis
unsigned short dmm_command( unsigned short function,
                           unsigned short address,
                           unsigned short data );
unsigned short dmm_shutdown(void);
unsigned short dmm_quit(void);
unsigned short dmm_read(unsigned short address);
unsigned short dmm_write(unsigned short address, unsigned short value);
unsigned short dmm_scdc_command(unsigned short addr);
unsigned short NCAR_read_rf_a2d(unsigned short addr);
unsigned short NCAR_read_rf_d2a(unsigned short addr);
unsigned short NCAR_write_rf_d2a(unsigned short addr, unsigned short value);
unsigned short NCAR_read_500V_a2d(unsigned short addr);
unsigned short NCAR_read_500V_d2a(unsigned short addr);
unsigned short NCAR_write_500V_d2a(unsigned short addr, unsigned short value);
unsigned short NCAR_set_mass(unsigned short mass);
unsigned short NCAR_set_gain(unsigned short addr, unsigned short gain);

=Description
  
  <P>NCAR_set_mass() causes both of the D/A channels on the RF
  board to be written. The mass parameter is in .1 AMU units,
  i.e. 10 represents 1 AMU. This function uses three pairs of
  constants to calculate the appropriate output values by a
  simple slope-intercept formula. The constants may be changed
  via NCAR_set_gain(), with the appropriate addresses defined in
  the header file.</P>

=Returns

  <P>Most functions return 0 on success. Failures are generally
  non-fatal. Read functions return values appropriate to the
  device being read. The client is responsible for converting the
  raw value to reasonable units, which may involve converting the
  unsigned value to a signed value.</P>
  
=SeeAlso

  =Driver Interfaces=
  =Startup=
  =Shutdown=
=End
*/
