# table.pm
#  Module to create html tables
# Needed still:
#   Support for colspan (for titles)
#   Support for justification (review how it's done in HTML)
#   Support for colors in text mode?
# Might like to have @table as an object. It could be built
# up and then output in multiple formats to multiple files...
# Would need to defer html-tables output to the End processing
# Structures need to get more complicated:
#  The table itself needs to record headings
#  (headings should be of the same form as the other calls)
#  %table 'options' 'rows'
#  Rows need to record options and cols also
#  Table::Begin => Table::New returning ref
#  Table::End => Table::Output taking the mode, filehandle

package Table;
use strict;

$Table::Mode = "html-tables";
my %table_modes =
  ( "html-tables" => 0, "html-lynx" => 1, "text" => 2 );
my $table_mode;
my $table_fh;
my @table;

sub Table::Begin {
  my ( $fh, $headings ) = @_;
  $table_fh = $fh;
  die "New Begin before End\n" if @table > 0;
  die "Undefined Table::Mode: $Table::Mode\n" unless
	defined $table_modes{$Table::Mode};
  $table_mode = $table_modes{$Table::Mode};
  if ( $table_mode == 0 ) {
	print $table_fh "<TABLE $headings>\n";
  } elsif ( $table_mode == 1 ) {
	print $table_fh "<PRE>\n";
  }
}

sub Table::NewRow {
  my ( %options ) = @_;
  my $options = "";
  foreach my $option ( keys(%options) ) {
	if ( $option eq "asis" ) {
	  $options .= " $options{$option}";
	} else {
	  $options .= " $option=$options{$option}";
	}
  }
  push( @table, [] );
  print $table_fh "<TR$options>\n" if $table_mode == 0;
}

sub Table::EndRow {
  print $table_fh "</TR>\n" if $table_mode == 0;
}

my @lsublist = (
  "<[^>]+>" => '',
  "&gt;" => '>',
  "&lt;" => '<',
  "&amp;" => '&',
);

my %cellopts = (
  'colspan' => 1,
  'rowspan' => 1,
  'align' => 1
);

