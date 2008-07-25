%{
  #include "collect.h"
  #include "htrsim.h"
  #define IOMODE_INIT 27
  send_id Htr_id;
%}
&command
	: &collect_cmd {
		if ( Htr_id == 0 )
		  Htr_id = Col_send_init( "HtrData", &HtrData,
			sizeof( HtrData ), 0 );
		Col_send( Htr_id );
	  }
	;
&collect_cmd
	: Set &pointer %f * { *($2) = $3; }
	: Add %f to &pointer * { *($4) += $2; }
	;
&pointer <double *>
	: Heater Current { $0 = &HtrData.I; }
	: Heater Resistance { $0 = &HtrData.R; }
	: Heater Thermal Resistivity { $0 = &HtrData.Rt; }
	: Heater Thermal Mass { $0 = &HtrData.Ct; }
	: Ambient Temperature { $0 = &HtrData.Tamb; }
	: Proportional Gain { $0 = &HtrData.Gp; }
	: Integral Gain { $0 = &HtrData.Gi; }
	: Differential Gain { $0 = &HtrData.Gd; }
	: Maximum Integral Term { $0 = &HtrData.Simax; }
	: Temperature Set Point { $0 = &HtrData.Tset; }
	;
