@rem = '--*-Perl-*--
@echo off
if "%OS%" == "Windows_NT" goto WinNT
perl -x -S "%0" %1 %2 %3 %4 %5 %6 %7 %8 %9
goto endofperl
:WinNT
perl -x -S "%0" %*
if NOT "%COMSPEC%" == "%SystemRoot%\system32\cmd.exe" goto endofperl
if %errorlevel% == 9009 echo You do not have Perl in your PATH.
goto endofperl
@rem ';
#! /usr/local/bin/perl -w

# Generates the following:
#  master.html
#  index.html
#  SIG*.html
#  <Comp>/index.html
#  <Comp>/<conn>.html

use strict;
use FindBin;
use lib "$FindBin::Bin";
use SIGNAL;
use NETSLIB qw(open_nets mkdirp find_nets);
use CGI qw(start_html end_html h2 center p -no_debug);
use Table;
use HTML;
use File::Copy;
my $warn_buficon = 1;
my @html_refs = ( "index:Components", "master:Master",
				  "SIG_A-C:Signals", "chksig:Check" );
my @mon = ( "January", "February", "March", "April", "May",
         "June", "July", "August", "September", "October",
		 "November", "December" );

my $nets_dir = SIGNAL::siginit( "nets2html", 1, shift @ARGV );
SIGNAL::LogMsg "Nets2HTML: $nets_dir\n";

$SIGNAL::context = 'Nets2html';
SIGNAL::load_signals();

my %first_pin;

mkdirp( "html" );

my $html_author = "allen\@huarp.harvard.edu";

# If the specified file is present {
#   copy it's contents
#   look for $file.files {
#     src dest
#     where src will be found via open_nets
#     dest is relative to html/
#   }
# }
sub html_opt_header {
  my ( $filename, @default ) = @_;
  return join( "", @default ) unless $filename;
  my @output;
  if ( open_nets( *IFILE{FILEHANDLE}, $filename ) ) {
	@output = <IFILE>;
	close IFILE;
	if ( open_nets( *IFILE{FILEHANDLE}, "$filename.files" ) ) {
	  local $SIGNAL::context = "$filename.files";
	  while ( <IFILE> ) {
		s/#.*$//;
		next if /^\s*$/;
		if ( m|^(\S+)\s+(([-+\w][-+.\w]*/)*)([-+\w][-+.\w]*)$| ) {
		  my ( $src, $dpath, $dfile ) = ( $1, $2, $4 );
		  $dpath = "html/$dpath";
		  mkdirp( $dpath );
		  my $srcpath = find_nets( $src );
		  if ( $srcpath ) {
			copy( $srcpath, "$dpath$dfile" );
		  } else {
			warn "$SIGNAL::context: ",
			  "Unable to locate source file '$src'\n";
		  }
		} else {
		  warn "$SIGNAL::context: Cannot parse: '$_'\n";
		}
	  }
	  close IFILE || warn "$SIGNAL::context: Error closing\n";
	}
  } else {
	@output = @default;
  }
  join "", @output;
}

#----------------------------------------------------------------
# Build the basic header
#----------------------------------------------------------------
my $page_header = html_opt_header( "header.html",
  "<CENTER><TABLE WIDTH=\"100%\" BORDER=\"1\" CELLPADDING=\"4\" BGCOLOR=\"Teal\">\n",
  "<TR><TD ALIGN=\"CENTER\" BGCOLOR=\"#BFFFFF\">$SIGNAL::global{Organization}<BR>\n",
  "<B>$SIGNAL::global{Experiment}</B></TD></TR>\n",
  "</TABLE></CENTER>\n" );

my $copyright = $SIGNAL::global{Copyright} ||
	  "the President and Fellows of Harvard College";
my $page_trailer = join( '',
	html_opt_header( "trailer.html", '' ),
	"Web listings compiled using ",
	"<A HREF=\"http://www.arp.harvard.edu/eng/elec/nets.html\">",
	"Nets</A>\nsoftware on ",
	scalar(localtime),
	"<BR>\n<FONT SIZE=\"-2\">Copyright ",
	((localtime())[5]+1900),
	" by $copyright</FONT>\n" );

sub html_header {
  my ( $title ) = @_;
  join "",
	start_html(
	  '-title' => $title,
	  '-author' => $html_author,
	  '-BGCOLOR' => "white"
	), "\n",
	$page_header;
}

