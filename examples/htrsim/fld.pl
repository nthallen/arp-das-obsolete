require "box.pl";

$max_row = -1;
$max_col = -1;
sub check_maxes {
  local( $row, $col ) = @_;
  $col++;
  $max_row = $row if ( $row > $max_row );
  $max_col = $col if ( $col > $max_col );
}

sub define_attr {
  local($attr) = @_;
  if ( ! defined $attrs{$attr} && $attr ne "NORMAL" ) {
	push(@attrs, $attr);
	$attrs{$attr} = 1;
  }
}
sub string {
  local( $row, $col, $attr, $txt ) = @_;
  &define_attr( $attr );
  push( @strings, "$row $col$;$attr$;$txt" )
	if ( length($txt) > 0 );
  $col += length($txt);
  &check_maxes( $row+1, $col );
}
sub field {
  local( $row, $col, $width, $len, $attr, $txt ) = @_;
  $txt = $field{$txt} if ( defined $field{$txt} );
  if ( $txt ne "" ) {
	$nfields++;
	push( @fields, "$nfields $row $col $width $len$;$attr$;$txt" );
	while ( $len-- > 0 ) {
	  &string( $row++, $col, $attr, sprintf( "%${width}s", "" ) );
	}
  }
}
sub lfield {
  local( $row, $col, $width, $label, $txt, $unit ) = @_;
  local( $len );
  $len = length( $label );
  if ( $len > 0 ) {
	&string( $row, $col, "LABEL", "$label:" );
	$col += $len + 2;
  }
  &field( $row, $col, $width, 1, "FIELD", $txt );
  $col += $width;
  $len = length( $unit );
  if ( $len > 0 ) {
	$col += 2;
	&string( $row, $col, "UNITS", $unit );
	$col += $len;
  }
  $col;
}
sub center_string {
  local( $wid, $row, $col, $attr, $txt ) = @_;
  local( $slop, $slopa );
  $slop = $wid - length($txt);
  if ( $slop < 0 ) {
	warn "Text \"$txt\" too long for centering\n";
	$txt = sprintf( "%${wid}.${wid}s", $txt );
	$slop = 0;
  }
  $slop = int( $slop/2 );
  &string( $row, $col+$slop, $attr, $txt );
}
sub tblbegin {
  @tblsystems = @_;
  undef %tbl_len; # label, field, units
  undef @tbl_sens;
  undef %tbl; # title, hdr
}
sub itbl_length {
  local( $name, $length ) = @_;
  $tbl_len{$name} = $length if ( $length > $tbl_len{$name} );
}
sub tbltitle {
  local( $title ) = @_;
  $tbl{"title"} = $title;
}
sub tblhdr {
  # generate a row of column headers based on @cols
  local( $label ) = @_;
  $tbl{"hdr"} = $label;
  &itbl_length( "label", length($label) );
}
sub tblsensor {
  local( $label, $field, $fwidth, $units ) = @_;
  &itbl_length( "label", length($label) );
  &itbl_length( "field", $fwidth );
  &itbl_length( "units", length( $units ) );
  push( @tbl_sens, "$label$;$field$;$fwidth$;$units" );
}
sub tblend {
  local( $row, $col, $swap, $lines ) = @_;
  local( $cn, $trow, $srow, $scol, $sys );
  $col++ if ( $lines > 0 );
  $scol = $col;
  $trow = $row;
  $row++ if ( defined $tbl{"title"} || $lines > 0 );
  $srow = $row;
  if ( $swap == 0 ) {
	local( $lwid, $fwid, $uwid ) = @tbl_len{ "label", "field", "units" };
	if ( defined $tbl{"hdr"} ) {
	  &string( $row, $col, "HEADER", $tbl{"hdr"} );
	  $col += $lwid+2 if ( $lwid > 0 );
	  foreach $sys ( @tblsystems ) {
		&center_string( $fwid, $row, $col, "HEADER", $sys );
		$col += $fwid+2;
	  }
	  $row++;
	}
	local( $label, $field, $fwidth, $units, $fsub, $colhdr );
	foreach (@tbl_sens) {
	  ( $label, $field, $fwidth, $units ) = split(/$;/);
	  $col = $scol;
	  &string( $row, $col, "LABEL", "$label:" );
	  $col += $lwid + 2 if ( $lwid > 0 );
	  foreach $sys ( @tblsystems ) {
		$fsub = $field;
		$fsub =~ s/\*/$sys/g;
		&field( $row, $col + $fwid - $fwidth, $fwidth, 1, "FIELD", $fsub );
		$col += $fwid+2;
	  }
	  if ( $uwid > 0 ) {
		&string( $row, $col, "UNITS", $units );
		$col += $uwid;
	  } else {
		$col -= 2;
	  }
	  $row++;
	}
  } else {
	local( $colw );
	if ( defined $tbl{"hdr"} ) {
	  &string( $row++, $col, "HEADER", $tbl{"hdr"} );
	  $colw = length($tbl{"hdr"});
	  foreach $sys ( @tblsystems ) {
		&string( $row++, $col, "HEADER", "$sys:" );
		$colw = length( $sys ) if ( length($sys) > $colw );
	  }
	  $col += $colw + 2;
	}
	local( $label, $field, $units, $fsub );
	foreach (@tbl_sens) {
	  ( $label, $field, $colw, $units, $fsub ) = split(/$;/);
	  $row = $srow;
	  &center_string( $colw, $row++, $col, "LABEL", $label );
	  foreach $sys ( @tblsystems ) {
		$fsub = $field;
		$fsub =~ s/\*/$sys/g;
		&field( $row++, $col, $colw, 1, "FIELD", $fsub );
	  }
	  &center_string( $colw, $row++, $col, "UNITS", $units );
	  $col += $colw + 2;
	}
	$col -= 2;
  }
  &addbox( $scol-1, $col, $trow, $row, $tbl{"title"}, $lines );
  if ( $lines > 0 ) {
	$row++; $col++;
  }
  #&center_string( $col-$scol, $trow, $scol, "TITLE", $tbl{"title"} )
  #	if ( defined $tbl{"title"} );
  ( $row, $col );
}
# fld_end outputs all the fields and strings
sub fldend {
  &draw_boxes;
  $max_row++;
  print "/* form: lines, cols, pos_y, pos_x */\n" .
        "#FORM# $max_row $max_col 0 0\n";
  # sort attributes and output them
  # @attrs = sort @attrs;
  unshift( @attrs, "NORMAL" );
  for $attr (0..$#attrs) {
	$attrs{$attrs[$attr]} = $attr;
  }
  print "/* attribute type */\n" .
		"#ATTRIBUTES#";
  foreach $attr ( @attrs ) {
	print " $attr";
  }
  print "\n";

  # output fields, translating attributes
  print "/* fields: number, line, col, width, length, " .
		"attribute code, string */\n";
  local( $pos, $attr, $txt );
  foreach ( @fields ) {
	( $pos, $attr, $txt ) = split(/$;/);
	print "#FIELD# $pos $attrs{$attr} \"$txt\"\n";
  }

  # output strings, translating attributes
  print "/* strings: line, col, attribute code, string */\n";
  foreach ( @strings ) {
	( $pos, $attr, $txt ) = split(/$;/);
	print "#STRING $pos $attrs{$attr} \"$txt\"\n";
  }
}
1;
