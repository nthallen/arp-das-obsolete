%{
  #include "runnum.h"

  #ifdef SERVER
	/* Run Kinetics man1 bulb1 OH algo XS RA FL */
	/*   man bulb/molecule radical algo XSDet RadDet Flow */
	#include <string.h>
	#include "msg.h"

	run_params run_reset  = { "", "", "", "", "", "", "", -1, -1 };
	run_params K_defaults = { "", "", "", "", "", "", "", -1, -1 };
	run_params P_defaults = { "", "", "", "", "", "", "", -1, -1 };
	run_params S_defaults = { "", "", "", "", "", "", "", -1, -1 };
	run_params C_defaults = { "", "", "", "", "", "", "", -1, -1 };
	run_params T_defaults = { "", "", "", "", "", "", "", -1, -1 };
  
	static void set_string( char **pp, char *p ) {
	  if (*pp != NULL && **pp != '\0') free_memory(*pp);
	  if ( p == NULL || *p == '\0' ) *pp = "";
	  else *pp = nl_strdup( p );
	}
	static void set_defaults( run_params *KD, run_params *KP ) {
	  set_string( &KD->man, KP->man );
	  set_string( &KD->bulb, KP->bulb );
	  set_string( &KD->radical, KP->radical );
	  set_string( &KD->algo, KP->algo );
	  set_string( &KD->det1, KP->det1 );
	  set_string( &KD->det2, KP->det2 );
	  set_string( &KD->Flow, KP->Flow );
	}
  #endif
%}
&run_type
	: Kinetics &K_man {
		FILE *fp = RunLog_write( "Kinetics" );
		if ( fp != NULL ) {
		  read_molecule_list( fp, &$2 );
		  RunLog_hdr( fp, "XSD", $2.det1 );
		  RunLog_hdr( fp, "Radical", $2.radical );
		  RunLog_hdr( fp, "RadD", $2.det2 );
		  RunLog_hdr( fp, "FlowModel", $2.Flow );
		  RunLog_close( fp, &$2 );
		}
	  }
	: ProductStudy &P_man {
		FILE *fp = RunLog_write( "ProductStudy" );
		if ( fp != NULL ) {
		  read_molecule_list( fp, &$2 );
		  RunLog_hdr( fp, "IRD", $2.det1 );
		  RunLog_hdr( fp, "Radical", $2.radical );
		  RunLog_hdr( fp, "RadD", $2.det2 );
		  RunLog_close( fp, &$2 );
		}
	  }
	: Spectra &S_man {
		FILE *fp = RunLog_write( "Spectra" );
		if ( fp != NULL ) {
		  read_molecule_list( fp, &$2 );
		  RunLog_hdr( fp, "IRD", $2.det1 );
		  RunLog_close( fp, &$2 );
		}
	  }
	: Calibration &C_type {
		FILE *fp = RunLog_write( "Calibration" );
		if ( fp != NULL ) {
		  RunLog_hdr( fp, "SubType", $2.man );
		  RunLog_close( fp, &$2 );
		}
	  }
	: Test &T_type {
		FILE *fp = RunLog_write( "Test" );
		if ( fp != NULL ) {
		  RunLog_hdr( fp, "SubType", $2.man );
		  RunLog_close( fp, &$2 );
		}
	  }
	;
&set_type
	: Kinetics &K_man { set_defaults( &K_defaults, &$2 ); }
	: ProductStudy &P_man { set_defaults( &P_defaults, &$2 ); }
	: Spectra &S_man { set_defaults( &S_defaults, &$2 ); }
	: Calibration &C_type { set_defaults( &C_defaults, &$2 ); }
	: Test &T_type { set_defaults( &T_defaults, &$2 ); }
	;

&K_man <run_params>
	: &K_defaults { $0 = $1; }
	: &man &K_bulb { $0 = $2; $0.man = $1; }
	;
&K_bulb <run_params>
	: &K_defaults { $0 = $1; }
	: &bulb &K_radical { $0 = $2; $0.bulb = $1; }
	;
&K_radical <run_params>
	: &K_nodefaults { $0 = $1; }
	: &radical &K_algo { $0 = $2; $0.radical = $1; }
	;
&K_algo <run_params>
	: &K_nodefaults { $0 = $1; }
	: &algo &K_XS { $0 = $2; $0.algo = $1; }
	;
&K_XS <run_params>
	: &K_nodefaults { $0 = $1; }
	: &XS &K_RA { $0 = $2; $0.det1 = $1; }
	;
&K_RA <run_params>
	: &K_nodefaults { $0 = $1; }
	: &RA &K_Flow { $0 = $2; $0.det2 = $1; }
	;