my %ftcable; # points to "other" cable for canonical connector
my %ftconn; # points to canonical connector when different
foreach my $comptype ( keys %SIGNAL::comptype ) {
  my $ct = $SIGNAL::comptype{$comptype};
  if ( $ct->{fdthr} ) {
	foreach my $group ( keys %{$ct->{fdthr}} ) {
	  my ( $canon ) = sort( keys %{$ct->{fdthr}->{$group}} );
	  foreach my $comp ( @{$ct->{comps}} ) {
		foreach my $conn ( keys %{$ct->{fdthr}->{$group}} ) {
		  if ( $conn ne $canon ) {
			# find full global alias for $conn $comp
			# find full global alias for $canon $comp
			# set $ftconn{$conn $comp} = $canon $comp;
			# set $ftcable{$canon $comp} = cable of $conn $comp;
			my $conncomp = SIGNAL::make_conncomp( $conn, $comp );
			my $galias = SIGNAL::get_global_alias( $conncomp );
			my $canoncomp = SIGNAL::make_conncomp( $canon, $comp );
			my $cgalias = SIGNAL::get_global_alias( $canoncomp );
			$ftconn{$galias} = $cgalias;
			$ftcable{$cgalias} =
			  $SIGNAL::comp{$comp}->{cable}->{$conn};
		  }
		}
	  }
	}
  }
}

{ local $SIGNAL::context = 'html/master.html';
  open( STDOUT, ">$SIGNAL::context" ) ||
	die "$SIGNAL::context: Unable to open $SIGNAL::context\n";

  print
	html_header( "$SIGNAL::global{Exp} Master Connector List" ),
	html_opt_header( "master.html",
	  center(h2( "Master Connector List" ))),
	"\n<UL>\n";

  my @cables =
	map $_->[0], sort {
	  $a->[1] cmp $b->[1] ||
	  $a->[2] <=> $b->[2] ||
	  $a->[0] cmp $b->[0]
	} map {
	  $_ =~ m/^(\D+)(\d*)\D*$/;
	  [ $_, $1 || '', $2 || 0 ]
	} keys %SIGNAL::cable;

  my $t = Table::new( BORDER => 1 );
  $t->NewRow;
  $t->Head( 'Cable' );
  $t->Head( 'Suffix' );
  $t->Head( 'Description' );
  $t->Head( 'Type' );
  $t->Head( 'Location' );
  $t->Head( 'Local Name' );
  
  foreach my $cable ( @cables ) {
	# We've already sorted by cable
	# Now eliminate connectors if ftconn and
	# sort connectors by ftcable || cable, comp, conncomp
	my @cableconns = sort {
		$a->[2] cmp $b->[2] ||  # feedthrough cable
		$a->[4] cmp $b->[4] ||  # $acomp
		$a->[3] <=> $b->[3] ||  # $aconn number
		$a->[0] cmp $b->[0]
	  } grep ! $ftconn{$_->[0]},
	  map {
		my $alias = SIGNAL::get_global_alias( $_ );
		my ( $aconn, $acomp ) = SIGNAL::split_conncomp( $alias );
		$aconn =~ m/^(\D+)(\d*)\D*$/;
		[ $alias, $_, ($ftcable{$alias} || $cable), $2, $acomp ];
	  } @{$SIGNAL::cable{$cable}};
	foreach ( @cableconns ) {
	  my ( $alias, $conncomp ) = @$_;
	  my ( $conn, $comp ) = SIGNAL::split_conncomp( $conncomp ); 
	  $t->NewRow;
	  if ( $comp ) {
		my $comptype = $SIGNAL::comp{$comp}->{type};
		my $conndef = $SIGNAL::comptype{$comptype}->{conn}->{$conn};
		my $compdef = $SIGNAL::comp{$comp};
		my $conntype = $conndef->{type} || "";
		my $conndesc = $compdef->{conndesc}->{$conn} ||
		  $conndef->{desc} || "";
		my ( $aconn, $acomp ) = SIGNAL::split_conncomp( $alias );
		
		$t->Cell( "<A HREF=\"$comp/$conn.html\">$aconn</A>" );
		$t->Cell( "<A HREF=\"$comp/index.html\">$acomp</A>" );
		$t->Cell( $conndesc );
		$t->Cell( $conntype );
		$t->Cell( "<A HREF=\"$comp/index.html\">$comp</A>" );
		my $ft = $ftcable{$alias} ? " (FT)" : "";
		$t->Cell( "<A HREF=\"$comp/$conn.html\">$conn</A>$ft" );
	  } else {
		warn "$SIGNAL::context: Unable to understand conncomp '$conncomp'\n";
		$t->Cell( $conncomp );
	  }
	}
  }
  print
	"<CENTER>\n",
	$t->Output( 'html-tables' ),
	html_trailer( '', 'master' ),
	"</CENTER>\n",
	end_html, "\n";
  close STDOUT || warn "$SIGNAL::context: Error closing html/master.html\n";
}

