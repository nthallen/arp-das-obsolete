# table.pm
#  Module to create html tables
# Needed still:
#   Support for colspan (for titles)
#   Support for justification (review how it's done in HTML)
#   Support for colors in text mode?

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
  push( @table, [] );
  print $table_fh "<TR>\n" if $table_mode == 0;
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

sub Table::data {
  my ( $type, $text, %options ) = @_;
  my $options = "";
  my $cell = {};
  foreach my $option ( keys(%options) ) {
	if ( $option eq "asis" ) {
	  $options .= " $options{$option}";
	} else {
	  $options .= " $option=$options{$option}";
	  if ( $option eq "colspan" || $option eq "rowspan" ) {
		$cell->{$option} = $options{$option};
	  }
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
  # 
  if ( $table_mode > 0 ) {
	my ( @cols, @rows );
	# Calculate column widths and row heights
	# Assuming no colspan or rowspan
	foreach my $row ( @table ) {
	  my $height = 0;
	  foreach my $col ( 0 .. $#$row ) {
		my $cell = $row->[$col];
		$cols[$col] = 0 unless $cols[$col];
		$cols[$col] = $cell->{'width'}
		  if $cell->{'width'} > $cols[$col];
		$height = $cell->{'height'}
		  if $cell->{'height'} > $height;
	  }
	  push( @rows, $height );
	}
	# Go through again and fixup any colspan, rowspan
	foreach my $row ( 0 .. $#table ) {
	  my $height = $rows[$row];
	  foreach my $rowrow ( 0 .. $height-1 ) {
		my $rowtext = "";
		foreach my $col ( 0 .. $#{$table[$row]} ) {
		  my $cell = $table[$row]->[$col];
		  my $width = $cols[$col];
		  my $ltext = $cell->{'ltext'}->[$rowrow] || "";
		  my $text = ( $table_mode == 1 ) ?
			$cell->{'lynxtext'}->[$rowrow] || "" : $ltext;
		  $text .= " " x ( $width - length($ltext) + 2 );
		  $rowtext .= $text;
		}
		$rowtext =~ s/\s*$//;
		print $table_fh "$rowtext\n";
	  }
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
