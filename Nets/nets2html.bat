#! /usr/local/bin/perl -w

# Generates the following:
#  CABLES.html
#  index.html
#  SIG*.html
#  <Comp>/index.html
#  <Comp>/<conn>.html

use FindBin;
use lib "$FindBin::Bin";
use SIGNAL;
use NETSLIB qw(open_nets mkdirp );
use CGI qw(:all -no_debug );
use Table;
use HTML;

SIGNAL::load_signals();

my @slices = ( "A-C", "D", "E-M", "N-R", "S", "T-Z" );
my %first_pin;

mkdirp( "html" );

{
  open( STDOUT, ">html/CABLES.html" ) ||
	die "Unable to open html/CABLES.html\n";

  print
	start_html(
	  '-title' => "$SIGNAL::global{Exp} Cable Definitions",
	  '-author' => "allen\@huarp.harvard.edu"
	),
	"\n",
	center(
	  h2( "$SIGNAL::global{Experiment}<BR>Cable Definitions" )),
	"\n<UL>\n";

  my @cables =
	map $_->[0], sort {
	  $a->[1] cmp $b->[1] ||
	  $a->[2] <=> $b->[2] ||
	  $a->[0] cmp $b->[0]
	} map {
	  $_ =~ m/^(.*\D)(\d*)$/;
	  [ $_, $1 || '', $2 || 0 ]
	} keys %SIGNAL::cable;

  my $t = Table::new( BORDER => 1 );
  $t->NewRow;
  $t->Head( 'Alias' );
  $t->Head( 'Comp' );
  $t->Head( 'Conn' );
  $t->Head( 'Description' );
  foreach my $cable ( @cables ) {
	my $cableconns = $SIGNAL::cable{$cable};
	foreach my $conncomp ( @$cableconns ) {
	  my ( $conn, $comp ) = SIGNAL::split_conncomp( $conncomp );
	  $t->NewRow;
	  if ( $comp ) {
		my $comptype = $SIGNAL::comp{$comp}->{type};
		my $conndef = $SIGNAL::comptype{$comptype}->{conn}->{$conn};
		my $conntype = $conndef->{type} || "";
		my $conndesc = $conndef->{desc} || "";
		my $alias = SIGNAL::make_conncomp( $conn, $comp );
		if ( $SIGNAL::comp{$comp}->{alias} &&
			 $SIGNAL::comp{$comp}->{alias}->{$conn} ) {
		  $alias = $SIGNAL::comp{$comp}->{alias}->{$conn};
		  my ( $aconn, $acomp ) =
			SIGNAL::split_conncomp( $alias );
		  $alias = SIGNAL::make_conncomp( $aconn, $acomp )
			if $aconn;
		}
		$t->Cell( $alias );
		$t->Cell( "<A HREF=\"$comp/index.html\">$comp</A>" );
		$t->Cell( "<A HREF=\"$comp/$conn.html\">$conn</A>" );
		$t->Cell( $conndesc );
	  } else {
		warn "Unable to understand conncomp '$conncomp'\n";
		$t->Cell( $conncomp );
	  }
	}
  }
  print
	$t->Output( 'html-tables' ),
	HTML::trailer( '', 'CABLES' ),
	end_html, "\n";
  close STDOUT || warn "Error closing html/CABLES.html\n";
  print STDERR "html/CABLES.html written\n";
}