#----------------------------------------------------------------
# Generate index.html
#----------------------------------------------------------------
{ my %part_html;
  local $SIGNAL::context = 'html/index.html';

  open( STDOUT, ">$SIGNAL::context" ) ||
	die "$SIGNAL::context: Unable to open $SIGNAL::context\n";
  print
	html_header( "$SIGNAL::global{Exp} Electrical Systems" ),
	html_opt_header( "index.html",
		  center( h2( "Electrical Systems" ))),
	"\n";
  if ( open(IFILE, "<PARTS.ORG") ) {
	print "\n<UL>\n";
	my $lev = 0;
	my ( $newlev, $string );
	ORGFILE:
	while (<IFILE>) {
	  chomp;
	  m/^(\s*)(.*)$/;
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
	  last if ( $string eq "" );
	  print "<LI>" if ( $lev > 0 );
	  while ( $string =~ m/^(.*){([A-Za-z_0-9]+)}(.*)$/ ) {
		my ($pre,$comp,$post) = ($1,$2,$3);
		if ( ! defined $SIGNAL::comp{$comp} ) {
		  warn "$SIGNAL::context: Unknown comp '$comp' in PARTS.org\n";
		  $comp = "[$comp]";
		} else {
		  $part_html{$comp} = 1;
		  $comp = 
			"<A HREF=$comp/index.html>$comp</A>: " .
			"$SIGNAL::comp{$comp}->{desc}\n";
		}
		$string = "$pre$comp$post";
	  }
	  while ( $string =~ m/^(.*){([A-Za-z_0-9:]+)}(.*)$/ ) {
		my ( $pre, $list, $post ) = ( $1, $2, $3 );
		my @out;
		foreach my $comp ( split( /:/, $list ) ) {
		  if ( ! defined $SIGNAL::comp{$comp} ) {
			warn "$SIGNAL::context: Unknown comp '$comp' in PARTS.org\n";
			push( @out, "[$comp]" );
		  } else {
			push( @out, "<A HREF=$comp/index.html>$comp</A> " );
			$part_html{$comp} = 1;
		  }
		}
		$string = join('', $pre, @out, $post, );
	  }
	  print "$string\n";
	}
	close IFILE;
	while ( 0 < $lev ) {
	  print "</UL>\n";
	  $lev--;
	}
  }
  my @uncat = grep !$part_html{$_}, sort keys %SIGNAL::comp;
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
  print html_trailer( "", "index" ), end_html;
  close STDOUT || warn "$SIGNAL::context: Error writing html/index.html\n";
}

# Pick up the gifpins database
my %gifpins;
use Fcntl;
use SDBM_File;
tie( %gifpins, 'SDBM_File', 'net/gifpins',
	  O_RDONLY, 0640);

