# tmg2tmc
# RTG_REPORT( <name>, x, y)
#  rtg_t *<name>_rtg;
# RTG_REPORT( <name>[dim], x, y)
#  rtg_t *<name>_rtg[dim];
#  int <name>_rtginc;
# RTG_SCAN( <state>, <name>[dim], x, y)
#  same + depending on...
# RTG_TIME_SCAN( <state>, <name>[dim], y)
#  double <name>_rtgbt;
#
function declare( name, dim, state, time ) {
  if ( decls[ name ] == 0 ) {
	ndecls++
	decls[ name ] = ndecls
	names[ ndecls ] = name
	dims[ ndecls ] = dim
	nmstate[ name ] = state
	if ( state != "" && states[ state ] == 0 ) {
	  nstates++
	  stnames[ nstates ] = state
	  states[ state ] = nstates
	}
	if ( hdr1_printed == 0 ) {
	  print "%{"
	  print "  #include \"rtgapi.h\""
	  hdr1_printed = 1
	}
	printf "%s", "  rtg_t *" name "_rtg"
	if ( dim > 1 ) printf "%s", "[" dim "]"
	print ";"
	if ( dim > 1 ) print "  int " name "_rtginc = " dim-1 ";"
	if ( time != 0 && sttime[ state ] == 0 ) {
	  sttime[ state ] = 1
	  print "  double " state "_rtgbt;"
	  if ( hdr2_printed == 0 ) {
		print "  #include \"tmctime.h\""
		hdr2_printed = 1
	  }
	}
  }
}

{ scan_state=""
  time_cmd=0
  name=""
  dimension=1
}

pass != 1 && hdr1_printed == 1 {
  print "  void rtgext_init( void ) {"
  for ( i = 1; i <= ndecls; i++ ) {
	if ( dims[i] == 1 ) {
	  print "    " names[ i ] "_rtg = rtg_init( \"" names[i] "\" );"
	} else for ( j = 0; j < dims[i]; j++ ) {
	  printf "%s", "    " names[ i ] "_rtg[" j "] = rtg_init( \""
	  printf "%s", names[i] "/" j "\" );\n"
	}
  }
  print "  }"
  print "%}"
  for ( i = 1; i <= nstates; i++ ) {
	state = stnames[ i ];
	print "depending on ( " state " once ) {"
	for ( j = 1; j <= ndecls; j++ ) {
	  if ( nmstate[ names[ j ] ] == state ) {
		printf "%s", "  rtg_increment( " names[j] "_rtginc, "
		print  dims[j] " );"
	  }
	}
	if ( sttime[ state ] == 1 ) {
	  print "  " state "_rtgbt = dtime();"
	}
	print "}"
  }
  hdr1_printed = 2
}

/RTG_TIME_SCAN/ {
  time_cmd = 1
  sub( "RTG_TIME_SCAN", "RTG_SCAN" )
}
/RTG_SCAN *\( *[A-Za-z_][A-Za-z0-9_]* *,/ {
  match( $0, "RTG_SCAN *\\( *[A-Za-z_][A-Za-z0-9_]*" )
  scan_state = substr( $0, RSTART, RLENGTH )
  sub( "RTG_SCAN *\\( *", "", scan_state )
  sub( "RTG_SCAN *\\( *[A-Za-z_][A-Za-z0-9_]* *,", "RTG_REPORT(" )
}
/RTG_REPORT *\( *[A-Za-z_][A-Za-z0-9_]*/ {
  match( $0, "RTG_REPORT *\\( *[A-Za-z_][A-Za-z0-9_]*" )
  name = substr( $0, RSTART, RLENGTH )
  sub( "RTG_REPORT *\\( *", "", name )
  sub( "RTG_REPORT *\\( *[A-Za-z_][A-Za-z0-9_]*", "RTG_TOKEN:" )
}
/RTG_TOKEN: *\[ *[0-9]+ *\]/ {
  match( $0, "RTG_TOKEN: *\\[ *[0-9]+ *\\]" )
  dimension = substr( $0, RSTART, RLENGTH )
  sub( "RTG_TOKEN: *\\[ *", "", dimension )
  sub( " *\\]", "", dimension )
  sub( "RTG_TOKEN: *\\[ *[0-9]+ *\\]", "RTG_TOKEN:" )
}
/RTG_TOKEN:/ {
  cmd="rtg_report( " name "_rtg"
  if ( dimension > 1 ) cmd = cmd "[ " name "_rtginc ]"
  if ( time_cmd != 0 ) cmd = cmd ", dtime() - " scan_state "_rtgbt"
  sub( "RTG_TOKEN:", cmd )
}
pass==1 && name != "" {
  declare( name, dimension, scan_state, time_cmd )
}
/RTG_INCREMENT *\( *[A-Za-z_][A-Za-z0-9_]* *\)/ {
  match( $0, "RTG_INCREMENT *\\( *[A-Za-z_][A-Za-z0-9_]*" )
  name = substr( $0, RSTART, RLENGTH )
  sub( "RTG_INCREMENT *\\( *", "", name )
  ndecl = decls[ name ]
  if ( dims[ ndecl ] > 1 )
	cmd = "rtg_increment( " name "_rtginc, " dims[ ndecl ] " )"
  else
    cmd = ""	
  sub( "RTG_INCREMENT *\\( *[A-Za-z_][A-Za-z0-9_]* *\\)", cmd )
}
pass!=1 {
  if ( scan_state != "" )
	printf "%s", "depending on ( " scan_state " ) {\n  "
  print
  if ( scan_state != "" ) print "}"
}
