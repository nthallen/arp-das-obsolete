function declare( name ) {
  if ( decls[ name ] == 0 ) {
	decls[ name ] = 1
	ndecls++
	names[ ndecls ] = name
	if ( printed == 0 ) {
	  print "%{"
	  print "  #include \"rtgapi.h\""
	  printed = 1
	}
	print "  rtg_t *" name "_rtg;"
  }
}

pass!=1 && printed != 0 {
  print "  void rtgext_init( void ) {"
  for ( i = 1; i <= ndecls; i++ ) {
	print "    " names[ i ] "_rtg = rtg_init( \"" names[i] "\" );"
  }
  print "  }"
  print "%}"
  printed = 0
}

/RTG_REPORT *\( *[A-Za-z_][A-Za-z0-9_]*/ {
  match( $0, "RTG_REPORT *\\( *[A-Za-z_][A-Za-z0-9_]*" )
  name = substr( $0, RSTART, RLENGTH )
  sub( "RTG_REPORT *\\( *", "", name )
  if ( pass == 1 ) {
    declare( name )
  } else {
	sub( "RTG_REPORT *\\( *[A-Za-z_][A-Za-z0-9_]*",
	  "rtg_report( " name "_rtg" );
	print
  }
  next
}

pass!=1 { print }