#----------------------------------------------------------------
# Process Components
#----------------------------------------------------------------
foreach my $comp ( keys %SIGNAL::comp ) {
  local $SIGNAL::context = $comp;
  mkdirp( "html/$comp" );
  my $comptype = $SIGNAL::comp{$comp}->{type} ||
	die "$SIGNAL::context: No comptype for component ${comp}\n";
  my $ct = $SIGNAL::comptype{$comptype} ||
	die "$SIGNAL::context: No comptype{} for component '${comp}' type '$comptype'\n";
  my $CompDesc = $SIGNAL::comp{$comp}->{desc} || $ct->{desc} || '';

  my ( %conn, %pin, %others );
  SIGNAL::load_netlist( $comptype, $comp, \%conn, \%pin, \%others );
  open( STDOUT, ">html/$comp/index.html" ) ||
	die "$SIGNAL::context: Unable to write to html/$comp/index.html\n";
  print
	html_header( "$SIGNAL::global{Exp} $CompDesc $comp" ),
	html_opt_header( "comp/$comp/index.html",
	  "<CENTER><TABLE>\n",
	  "<TR><TH ALIGN=\"RIGHT\">Component:</TH>\n",
	  "<TD>$comp</TD></TR>\n",
	  "<TR><TH ALIGN=\"RIGHT\">Description:</TH>\n",
	  "<TD>$CompDesc</TD></TR></TABLE></CENTER><P>\n" ),
	html_opt_header( "sym/$comptype/index.html", "" ),
	"\n";
  my $t = Table::new( BORDER => 1 );
  $t->NewRow;
  $t->Head( "Global" );
  $t->Head( "Description" );
  $t->Head( "Type" );
  $t->Head( "Local" );
  
  # Eliminate extra feedthrough listings
  # Sort by global number, then name
  # Want to display alias, description, type, localname (FT)
  my @conns =
	sort {
	  $a->[2] <=> $b->[2] || # Global connector number
	  $a->[0] cmp $b->[0];
	}
	grep ! $ftconn{$_->[0]},
	map {
	  my $conncomp = SIGNAL::make_conncomp( $_, $comp );
	  my $alias = SIGNAL::get_global_alias( $conncomp );
	  my ( $aconn, $acomp ) = SIGNAL::split_conncomp( $alias );
	  $aconn =~ m/^\D+(\d*)\D*/;
	  [ $alias, $_, $1 || 0, $conncomp ];
	} @{$SIGNAL::comptype{$comptype}->{conns}};
  foreach ( @conns ) {
	my ( $alias, $conn, $num, $conncomp ) = @$_;
	my $conntype = $ct->{conn}->{$conn}->{type} || '';
	my $conndesc = $ct->{conn}->{$conn}->{desc} || '';
	$t->NewRow;
	$t->Cell( "<A HREF=\"$conn.html\">$alias</A>" );
	$t->Cell( $conndesc );
	$t->Cell( $conntype );
	my $lname = ( $conncomp ne $alias ) ? $conn : "";
	$lname .= " (FT)" if $ftcable{$alias};
	$t->Cell( $lname );
  }
  if ( scalar(keys %others) > 0 ) {
	$t->NewRow;
	$t->Cell( "<A HREF=\"other.html\">Others</A>" );
	$t->Cell( "Other Components" );
	$t->Cell( "" );
  }
  print
	"<center>\n",
	$t->Output('html-tables'),
	"</center>\n",
	html_trailer( "../", "", "[$comp]\n" ),
	end_html;
  close(STDOUT) || warn "$SIGNAL::context: Error closing html/$comp/index.html\n";
  # print STDERR "html/$comp/index.html written\n";

  foreach my $conn ( @{$SIGNAL::comptype{$comptype}->{conns}} ) {
	my $conntype = $ct->{conn}->{$conn}->{type} || '';
	my $conndesc = $ct->{conn}->{$conn}->{desc} || '';
	my $myconncomp = SIGNAL::make_conncomp( $conn, $comp );
	my $myalias = $SIGNAL::comp{$comp}->{alias}->{$conn} ||
					$myconncomp;
	next if $ftconn{$myalias};
	$conndesc .= " (Feedthrough)" if $ftcable{$myalias};
	
	my @pins;
	SIGNAL::define_pins( $conntype, \@pins );
	open( STDOUT, ">html/$comp/$conn.html" ) ||
	  die "$SIGNAL::context: Unable to write to html/$comp/$conn.html\n";
	print
	  html_header( "$SIGNAL::global{Exp} $comp $conn" ),
	  html_opt_header( "comp/$comp/$conn.html", "" ) ||
	  html_opt_header( "sym/$comptype/$conn.html", "" ),
	  "<CENTER><TABLE>\n",
	  "<TR><TH ALIGN=\"RIGHT\">Connector:</TH>",
	  "<TD>$myalias</TD></TR>\n",
	  "<TR><TH ALIGN=\"RIGHT\">Local Name:</TH>",
	  "<TD>$myconncomp</TD></TR>\n",
	  "<TR><TH ALIGN=\"RIGHT\">Location:</TH>",
	  "<TD><A HREF=\"index.html\">$comp: $CompDesc</A></TD></TR>\n",
	  "<TR><TH ALIGN=\"RIGHT\">Type:</TH>",
	  "<TD>$conntype</TD></TR>\n",
	  "<TR><TH ALIGN=\"RIGHT\">Description:</TH>",
	  "<TD>$conndesc</TD></TR>\n";

	#----------------------------------------------------------------
	# Extract Cable info from database
	#----------------------------------------------------------------
	my %rsigs;
	my %rpins;
	my ( $fconn, $falias ) = ( '', '' );
	print
	  "<TR><TH ALIGN=\"RIGHT\">Cabled To:</TH><TD>",
	  get_cable_info( $comp, $conn, \%rsigs, \%rpins ),
	  "</TD></TR>\n";
	if ( $ftcable{$myalias} ) {
	  my $fgrp =
		$SIGNAL::comptype{$comptype}->{conn}->{$conn}->{fdthr};
	  ( $fconn ) = grep $_ ne $conn,
		keys %{$SIGNAL::comptype{$comptype}->{fdthr}->{$fgrp}};
	  die "fconn undefined!\n" unless $fconn;
	  my $fconncomp = SIGNAL::make_conncomp( $fconn, $comp );
	  $falias = SIGNAL::get_global_alias( $fconncomp );
	  print
		"<TR><TH ALIGN=\"RIGHT\">Cabled To:</TH><TD>",
		get_cable_info( $comp, $fconn, \%rsigs, \%rpins ),
		"</TD></TR>\n";
	}
	print
	  "</TABLE></CENTER><P>\n";

	$t = Table::new( BORDER => 1);
	$t->NewRow;
	$t->Head('Pin');
	$t->Head('Signal');
	$t->Head($fconn ? 'Cable-To' : 'Link-To');
	$t->Head('Cable-To');
	foreach my $pin ( @pins ) {
	  $t->NewRow;
	  $t->Cell( "<A NAME=\"P$pin\">$pin</A>" );
	  my ( $gsignal, $link ) =
		get_sig_link( $conn, $comp, $pin, \%conn, \%pin,
				\%others );
	  $t->Cell( $gsignal );
	  $t->Cell( $link ) unless $fconn;

	  $link = '';
	  my $rsig = $rpins{$myalias}->{$pin} || '';
	  if ( $rsig && $rsigs{$rsig} ) {
		my $rlink = $rsigs{$rsig}->{"$myalias.$pin"};
		if ( $rlink ) {
		  $rlink =~ m/^([^.]+)\.(\w+)$/ ||
			die "$SIGNAL::context: No Link! '$rlink'\n";
		  my ( $ralias, $rpin ) = ( $1, $2 );
		  my $rconncomp = $SIGNAL::conlocname{$ralias};
		  my ( $rconn, $rcomp ) = SIGNAL::split_conncomp( $rconncomp );
		  $link = get_link( $rcomp, $rconn, $rpin,
						  \%others, "../$rcomp/" );
		  # $t->Cell( 
		  #   "<A HREF=\"../$rcomp/$rconn.html#P$rpin\">$ralias.$rpin</A>"
		  # );
		}
	  }
	  $t->Cell( $link );
	  
	  if ( $fconn ) {
		$link = '';
		my $rsig = $rpins{$falias}->{$pin} || '';
		if ( $rsig && $rsigs{$rsig} ) {
		  my $rlink = $rsigs{$rsig}->{"$falias.$pin"};
		  if ( $rlink ) {
			$rlink =~ m/^([^.]+)\.(\w+)$/ ||
			  die "$SIGNAL::context: No Link! '$rlink'\n";
			my ( $ralias, $rpin ) = ( $1, $2 );
			my $rconncomp = $SIGNAL::conlocname{$ralias};
			my ( $rconn, $rcomp ) = SIGNAL::split_conncomp( $rconncomp );
			$link = get_link( $rcomp, $rconn, $rpin,
						  \%others, "../$rcomp/" );
		  }
		}
		$t->Cell( $link );
	  }
	}
	print center( "\n", $t->Output( 'html-tables' ) ), "\n";
	print
	  html_trailer( "../", "", "[<A HREF=\"index.html\">$comp</A>]\n" ),
	  end_html;
	close STDOUT || warn "$SIGNAL::context: Error closing html/$comp/$conn.html\n";
	# print STDERR "html/$comp/$conn.html written\n";
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
		die "$SIGNAL::context: Unable to create html/$comp/other.html\n";
	  my $title = "$SIGNAL::global{Experiment}<BR>$CompDesc $comp<BR>\n";
	  $title .= "Other Components";
	  print
		html_header( "$SIGNAL::global{Exp} $comp Other Components" ),
		html_opt_header( "comp/$comp/other.html", "" ) ||
		html_opt_header( "sym/$comptype/other.html",
		  "<CENTER>$comp Other Components</CENTER>\n" ),
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
		html_trailer( '../', '', "[<A HREF=\"index.html\">$comp</A>]\n" ),
		end_html, "\n";
	  close STDOUT ||
		warn "$SIGNAL::context: Error closing html/$comp/other.html\n";
	  # print STDERR "html/$comp/other.html written\n";
	  undef $table;

	  my $curfile = '';
	  foreach my $refdes ( @others ) {
		$refdes =~ m/^(.*\D)\d*$/;
		if ( $1 ne $curfile ) {
		  if ( $curfile ) {
			print "[<A HREF=\"index.html\">$comp</A>]\n",
			  html_trailer( '../', '', "[<A HREF=\"index.html\">$comp</A>]\n" ),
			  end_html, "\n";
			close STDOUT ||
			  warn "$SIGNAL::context: Error closing html/$comp/$curfile.html\n";
			# print STDERR "html/$comp/$curfile.html written\n";
		  }
		  $curfile = $1;
		  open( STDOUT, ">html/$comp/$curfile.html" ) ||
			die "$SIGNAL::context: Unable to create html/$comp/$curfile.html\n";
		  my $title = "$SIGNAL::global{Experiment}<BR>$CompDesc $comp<BR>\n";
		  $title .= "Other Components: '$curfile'";
		  print
			html_header(
			  "$SIGNAL::global{Exp} $comp Other Components '$curfile'" ),
			"\n",
			html_opt_header( "comp/$comp/$curfile.html", "") ||
			html_opt_header( "sym/$comptype/$curfile.html",
			  center(h2( "$comp Other Components: '$curfile'" ))),
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
		print
		  html_trailer( '../', '', "[<A HREF=\"index.html\">$comp</A>]\n" ),
		  end_html, "\n";
		close STDOUT ||
		  warn "$SIGNAL::context: Error closing html/$comp/$curfile.html\n";
		# print STDERR "html/$comp/$curfile.html written\n";
	  }
	}
  }
  
  #----------------------------------------------------------------
  # Now process areas.dat if present
  #----------------------------------------------------------------
  if ( open( AREAFILE, "<net/comp/$comp/areas.dat" ) ) {
	local $SIGNAL::context = "net/comp/$comp/areas.dat";
	while (<AREAFILE>) {
	  m/^GIF:([^:]+):(\d+),(\d+)$/ || die "$SIGNAL::context: Syntax error\n";
	  my ( $gif, $w, $h ) = ( $1, $2, $3 );
	  open( STDOUT, ">html/$comp/$gif.html" ) ||
		die "$SIGNAL::context:$gif.html: Error writing\n";
	  print
		html_header( "$SIGNAL::global{Exp} $comp $gif Buffer" ),
		html_opt_header( "comp/$comp/$gif.html", "" ) ||
		html_opt_header( "comp/$comp/gif.html", "" ) ||
		html_opt_header( "sym/$comptype/$gif.html", "" ) ||
		html_opt_header( "sym/$comptype/gif.html",
		  center( h2( "$comp $gif Schematic" )) ),
		"\n",
		"<IMG SRC=\"$gif.gif\" WIDTH=$w HEIGHT=$h BORDER=0 ",
		  "ALT=\"\" USEMAP=\"#MAP\">\n",
		"<MAP NAME=\"MAP\">\n";
	  while (<AREAFILE>) {
		last if /^$/;
		m/^(\w+):([-0-9,]+):(.*)$/ ||
		  die "$SIGNAL::context: Syntax error: $_\n";
		my ( $atype, $coords, $text ) = ( $1, $2, $3 );
		my $href = '';
		if ( $atype eq 'SIGNAL' ) {
		  my $lsignal = "$text($comp)" unless $text =~ m/\(.+\)$/;
		  my $gsignal = $SIGNAL::globsig{$lsignal};
		  if ( $gsignal ) {
			my $slice = SIGNAL::pick_slice($gsignal);
			$href = "../SIG_$slice.html#$gsignal";
		  }
		} elsif ( $atype eq 'REFDES' ) {
		  if ( $others{$text} ) {
			$text =~ m/^(.*\D)(\d*)$/ || die;
			$href = "$1.html#$text";
		  } else {
			warn "$SIGNAL::context: Unlisted REFDES $text\n";
		  }
		} elsif ( $atype eq 'LINKTO' ) {
		  if ( $text =~ m/^([^.]+)\.(\w+)$/ ) {
			my ( $conn, $pin ) = ( $1, $2 );
			$href = get_link_href( $comp, $conn, $pin, \%others, '' );
		  }
		}
		if ( $href ) {
		  print
			"<AREA COORDS=\"$coords\" HREF=\"$href\">\n";
		}
	  }
	  print
		"</MAP>\n",
		html_trailer( '../', '', "[<A HREF=\"index.html\">$comp</A>]\n" ),
		end_html;
	  close STDOUT ||
		warn "$SIGNAL::context: error closing $gif.html\n";
	}
  }
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
	  warn "$SIGNAL::context: No first pin for $locsig\n" unless $link;
	  print "<LI>$locsig$link\n";
	}
	print "</UL>\n";
  }
  close_slices();
}