#----------------------------------------------------------------
# Generate index.html
#----------------------------------------------------------------
{ my %part_html;

  open( STDOUT, ">html/index.html" ) ||
	die "Unable to open html/index.html\n";
  print
	start_html(
	  '-title' => "$SIGNAL::global{Exp} Electrical Systems",
	  '-author' => "allen\@huarp.harvard.edu"
	),
	"\n",
	center(
	  h2( "$SIGNAL::global{Experiment}<BR>Electrical Systems" ));
  if ( open(IFILE, "<PARTS.ORG") ) {
	print "\n<UL>\n";
	my $lev = 0;
	my ( $newlev, $string );
	ORGFILE:
	while (<IFILE>) {
	  chop;
	  m/^( *)(.*)$/;
	  $newlev = length($1);
	  $string = $2;
	  while ( $newlev > $lev ) {
		print "<UL>\n";
		$lev++;
	  }
	  while ( $newlev < $lev ) {
		print "</UL>\n";
		$lev--;
	  }
	  last ORGFILE if ( $string eq "" );
	  print "<LI>" if ( $lev > 0 );
	  if ( $string =~ m/^<([A-Za-z_0-9]+)>$/ ) {
		my $comp = $1;
		die "Unknown comp '$comp' in PARTS.org"
		  if ( ! defined $SIGNAL::comp{$comp} );
		print
		  "<A HREF=$comp/index.html>$comp</A>: ",
		  "$SIGNAL::comp{$comp}->{desc}\n";
		$part_html[$comp_key{$comp}] = 1;
	  } elsif ( $string =~ m/^<([A-Za-z_0-9:]+)>$/ ) {
		foreach my $comp ( split( /:/, $1 ) ) {
		  die "Unknown comp '$comp' in PARTS.org\n"
			if ( ! defined $SIGNAL::comp{$comp} );
		  print
			"<A HREF=$comp/index.html>$comp</A> ";
		  $part_html[$comp_key{$_}] = 1;
		}
		print "\n";
	  } else {
		print "$string\n";
	  }
	}
	close IFILE;
	while ( 0 < $lev ) {
	  print "</UL>\n";
	  $lev--;
	}
  }
  my @uncat = grep !defined($part_html{$_}), sort keys %SIGNAL::comp;
  if ( @uncat > 0 ) {
	print "<H3>Uncategorized Components</H3>\n";
	print "<UL>\n";
	foreach my $comp ( @uncat ) {
	  print
		"<LI><A HREF=$comp/index.html>$comp</A>: ",
		$SIGNAL::comp{$comp}->{desc} || '', "\n";
	}
	print "</UL>\n";
  }
  print HTML::trailer( "", "index" ), end_html;
  close STDOUT || warn "Error writing html/index.html\n";
  print STDERR "html/index.html written\n";
}