&K_Flow <run_params>
	: &K_nodefaults { $0 = $1; }
	: &Flow * { $0 = K_defaults; $0.Flow = $1; }
	;
&K_nodefaults <run_params>
	: &K_defaults { $0 = $1; }
	: NoDefaults * { $0 = run_reset; }
	;
&K_defaults <run_params>
	: * { $0 = K_defaults; }
	;

&P_man <run_params>
	: &P_defaults { $0 = $1; }
	: &man &P_bulb { $0 = $2; $0.man = $1; }
	;
&P_bulb <run_params>
	: &P_defaults { $0 = $1; }
	: &bulb &P_radical { $0 = $2; $0.bulb = $1; }
	;
&P_radical <run_params>
	: &P_nodefaults { $0 = $1; }
	: &radical &P_algo { $0 = $2; $0.radical = $1; }
	;
&P_algo <run_params>
	: &P_nodefaults { $0 = $1; }
	: &algo &P_IR { $0 = $2; $0.algo = $1; }
	;
&P_IR <run_params>
	: &P_nodefaults { $0 = $1; }
	: &IR &P_RA { $0 = $2; $0.det1 = $1; }
	;
&P_RA <run_params>
	: &P_nodefaults { $0 = $1; }
	: &RA * { $0 = P_defaults; $0.det2 = $1; }
	;
&P_nodefaults <run_params>
	: &P_defaults { $0 = $1; }
	: NoDefaults * { $0 = run_reset; }
	;
&P_defaults <run_params>
	: * { $0 = P_defaults; }
	;

&S_man <run_params>
	: &S_defaults { $0 = $1; }
	: &man &S_bulb { $0 = $2; $0.man = $1; }
	;
&S_bulb <run_params>
	: &S_defaults { $0 = $1; }
	: &bulb &S_algo { $0 = $2; $0.bulb = $1; }
	;
&S_algo <run_params>
	: &S_nodefaults { $0 = $1; }
	: &algo &S_IR { $0 = $2; $0.algo = $1; }
	;
&S_IR <run_params>
	: &S_nodefaults { $0 = $1; }
	: &IR * { $0 = S_defaults; $0.det1 = $1; }
	;
&S_nodefaults <run_params>
	: &S_defaults { $0 = $1; }
	: NoDefaults * { $0 = run_reset; }
	;
&S_defaults <run_params>
	: * { $0 = S_defaults; }
	;

&C_type <run_params>
	: &C_defaults { $0 = $1; }
	: &caltype &C_algo { $0 = $2; $0.man = $1; }
	;
&C_algo <run_params>
	: &C_nodefaults { $0 = $1; }
	: &algo * { $0 = C_defaults; $0.algo = $1; }
	;
&C_nodefaults <run_params>
	: &C_defaults { $0 = $1; }
	: NoDefaults * { $0 = run_reset; }
	;
&C_defaults <run_params>
	: * { $0 = C_defaults; }
	;

&T_type <run_params>
	: &T_defaults { $0 = $1; }
	: &testtype &T_algo { $0 = $2; $0.man = $1; }
	;
&T_algo <run_params>
	: &T_nodefaults { $0 = $1; }
	: &algo * { $0 = T_defaults; $0.algo = $1; }
	;
&T_nodefaults <run_params>
	: &T_defaults { $0 = $1; }
	: NoDefaults * { $0 = run_reset; }
	;
&T_defaults <run_params>
	: * { $0 = T_defaults; }
	;

&man <char *>
	: man1 { $0 = "man1"; }
	: man2 { $0 = "man2"; }
	;
&bulb <char *>
	: bulb %d (Enter bulb number 1-7) {
		if ( $2 < 1 || $2 > 7 ) {
		  msg( 2, "Invalid bulb number" );
		  CANCEL_LINE; /* Will work better on server */
		} else {
		  static char bulb[8];
		  sprintf( bulb, "bulb%d", $2 );
		  $0 = bulb;
		}
	  }
	: molecule %w { $0 = $2; }
	;
&radical <char *>
	: Radical %w (Enter Radical name) { $0 = $2; }
	: NoRadical { $0 = ""; }
	;
&algo <char *>
	: Algo %w (Enter Algo name) { $0 = $2; }
	: NoAlgo { $0 = ""; }
	;
# XS Reagent Detection
&XS <char *>
	: XS %w (Enter XS Detection Method) { $0 = $2; }
	;
# Radical Detection
&RA <char *>
	: RadDet %w (Enter Radical Detection Method) { $0 = $2; }
	;
&IR <char *>
	: IRDet %w (Enter IR Detector) { $0 = $2; }
	;
&caltype <char *>
	: Type %w (Enter Calibration Type) { $0 = $2; }
	;
&testtype <char *>
	: Type %w (Enter Test Type) { $0 = $2; }
	;