if ( open( STDOUT, ">html/chksig.html" ) ) {
  my $title = "$SIGNAL::global{Experiment}<BR>\n";
  $title .= "Signal Cross-Check";
  print
	html_header( "$SIGNAL::global{Exp} Signal Cross-Check" ),
	html_opt_header( "chksig.html",
	  "<CENTER><H2>Signal Cross-Check</H2></CENTER>\n" ),
	"<P>This page lists signal names which are associated\n",
	"with more than one global signal or are associated with\n",
	"one global signal of a different name. There may not be\n",
	"anything wrong with either of these situations, but you\n",
	"may want to double check.\n",
	"\n";

  my @sigs = sort keys %SIGNAL::sigcomps;
  foreach my $signal ( @sigs ) {
	my $desc = SIGNAL::get_sigdesc($signal) || '';
	my %gn;
	foreach my $comp ( keys %{$SIGNAL::sigcomps{$signal}} ) {
	  my $locsig = "$signal($comp)";
	  my $gn = $SIGNAL::globsig{$locsig} || "UNDEFINED";
	  $gn{$gn} = $locsig;
	}
	my @gn = sort keys %gn;
	if ( $signal !~ m/^(_AD|\$)/ &&
		 ( @gn > 1 ||
		   $gn[0] !~ m/^\Q$signal\E(\(.*\))?$/ ) ) {
	  print
		"<P><B>$signal:</B> $desc\n<UL>\n";
	  foreach my $gn ( @gn ) {
		my $slice = SIGNAL::pick_slice($gn);
		my $glink =
		  "<A HREF=\"SIG_$slice.html#$gn\">$gn</A>";
		print
		  "<LI>$glink: $gn{$gn}\n";
	  }
	  print "</UL>\n";
	}
  }
  print
	#"</UL>\n",
	html_trailer( '', 'chksig' ),
	end_html, "\n";
} else {
  warn "$SIGNAL::context: Unable to create html/chksig.html\n";
}