#----------------------------------------------------------------
# Process Components
#----------------------------------------------------------------
# foreach my $comp ( "MDP" ) {
foreach my $comp ( keys %SIGNAL::comp ) {
  mkdirp( "html/$comp" );
  my $comptype = $SIGNAL::comp{$comp}->{type} ||
	die "No comptype for component ${comp}\n";
  my $ct = $SIGNAL::comptype{$comptype} ||
	die "No comptype{} for component '${comp}' type '$comptype'\n";
  my $CompDesc = $SIGNAL::comp{$comp}->{desc} || $ct->{desc} || '';
  my ( %conn, %pin, %others );
  SIGNAL::load_netlist( $comptype, $comp, \%conn, \%pin, \%others );
  open( STDOUT, ">html/$comp/index.html" ) ||
	die "Unable to write to html/$comp/index.html\n";
  print
	start_html(
	  '-title' => "$SIGNAL::global{Exp} $CompDesc $comp",
	  '-author' => "allen\@huarp.harvard.edu"
	),
	"\n",
	center(
	  h2( "$SIGNAL::global{Experiment}<BR>$CompDesc $comp" )),
	"\n";
  my $t = Table::new( BORDER => 1 );
  $t->NewRow;
  $t->Head( "REFDES" );
  $t->Head( "Type" );
  $t->Head( "Description" );
  foreach my $conn ( @{$SIGNAL::comptype{$comptype}->{conns}} ) {
	my $conntype = $ct->{conn}->{$conn}->{type} || '';
	my $conndesc = $ct->{conn}->{$conn}->{desc} || '';
	$t->NewRow;
	$t->Cell( "<A HREF=$conn.html>$conn</A>" );
	$t->Cell( $conntype );
	$t->Cell( $conndesc );
  }
  if ( scalar(keys %others) > 0 ) {
	$t->NewRow;
	$t->Cell( "<A HREF=other.html>Others</A>" );
	$t->Cell( "" );
	$t->Cell( "Other Components" );
  }
  print
	"<center>\n",
	$t->Output('html-tables'),
	"</center>\n",
	HTML::trailer( "../", "", "[$comp]\n" ),
	end_html;
  close(STDOUT) || warn "Error closing html/$comp/index.html\n";
  print STDERR "html/$comp/index.html written\n";

  foreach my $conn ( @{$SIGNAL::comptype{$comptype}->{conns}} ) {
	my $conntype = $ct->{conn}->{$conn}->{type} || '';
	my $conndesc = $ct->{conn}->{$conn}->{desc} || '';
	my $myconncomp = SIGNAL::make_conncomp( $conn, $comp );
	my @pins;
	SIGNAL::define_pins( $conntype, \@pins );
	open( STDOUT, ">html/$comp/$conn.html" ) ||
	  die "Unable to write to html/$comp/$conn.html\n";
	my $title = "$SIGNAL::global{Experiment}<BR>$CompDesc $comp<BR>\n";
	$title .= "Connector $conn $conntype";
	$title .= "\n<BR>$conndesc" if $conndesc;
	print
	  start_html(
		'-title' => "$SIGNAL::global{Exp} $comp $conn",
		'-author' => "allen\@huarp.harvard.edu"
	  ),
	  "\n",
	  center( h2( $title ) ),
	  "\n";

	#----------------------------------------------------------------
	# Extract Cable info from database
	#----------------------------------------------------------------
	my $follow = '';
	if ( $SIGNAL::comp{$comp}->{cable}->{$conn} ) {
	  my $cable = $SIGNAL::comp{$comp}->{cable}->{$conn};
	  my @OUTPUT;
	  die "No cable definition for $conn:$comp\n" unless
		defined $SIGNAL::cable{$cable};
	  my @cdef = @{$SIGNAL::cable{$cable}};

	  # Rotate the list so this conn is last:
	  { my @save;
		while ( my $conncomp = shift @cdef ) {
		  if ( $conncomp eq $myconncomp ) {
			push( @cdef, @save );
			last;
		  } else {
			push( @save, $conncomp );
		  }
		}
	  }
	  if ( @cdef > 0 ) {
		push @OUTPUT, "Connected via cable $cable to\n";
		my @cdefs;
		foreach my $conncomp ( @cdef ) {
		  my ( $cconn, $ccomp ) = SIGNAL::split_conncomp( $conncomp);
		  $cconn || die "$SIGNAL::context: Bad conncomp '$conncomp'\n";
		  push( @cdefs, "<A HREF=\"../$ccomp/$cconn.html\">$cconn</A>" .
			  ":<A HREF=\"../$ccomp/index.html\">$ccomp</A>" );
		}
		while ( my $cdef = shift @cdefs ) {
		  push @OUTPUT, $cdef;
		  if ( @cdefs > 1 ) {
			push @OUTPUT, ", ";
		  } elsif ( @cdefs == 1 ) {
			push @OUTPUT, " and ";
		  }
		}
	  } else {
		push @OUTPUT, "No Cable Defined\n";
	  }
	  print
		p(join "", @OUTPUT ), "\n";
	  if ( $cdef[0] ) {
		my ( $cconn, $ccomp ) = SIGNAL::split_conncomp($cdef[0]);
		$cconn || die "$SIGNAL::context: Bad conncomp '$cdef[0]'\n";
		$follow = "<A HREF=\"../$ccomp/$cconn.html#P%s\">$cdef[0].%s</A>";
	  }
	} else {
	  print
		p("No cable information."), "\n";
	}

	$t = Table::new( BORDER => 1);
	$t->NewRow;
	$t->Head('Pin');
	$t->Head('Signal');
	$t->Head('Link-To');
	$t->Head('Follow');
	foreach my $pin ( @pins ) {
	  $t->NewRow;
	  $t->Cell( "<A NAME=\"P$pin\">$pin</A>" );
	  my ( $gsignal, $link ) =
		get_sig_link( $conn, $comp, $pin, \%conn, \%pin, \%others );
	  $t->Cell( $gsignal );
	  $t->Cell( $link );
	  if ( $follow ) {
		$t->Cell( sprintf( $follow, $pin, $pin ) );
	  }
	}
	print center( "\n", $t->Output( 'html-tables' ) ), "\n";
	print
	  HTML::trailer( "../", "", "[<A HREF=\"index.html\">$comp</A>]\n" ),
	  end_html;
	close STDOUT || warn "Error closing html/$comp/$conn.html\n";
	print STDERR "html/$comp/$conn.html written\n";
  }
  if ( scalar(keys %others) > 0 ) {
	#----------------------------------------------------------------
	# Generate other.html
	#----------------------------------------------------------------
	if ( scalar(keys %others) > 0 ) {
	  my @others =
		map { $_->[0] }
		  sort { $a->[1] cmp $b->[1] || $a->[2] <=> $b->[2] }
			map { m/^(.*\D)(\d*)$/ || die; [ $_, $1, $2 ] }
			  keys %others;
	  mkdirp( "html/$comp" );
	  open( STDOUT, ">html/$comp/other.html" ) ||
		die "Unable to create html/$comp/other.html\n";
	  my $title = "$SIGNAL::global{Experiment}<BR>$CompDesc $comp<BR>\n";
	  $title .= "Other Components";
	  print
		start_html(
		  '-title' => "$SIGNAL::global{Exp} $comp Other Components",
		  '-author' => "allen\@huarp.harvard.edu"
		),
		"\n",
		center( h2( $title ) ),
		"\n";

	  my @headings;
	  my $curr_heading = "";
	  foreach my $refdes ( @others ) {
		$refdes =~ m/^(.*\D)\d*$/;
		if ( $1 ne $curr_heading ) {
		  $curr_heading = $1;
		  print "[<A HREF=\"#HD-$curr_heading\">$curr_heading</A>]\n";
		  push( @headings, $refdes );
		}
	  }
	  print "</P>\n";

	  my $table = Table::new( BORDER => 1 );
	  $table->NewRow;
	  $table->Head( "REFDES" ); # was 6, 7, 10%
	  $table->Head( "Type" ); # was 4, 8, 10%
	  $table->Head( "Package" );
	  foreach my $refdes ( @others ) {
		$refdes =~ m/^(.*\D)\d*$/ || die;
		my $heading = $1;
		if ( @headings && $refdes eq $headings[0] ) {
		  $table->NewRow;
		  $table->Cell( "<A NAME=\"HD-$heading\">$1</A>", COLSPAN => 3 );
		  shift(@headings);
		}
		$table->NewRow;
		$table->Cell( "<A HREF=\"$heading.html#$refdes\">$refdes</A>" );
		my ( $sym, $pkg ) = split( '@', $others{$refdes} );
		$table->Cell( $sym );
		$table->Cell( $pkg );
	  }
	  print
		$table->Output( 'html-tables' ),
		"[<A HREF=index.html>$comp</A>]\n",
		HTML::trailer( '../', '', "[<A HREF=\"index.html\">$comp</A>]\n" ),
		end_html, "\n";
	  close STDOUT ||
		warn "Error closing html/$comp/other.html\n";
	  print STDERR "html/$comp/other.html written\n";
	  undef $table;

	  my $curfile = '';
	  foreach my $refdes ( @others ) {
		$refdes =~ m/^(.*\D)\d*$/;
		if ( $1 ne $curfile ) {
		  if ( $curfile ) {
			print "[<A HREF=index.html>$comp</A>]\n",
			  HTML::trailer( '../', '', "[<A HREF=\"index.html\">$comp</A>]\n" ),
			  end_html, "\n";
			close STDOUT ||
			  warn "Error closing html/$comp/$curfile.html\n";
			print STDERR "html/$comp/$curfile.html written\n";
		  }
		  $curfile = $1;
		  open( STDOUT, ">html/$comp/$curfile.html" ) ||
			die "Unable to create html/$comp/$curfile.html\n";
		  my $title = "$SIGNAL::global{Experiment}<BR>$CompDesc $comp<BR>\n";
		  $title .= "Other Components: '$curfile'";
		  print
			start_html(
			  '-title' =>
				"$SIGNAL::global{Exp} $comp Other Components '$curfile'",
			  '-author' => "allen\@huarp.harvard.edu"
			),
			"\n",
			center( h2( $title ) ),
			"\n";
		}
		my ( $sym, $pkg_type ) = split '@', $others{$refdes};
		print
		  "<H3><A NAME=\"$refdes\">$refdes</A> $sym $pkg_type</H3>\n";
		my $table = Table::new( BORDER => 1 );
		$table->NewRow;
		$table->Head( "Pin" ); # was 10%
		$table->Head( "Signal" );
		$table->Head( "Link To" );
		my @pins =
		  map { $_->[0] }
		  sort { $a->[1] cmp $b->[1] || $a->[2] <=> $b->[2] }
		  map { m/^(.*\D)?(\d*)$/; [ $_, $1 || '', $2 || '' ] }
			keys %{$conn{$refdes}};
		SIGNAL::define_pins( $pkg_type, \@pins );
		foreach my $pin ( @pins ) {
		  $table->NewRow;
		  $table->Cell( "<A NAME=\"P$refdes.$pin\">$pin</A>" );
		  my ( $gsignal, $link ) =
			get_sig_link( $refdes, $comp, $pin, \%conn, \%pin,
							\%others );
		  $table->Cell( $gsignal ); # link to signal def?
		  $table->Cell( $link );
		}
		print $table->Output( 'html-tables' );
	  }
	  if ( $curfile ) {
		print "[<A HREF=index.html>$comp</A>]\n",
		  HTML::trailer( '../', '', "[<A HREF=\"index.html\">$comp</A>]\n" ),
		  end_html, "\n";
		close STDOUT ||
		  warn "Error closing html/$comp/$curfile.html\n";
		print STDERR "html/$comp/$curfile.html written\n";
	  }
	}
  }
}

