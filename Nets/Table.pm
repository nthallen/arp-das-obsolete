# table.pm
#  Module to create html tables
# Supports
#   colspan
# Needed still:
#   Support for justification (review how it's done in HTML)
#   Support for rowspan
#   Support for nested tables
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

#----------------------------------------------------------------
# Want to build up table, then produce output in multiple modes
# Need to be able to strip out HTML tags.
# contents for now will be straight text with tags (later could
# be objects or collections of objects)
# options that matter are colspan
# tags that matter are <br>
#----------------------------------------------------------------
#  $table = Table::new( options );
#  $table->Head( contents, options );
#  $table->Cell( contents, options );
#  $table->NewRow( options );
#  $table->Output( mode );
#----------------------------------------------------------------
# table => { options => {}
#            rows => [ <row> ]
#          }
# row   => { options => {}
#            cols => [ <col> ] }
# col   => { options => {}
#            type => 'TH', 'TD'
#            ttext => <table_text>
#            lynxtext => [ <lynx_text> ]
#            ltext => [ <stripped_text> ]
#            width => <width>
#            height => <height>
#          }

package Table;
use strict;

$Table::Mode = "html-tables";
my %table_modes =
  ( "html-tables" => 0, "html-lynx" => 1, "text" => 2 );

sub Table::new {
  my ( %options ) = @_;
  my $table = {};
  $table->{options} = \%options;
  $table->{rows} = [];
  bless $table;
  return $table;
}

sub Table::NewRow {
  my ( $table, %options ) = @_;
  my $row = {};
  $row->{options} = \%options;
  $row->{cols} = [];
  push( @{$table->{rows}}, $row );
}

my @lsublist = (
  "<[^>]+>" => '',
  "&gt;" => '>',
  "&lt;" => '<',
  "&amp;" => '&',
);

sub Table::_data {
  my ( $type, $table, $text, %options ) = @_;
  my $cell = {};
  $cell->{type} = $type;
  $cell->{options} = \%options;
  $cell->{ttext} = $text;
  # Now determine the width and height
  my $ltext = $text;
  $ltext = "<B>$ltext</B>" if $type eq "TH";
  $ltext =~ s/<BR>/\n/g;
  chomp $ltext;
  $cell->{lynxtext} = [ split('\n', $ltext ) ];
  my @subs = @lsublist;
  while ( @subs ) {
	my $from = shift(@subs);
	my $to = shift(@subs);
	$ltext =~ s/$from/$to/g;
  }
  $cell->{ltext} = [ split('\n', $ltext) ];
  $ltext = $cell->{ltext};
  $cell->{height} = @$ltext;
  my $width = 0;
  foreach my $line ( @$ltext ) {
	$width = length($line) if length($line) > $width;
  }
  $cell->{width} = $width;
  my $trows = $table->{rows};
  my $lastrow = $trows->[$#$trows];
  push( @{$lastrow->{cols}}, $cell );
}

sub Table::Head {
  Table::_data( "TH", @_ );
}

sub Table::Cell {
  Table::_data( "TD", @_ );
}

sub Table::optstring {
  my ( $opts ) = @_;
  join '', map { " $_=\"$opts->{$_}\"" } keys %$opts;
}

sub Table::Output {
  my ( $table, $mode ) = @_;
  my @OUTPUT;
  die "Undefined mode: $mode\n" unless
	defined $table_modes{$mode};
  my $table_mode = $table_modes{$mode};
  if ( $table_mode == 0 ) {
	my $options = Table::optstring($table->{options});
	push @OUTPUT, "<TABLE$options>\n";
  } elsif ( $table_mode == 1 ) {
	push @OUTPUT, "<PRE>\n";
  }
  my $midcolwid = 2;
  if ( $table_mode == 0 ) {
	foreach my $trow ( @{$table->{rows}} ) {
	  my $options = Table::optstring($trow->{options});
	  push @OUTPUT, "<TR$options>\n";
	  foreach my $cell ( @{$trow->{cols}} ) {
		my $text = $cell->{ttext} || "<BR>";
		my $type = $cell->{type};
		my $options = Table::optstring($cell->{options});
		push @OUTPUT, "<$type$options>$text</$type>\n";
	  }
	  push @OUTPUT, "</TR>\n";
	}
  } else {
	my ( @cols, @rows ); # col widths and row heights
	# This is a 3-pass process (at the moment)
	#  Pass 1: Calculate column widths and row heights
	#          based on atomic cells
	#  Pass 2: Advance column widths for colspan [and rowspan]
	#  Pass 3: Output
	
	# Pass 1: Calculate column widths and row heights
	foreach my $rowdef ( @{$table->{rows}} ) {
	  my $trow = $rowdef->{cols};
	  my $height = 0;
	  my $col = 0;
	  foreach my $cell ( @$trow ) {
		$cols[$col] = 0 unless $cols[$col];
		my $ncols = 1;
		if ( $cell->{options}->{colspan} ) {
		  $ncols = $cell->{options}->{colspan};
		  while ( $ncols > 1 ) {
			$col++;
			$cols[$col] = 0 unless $cols[$col];
			$ncols--;
		  }
		} else {
		  $cols[$col] = $cell->{width}
			if $cell->{width} > $cols[$col];
		}
		$height = $cell->{height}
		  if $cell->{height} > $height;
		$col += $ncols;
	  }
	  push( @rows, $height );
	}
	
	# Pass 2: Go through again and fixup any colspan, rowspan
	foreach my $rowdef ( @{$table->{rows}} ) {
	  my $trow = $rowdef->{cols};
	  my $col = 0;
	  foreach my $cell ( @$trow ) {
		if ( $cell->{options}->{colspan} ) {
		  my $ncols = $cell->{options}->{colspan};
		  my $spanwidth = $cols[$col];
		  foreach my $ecol ( 1 .. $ncols-1 ) {
			$spanwidth += $cols[$col+$ecol] + $midcolwid
			  if $cols[$col];
		  }
		  if ( $cell->{width} > $spanwidth ) {
			use integer;
			my $spaces = $cell->{width} - $spanwidth;
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

	# Pass 3: Output:
	my $row = 0;
	foreach my $rowdef ( @{$table->{rows}} ) {
	  my $trow = $rowdef->{cols};
	  my $height = $rows[$row];
	  foreach my $rowrow ( 0 .. $height-1 ) {
		my $rowtext = "";
		my $col = 0;
		foreach my $cell ( @$trow ) {
		  my $width = $cols[$col];
		  my $ncols = 1;
		  if ( $cell->{options}->{colspan} ) {
			$ncols = $cell->{options}->{colspan};
			foreach my $ecol ( 1 .. $ncols-1 ) {
			  $width += $cols[$col+$ecol] + $midcolwid
				if $cols[$col];
			}
		  }
		  my $ltext = $cell->{ltext}->[$rowrow] || "";
		  my $text = ( $table_mode == 1 ) ?
			$cell->{lynxtext}->[$rowrow] || "" : $ltext;
		  my $align = $cell->{options}->{align} ||
			$cell->{options}->{ALIGN} ||
			( $cell->{type} eq "TH" ? 'center' : 'left' );
		  $align = lc($align);
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
		push @OUTPUT, "$rowtext\n";
	  }
	  $row++;
	}
  }
  if ( $table_mode == 0 ) {
	push @OUTPUT, "</TABLE>\n";
  } elsif ( $table_mode == 1 ) {
	push @OUTPUT, "</PRE>\n";
  }
  join "", @OUTPUT;
}

__END__