untie %gifpins;

# get_link creates an HTML link to the specified connector and pin
# If $thisdir is TRUE, it is assumed that the target is to be found
# in the current directory. If false, the href is prefixed with
# "$comp/" (i.e. it is assumed the reference is from the parent
# directory, which is true for hrefs from the Signals pages.
#
# "Other" links are found in files named
#   html/<comp>/<P>.html#P<U>.$pin
# Where <P> is the non-digit prefix of <U>
# Hence R1, R2 and R3 are found in R.html.
sub get_link_href {
  my ( $comp, $conn, $pin, $pother, $thisdir ) = @_;
  my $conncomp = SIGNAL::make_conncomp( $conn, $comp );
  my $alias = SIGNAL::get_global_alias( $conncomp );
  if ( $ftconn{$alias} ) {
	$alias = $ftconn{$alias};
	$conncomp = $SIGNAL::conlocnam{$alias} || $alias;
	( $conn, $comp ) = SIGNAL::split_conncomp($conncomp);
  }
  my $href;
  if ( defined $pother->{$conn} ) {
	$conn =~ m/^(.*\D)\d*$/ || die "$SIGNAL::context: Could not match '$conn'";
	if ( $pin ne '' ) {
	  $href = "$thisdir$1.html#P$conn.$pin";
	} else {
	  $href = "$thisdir$1.html#$conn";
	}
  } else {
	$href = "$thisdir$conn.html";
	$href .= "#P$pin" if $pin ne '';
  }
  $href;
}

