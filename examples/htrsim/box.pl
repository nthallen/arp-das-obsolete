# Save all box coordinates
# Evaluate each box edge character against all the boxes

%char = (
  "0011", "\xB3",
  "1011", "\xB4",
  "2011", "\xB5",
  "1022", "\xB6",
  "1002", "\xB7",
  "2001", "\xB8",
  "2022", "\xB9",
  "0022", "\xBA",
  "2002", "\xBB",
  "2020", "\xBC",
  "1020", "\xBD",
  "2010", "\xBE",
  "1001", "\xBF",
  "0110", "\xC0",
  "1110", "\xC1",
  "1101", "\xC2",
  "0111", "\xC3",
  "1100", "\xC4",
  "1111", "\xC5",
  "0211", "\xC6",
  "0122", "\xC7",
  "0220", "\xC8",
  "0202", "\xC9",
  "2220", "\xCA",
  "2202", "\xCB",
  "0222", "\xCC",
  "2200", "\xCD",
  "2222", "\xCE",
  "2210", "\xCF",
  "1120", "\xD0",
  "2201", "\xD1",
  "1102", "\xD2",
  "0120", "\xD3",
  "0210", "\xD4",
  "0201", "\xD5",
  "0102", "\xD6",
  "1122", "\xD7",
  "2211", "\xD8",
  "1010", "\xD9",
  "0101", "\xDA",
  "1000", "\xC4",
  "0100", "\xC4",
  "0010", "\xB3",
  "0001", "\xB3",
  "2000", "\xCD",
  "0200", "\xCD",
  "0020", "\xBA",
  "0002", "\xBA",
);

sub addbox {
  local( $left, $right, $top, $bot, $title, $lines ) = @_;
  local( $twid, $slop, $tleft );

  if ( $lines > 0 ) {
	push( @hlines, "$top,$left,$right,$lines" );
	push( @hlines, "$bot,$left,$right,$lines" );
	push( @vlines, "$left,$top,$bot,$lines" );
	push( @vlines, "$right,$top,$bot,$lines" );
  }

  $twid = length($title);
  if ( $twid > 0 ) {
	$slop = $right-$left+1-$twid;
	$tleft = $left + int($slop/2) - 1;
	# warn "twid,left,right,tleft = $twid $left $right $tleft\n";
	&string($top, $tleft+1, "TITLE", $title );
	$tright = $tleft + $twid + 1;
	$tleft++; $tright--;
	push( @hlines, "$top,$tleft,$tright,3" );
  }
}

sub eval {
  local( $row, $col ) = @_;
  local( $l, $r, $t, $b ) = ( 0, 0, 0, 0 );
  local( $trace );
  $trace = ( $row == 10 );
  foreach (@hlines) {
	local($hrow,$left,$right,$lines) = split(/,/);
	if ( $row == $hrow ) {
	  # warn "eval $row $col: $hrow $left $right $lines\n" if ( $trace );
	  if ( $col >= $left && $col < $right && $lines > $r ) {
		$r = $lines;
		# warn "eval right\n" if ( $trace );
	  }
	  if ( $col > $left && $col <= $right && $lines > $l ) {
		$l = $lines;
		# warn "eval left\n" if ( $trace );
	  }
	}
  }
  foreach (@vlines) {
	local($vcol,$top,$bot,$lines) = split(/,/);
	if ( $col == $vcol ) {
	  if ( $row >= $top && $row < $bot && $lines > $b ) {
		$b = $lines;
	  }
	  if ( $row > $top && $row <= $bot && $lines > $t ) {
		$t = $lines;
	  }
	}
  }
  $char = "$l$r$t$b";
  # warn "eval $row $col = $char\n" if ( $col == 6 );
  $char = "0000" if ( $char =~ /3/);
  $char{ "$char" };
}
sub draw_hline {
  local( $row, $left, $right, $lines ) = @_;
  local( $line, $col, $char );
  $line = "";
  foreach $col ( $left..$right ) {
	$char = &eval( $row, $col );
	if ( $char eq "" ) {
	  if ( $line ne "" ) {
		&string( $row, $left, "BOX", $line );
		$line = "";
	  }
	  $left = $col+1;
	} else {
	  $line .= $char;
	}
  }
  &string( $row, $left, "BOX", $line );
}
sub draw_vline {
  local( $col, $top, $bot, $lines ) = @_;
  local( $char, $row );
  foreach $row ( $top..$bot ) {
	$char = &eval( $row, $col );
	&string( $row, $col, "BOX", $char );
  }
}
sub draw_boxes {
  foreach (@hlines) {
	local($row,$left,$right,$lines) = split(/,/);
	&draw_hline( $row, $left, $right );
  }
  foreach (@vlines) {
	local($col,$top,$bot,$lines) = split(/,/);
	&draw_vline( $col, $top, $bot );
  }
}
1;