sub get_link {
  my ( $comp, $conn, $pin, $pother ) = @_;
  my $href;
  $comp .= "/" if $comp;
  if ( defined $pother->{$conn} ) {
	$conn =~ m/^(.*\D)\d*$/ || die "Could not match '$conn'";
	$href = "$comp$1.html#P$conn.$pin";
  } else {
	$href = "$comp$conn.html#P$pin";
  }
  "<A HREF=\"$href\">$conn.$pin</A>";
}

sub get_sig_link {
  my ( $conn, $comp, $pin, $pconn, $ppin, $pother ) = @_;
  my ( $signal, $gsignal, $link ) = ( '', '', '' );
  if ( $pconn->{$conn} && $pconn->{$conn}->{$pin} ) {
	$signal =  $pconn->{$conn}->{$pin};
	my $lsignal = "$signal($comp)";
	$first_pin{$lsignal} =
		get_link( $comp, $conn, $pin, $pother )
	  unless $first_pin{$lsignal};
	$gsignal = $SIGNAL::globsig{$lsignal};
	if ( $gsignal ) {
	  my $slice = pick_slice($gsignal);
	  my $addr = SIGNAL::get_address($lsignal);
	  $addr = " @ $addr" if $addr;
	  $gsignal =
		"<A HREF=\"../SIG_$slice.html#$gsignal\">$gsignal</A>$addr";
	} else {
	  $gsignal = $lsignal;
	}
	my $cpin = "$conn.$pin";
	$link = $ppin->{$signal}->{$cpin} || '';
	$link = '' if $link eq $cpin;
	if ( $link =~ m/^(\w+)\.(\w+)$/ ) {
	  $link = get_link( '', $1, $2, $pother );
	}
  }
  ( $gsignal, $link );
}