# get_link returns an appropriate anchor link to the specified
# comp, conn and pin, appending any useful graphics where possible
# The (comp, conn) will be promoted to a global alias and also
# resolved to the appropriate feedthrough value.
sub get_link {
  my ( $comp, $conn, $pin, $pother, $thisdir ) = @_;
  my $href = get_link_href( $comp, $conn, $pin, $pother, $thisdir );
  my $conncomp = SIGNAL::make_conncomp( $conn, $comp );
  my $alias = SIGNAL::get_global_alias( $conncomp );
  if ( $ftconn{$alias} ) {
	$alias = $ftconn{$alias};
	$conncomp = $SIGNAL::conlocnam{$alias} || $alias;
	( $conn, $comp ) = SIGNAL::split_conncomp($conncomp);
  }
  my $link = "<A HREF=\"$href\">$alias.$pin</A>";
  if ( $gifpins{"$comp:$conn.$pin"} ) {
	my $dir = $thisdir ? '' : "$comp/";
	my $sig = $gifpins{"$comp:$conn.$pin"};
	my $href = "$dir$sig.html";
	my $rootdir = $thisdir ? '../' : '';
	$link .= "<A HREF=\"$href\"> <IMG SRC=\"${rootdir}buficon.gif\" " .
			  "WIDTH=30 HEIGHT=15 BORDER=0 ALT=\"\"></A>";
	unless ( -f "html/buficon.gif" ) {
	  my $src = find_nets( "sym/buficon.gif" );
	  if ( $src ) {
		copy( $src, "html/buficon.gif" );
	  } elsif ( $warn_buficon ) {
		warn "$SIGNAL::context: Unable to locate buficon.gif\n";
		$warn_buficon = 0;
	  }
	}
  }
  $link;
}

# get_sig_link is called from two places:
#   Generating connector listings
#   Generating "other" listings
# It produces an array of two elements which are HTML for
# the signal and the local link, along with any associated
# <A>links</A>. $pconn and $ppin are the usual netlist stuff.
# $pother is a the netlist 'other' hash ref with pkg_type info
# on other parts. 
sub get_sig_link {
  my ( $conn, $comp, $pin, $pconn, $ppin, $pother ) = @_;
  my ( $signal, $gsignal, $link ) = ( '', '', '' );
  if ( $pconn->{$conn} && $pconn->{$conn}->{$pin} ) {
	$signal =  $pconn->{$conn}->{$pin};
	my ( $lsignal, $lcomp );
	if ( $signal =~ m/\((.+)\)$/ ) {
	  $lsignal = $signal;
	  $lcomp = $1;
	} else {
	  $lsignal = "$signal($comp)";
	  $lcomp = $comp;
	}
	if ( ! $first_pin{$lsignal} && $lcomp eq $comp ) {
	  $first_pin{$lsignal} =
		get_link( $comp, $conn, $pin, $pother, "$comp/" );
	}
	$gsignal = $SIGNAL::globsig{$lsignal};
	if ( $gsignal ) {
	  my $slice = SIGNAL::pick_slice($gsignal);
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
	  my ( $refdes, $pin ) = ( $1, $2 );
	  $link = get_link( $comp, $refdes, $pin, $pother, "" );
	}
	
  }
  ( $gsignal, $link );
}