sub Table::data {
  my ( $type, $text, %options ) = @_;
  my $options = "";
  my $cell = {};
  $cell->{'type'} = $type;
  foreach my $option ( keys(%options) ) {
	if ( $option eq "asis" ) {
	  $options .= " $options{$option}";
	} else {
	  $options .= " $option=$options{$option}";
	  $cell->{$option} = $options{$option}
		if $cellopts{$option};
	}
  }
  if ( $table_mode == 0 ) {
	$text = "<BR>" unless $text;
	print $table_fh "<$type$options>$text</$type>\n";
  } else {
	# Now determine the width and height
	my $ltext = $text;
	$ltext =~ s/<BR>/\n/g;
	chomp $ltext;
	$cell->{'lynxtext'} = [ split('\n', $ltext ) ];
	my @subs = @lsublist;
	while ( @subs ) {
	  my $from = shift(@subs);
	  my $to = shift(@subs);
	  $ltext =~ s/$from/$to/g;
	}
	$cell->{'ltext'} = [ split('\n', $ltext) ];
	$ltext = $cell->{'ltext'};
	$cell->{'height'} = @$ltext;
	my $width = 0;
	foreach my $line ( @$ltext ) {
	  $width = length($line) if length($line) > $width;
	}
	$cell->{'width'} = $width;
	$cell->{'text'} = $text;
	push( @{$table[$#table]}, $cell );
  }
}

sub Table::Head {
  Table::data( "TH", @_ );
}

sub Table::Cell {
  Table::data( "TD", @_ );
}

sub Table::End {
  my $midcolwid = 2;
  if ( $table_mode > 0 ) {
	my ( @cols, @rows ); # col widths and row heights
	# This is a 3-pass process (at the moment)
	#  Pass 1: Calculate column widths and row heights
	#          based on atomic cells
	#  Pass 2: Advance column widths for colspan
	#  Pass 3: Output
	
	# Pass 1: Calculate column widths and row heights
	foreach my $trow ( @table ) {
	  my $height = 0;
	  my $col = 0;
	  foreach my $cell ( @$trow ) {
		$cols[$col] = 0 unless $cols[$col];
		my $ncols = 1;
		if ( $cell->{'colspan'} ) {
		  $ncols = $cell->{'colspan'};
		  while ( $ncols > 1 ) {
			$col++;
			$cols[$col] = 0 unless $cols[$col];
			$ncols--;
		  }
		} else {
		  $cols[$col] = $cell->{'width'}
			if $cell->{'width'} > $cols[$col];
		  $height = $cell->{'height'}
			if $cell->{'height'} > $height;
		}
		$col += $ncols;
	  }
	  push( @rows, $height );
	}
	
	# Pass 2: Go through again and fixup any colspan, rowspan
	foreach my $trow ( @table ) {
	  my $col = 0;
	  foreach my $cell ( @$trow ) {
		if ( $cell->{'colspan'} ) {
		  my $ncols = $cell->{'colspan'};
		  my $spanwidth = $cols[$col];
		  foreach my $ecol ( 1 .. $ncols-1 ) {
			$spanwidth += $cols[$col+$ecol] + $midcolwid
			  if $cols[$col];
		  }
		  if ( $cell->{'width'} > $spanwidth ) {
			use integer;
			my $spaces = $cell->{'width'} - $spanwidth;
			my $acc = $spaces - ($ncols/2);
			foreach my $ecol ( 0 .. $ncols-1 ) {
			  my $nsp = ( $acc > 0 ) ? ( $acc + $ncols -1 )/$ncols : 0;
			  $cols[$col] += $nsp;
			  $acc += $spaces - $nsp * $ncols;
			}
		  }
		  $col += $ncols;
		}
	  }
	}
	# Now output:
	my $row = 0;
	while ( my $trow = shift(@table) ) {
	  my $height = $rows[$row];
	  foreach my $rowrow ( 0 .. $height-1 ) {
		my $rowtext = "";
		my $col = 0;
		foreach my $cell ( @$trow ) {
		  my $width = $cols[$col];
		  my $ncols = 1;
		  if ( $cell->{'colspan'} ) {
			$ncols = $cell->{'colspan'};
			foreach my $ecol ( 1 .. $ncols-1 ) {
			  $width += $cols[$col+$ecol] + $midcolwid
				if $cols[$col];
			}
		  }
		  my $ltext = $cell->{'ltext'}->[$rowrow] || "";
		  my $text = ( $table_mode == 1 ) ?
			$cell->{'lynxtext'}->[$rowrow] || "" : $ltext;
		  my $align = $cell->{'align'} ||
			( $cell->{'type'} eq "TH" ? 'center' : 'left' );
		  my $nsp = $width - length($ltext);
		  if ( $nsp > 0 ) {
			use integer;
			my $lsp;
			if ( $align eq 'center' ) { $lsp = $nsp/2; }
			elsif ( $align eq 'right' ) { $lsp = $nsp; }
			else { $lsp = 0; }
			my $rsp = $nsp - $lsp;
			$text = " " x $lsp . $text . " " x $rsp;
		  }
		  $text .= " " x $midcolwid;
		  $rowtext .= $text;
		  $col += $ncols;
		}
		$rowtext =~ s/\s*$//;
		print $table_fh "$rowtext\n";
	  }
	  $row++;
	}
  }
  if ( $table_mode == 0 ) {
	print $table_fh "</TABLE>\n";
  } elsif ( $table_mode == 1 ) {
	print $table_fh "</PRE>\n";
  }
}

__END__

Table::Mode
Table::Begin( OFILE, color, etc. )
Table::Head( Text, colspan, color, etc. )
Table::Cell( Text, colspan, rowspan, color, etc. )
Table::NewRow
Table::End

Table saved as an array of rows.
Each row is an array of cells.
Each cell contains { text, width, height, colspan, (rowspan) }