#----------------------------------------------------------------
# Generate Signal files:
#   sort keys SIGNAL::sighash (see global)
# $pat = '^([~\$]?)([-+\w]*[-+a-zA-Z_])(\d*)(\((\w+)\))?$';
# my ( $prefix, $name, $index, $comp ) = ( $1, $2, $3, $4 );
#----------------------------------------------------------------
sub sortsigs {
  map $_->[0],
	sort {
	  $a->[1] cmp $b->[1] || # first send locals to the end
	  "\U$a->[3]" cmp "\U$b->[3]" || # first by name case-insensitive
	  $a->[3] cmp $b->[3] || # then case-sensitive
	  $a->[4] <=> $b->[4] || # then index
	  $a->[5] cmp $b->[5] || # them component
	  $a->[2] cmp $b->[2] # then polarity
	}
	  map { $_ =~ /^(\$?)(~?)([-+\w]*[-+a-zA-Z_])(\d*)(\((\w+)\))?$/ ||
			  die;
			[ $_, $1 || '', $2 || '', $3 || '', $4 || 0, $5 || '' ]; }
		@_;
}

#----------------------------------------------------------------
# Generate Signal files
#----------------------------------------------------------------
my %slice_begun;
my $currslice = '';
{ my @globsigs = sortsigs( keys %SIGNAL::sighash );
  foreach my $globsig ( @globsigs ) {
	open_slice( $globsig );
	my $addr = SIGNAL::get_address($globsig);
	$addr = " @ $addr" if $addr;
	my $desc = SIGNAL::get_sigdesc($globsig) || '';
	$desc = " - $desc" if $desc;
	print "<LI><A NAME=\"$globsig\">$globsig</A>$addr$desc\n<UL>";
	my @locsigs = sortsigs( keys %{$SIGNAL::sighash{$globsig}} );
	foreach my $locsig ( @locsigs ) {
	  my $link = $first_pin{$locsig} || '';
	  $link = ": $link" if $link;
	  warn "No first pin for $locsig\n" unless $link;
	  print "<LI>$locsig$link\n";
	}
	print "</UL>\n";
  }
  close_slices();
}