sub get_cable_info {
  my ( $comp, $conn, $rsigs, $rpins ) = @_;
  my $myconncomp = SIGNAL::make_conncomp( $conn, $comp );
  my $cable_html = '';
  if ( $SIGNAL::comp{$comp}->{cable}->{$conn} ) {
	my $cable = $SIGNAL::comp{$comp}->{cable}->{$conn};
	my @OUTPUT;
	die "$SIGNAL::context: No cable definition for $conn:$comp\n" unless
	  defined $SIGNAL::cable{$cable};
	my @cdef = @{$SIGNAL::cable{$cable}};

	if ( @cdef > 1 ) {
	  my @cdefs;
	  foreach my $conncomp ( @cdef ) {
		if ( $conncomp ne $myconncomp ) {
		  my ( $cconn, $ccomp ) = SIGNAL::split_conncomp( $conncomp);
		  $cconn || die "$SIGNAL::context: Bad conncomp '$conncomp'\n";
		  push( @cdefs,
			get_link( $ccomp, $cconn, '', {}, "../$ccomp/" ) );
		  #push( @cdefs, "<A HREF=\"../$ccomp/$cconn.html\">$cconn</A>" .
			  # ":<A HREF=\"../$ccomp/index.html\">$ccomp</A>" );
		}
	  }
	  while ( my $cdef = shift @cdefs ) {
		push @OUTPUT, $cdef;
		if ( @cdefs > 1 ) {
		  push @OUTPUT, ", ";
		} elsif ( @cdefs == 1 ) {
		  push @OUTPUT, " and ";
		}
	  }
	  SIGNAL::load_netlist( '#CABLE', $cable, $rpins, $rsigs );
	} else {
	  push @OUTPUT, "No Cable Defined\n";
	}
	$cable_html = join "", @OUTPUT;
  } else {
	$cable_html = "No cable information";
  }
  $cable_html;
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

sub open_slice {
  my ( $signal ) = @_;
  my $slice = SIGNAL::pick_slice( $signal );
  if ( $slice ne $currslice ) {
	close STDOUT if $currslice;
	$currslice = $slice;
	my $slicefile = "html/SIG_$slice.html";
	if ( defined $slice_begun{$slice} ) {
	  open( STDOUT, ">>$slicefile" ) ||
		die "$SIGNAL::context: Error reopening $slicefile\n";
	  print STDERR "Reopening $slicefile for $signal\n";
	} else {
	  open( STDOUT, ">$slicefile" ) ||
		die "$SIGNAL::context: Error opening $slicefile\n";
	  print
		html_header( "$SIGNAL::global{Exp} Signal Defs [$slice]" ),
		html_opt_header( "SIG_$slice.html",
		  html_opt_header( "SIG.html", "" ),
		  center( h2( "Signal Definitions [$slice]" ) ) ),
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
	  ( @SIGNAL::slices, 'etc' )),
	"</P>\n";
}

sub close_slices {
  map close_slice( $_ ), $currslice, @SIGNAL::slices;
}

sub close_slice {
  my ( $slice ) = @_;
  return unless $slice_begun{$slice};
  my $slicefile = "html/SIG_$slice.html";
  unless ( $slice eq $currslice ) {
	open( STDOUT, ">>$slicefile" ) ||
	  die "$SIGNAL::context: Unable to reopen $slicefile for close\n";
	$currslice = $slice;
  }
  print
	"</UL>\n",
	slicelinks(),
	html_trailer( '', "SIG_$slice" ),
	end_html;
  print STDERR "Closing $slicefile\n";
  delete $slice_begun{$slice};
  $currslice = '';
}


sub html_trailer {
  my ( $subdir, $thisfile, $xtraref, $tfile ) = @_;
  my @OUTPUT;
  push @OUTPUT, "<P>\n";
  push @OUTPUT, $xtraref if $xtraref;
  foreach ( @html_refs ) {
	my ( $file, $label ) = split( /:/ );
	if ( $file eq $thisfile ) {
	  push @OUTPUT, "[$label]\n";
	} else {
	  push @OUTPUT, "[<A HREF=\"${subdir}$file.html\">$label</A>]\n";
	}
  }
  push @OUTPUT,
	"<P>\n",
	html_opt_header( $tfile, "" ),
	$page_trailer;
  join '', @OUTPUT;
}

__END__
:endofperl
