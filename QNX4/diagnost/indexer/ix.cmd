&command
	: &indexer_cmd
	;
&indexer_cmd
	: Drive &drive &direction &steps * { indxr_drive($2,$3,$4); }
	: Scan &drive &direction &steps by %d (Enter Steps per Step) *
		{ indxr_scan($2,$3,$4,$6); }
	: Stop &drive * { indxr_stop($2); }
	: Drive &drive Online * { indxr_online($2); }
	: Drive &drive Offline * { indxr_offline($2); }
	: Drive &drive Altline * { indxr_altline($2); }
	: Preset &drive Position to &steps * { indxr_preset($2, $5); }
	: Set &chop_drive Online Position %d (Enter Online Position) *
		{ indxr_set_online($2, $5); }
	: Set &chop_drive Online Delta
		%d (Enter positive number of steps between dithered online positions) *
		  { indxr_online_delta($2, $5); }
	: Set &chop_drive Offline Delta
		%d (Enter signed number of steps from Online to Offline position) *
		  { indxr_offline_delta($2, $5); }
	: Set &chop_drive Altline Delta
		%d (Enter signed number of steps from Online to Altline position) *
		  { indxr_altline_delta($2, $5); }
	: Move &chop_drive Online Position Out * { indxr_move_out($2); }
	: Move &chop_drive Online Position In * { indxr_move_in($2); }
	: Peakup On * { indxr_cmd(IX_SET_NO_LOOPS, 0, 0, 0); }
	: Peakup Off * { indxr_cmd(IX_SET_NO_LOOPS, 0, 1, 0); }
	;
&chop_drive <byte_t>
	: &drive { $0 = $1; }
	;
&drive <byte_t>
	: drive %d (Enter Drive Number from 0-5) {
		if ($2 < 0 || $2 > 5) $0 = 0;
		else $0 = $2;
	  }
	;
&direction <byte_t>
	: In { $0 = IX_IN; }
	: Out { $0 = IX_OUT; }
	: To { $0 = IX_TO; }
	;
&steps <step_t>
	: %d (Enter Number of Steps or Step Number) { $0 = $1; }
	;