#----------------------------------------------------------------
# Slices for signals
#----------------------------------------------------------------
sub pick_slice {
  my ( $signal ) = @_;
  foreach my $slice ( @slices ) {
	return $slice if $signal =~ m/^~?[$slice]/i;
  }
  return "etc";
}

sub open_slice {
  my ( $signal ) = @_;
  my $slice = pick_slice( $signal );
  if ( $slice ne $currslice ) {
	close STDOUT if $currslice;
	$currslice = $slice;
	my $slicefile = "html/SIG_$slice.html";
	if ( defined $slice_begun{$slice} ) {
	  open( STDOUT, ">>$slicefile" ) ||
		die "Error reopening $slicefile\n";
	  print STDERR "Reopening $slicefile for $signal\n";
	} else {
	  open( STDOUT, ">$slicefile" ) ||
		die "Error opening $slicefile\n";
	  my $title = "$SIGNAL::global{Experiment}<BR>\n";
	  $title .= "Signal Definitions [$slice]";
	  print
		start_html(
		  '-title' => "$SIGNAL::global{Exp} Signal Defs [$slice]",
		  '-author' => "allen\@huarp.harvard.edu"
		),
		"\n",
		center( h2( $title ) ),
		"\n",
		slicelinks(),
		"<UL>\n";
	  $slice_begun{$slice} = 1;
	  print STDERR "Opening $slicefile for $signal\n";
	}
  }
}

sub slicelinks {
  join '',
	"<P>",
	( map { $_ eq $currslice ? "[$_]" :
			"[<A HREF=\"SIG_$_.html\">$_</A>]" }
	  ( @slices, 'etc' )),
	"</P>\n";
}

sub close_slices {
  map close_slice( $_ ), $currslice, @slices;
}

sub close_slice {
  my ( $slice ) = @_;
  return unless $slice_begun{$slice};
  my $slicefile = "html/SIG_$slice.html";
  unless ( $slice eq $currslice ) {
	open( STDOUT, ">>$slicefile" ) ||
	  die "Unable to reopen $slicefile for close\n";
	$currslice = $slice;
  }
  print
	"</UL>\n",
	slicelinks(),
	HTML::trailer( '', "SIG_$slice" ),
	end_html;
  print STDERR "Closing $slicefile\n";
  delete $slice_begun{$slice};
  $currslice = '';
}
