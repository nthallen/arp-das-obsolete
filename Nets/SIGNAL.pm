package SIGNAL;
use strict;

use Data::Dumper;
use NETSLIB qw(open_nets mkdirp);

$SIGNAL::context = "";

# Definitions:
# SigComp /^(~?)([$\w]+)(\((\w+)\))?$/
#  Signal Name with an optional component name: A_UVV(MDP)
#  May include leading tilde.

#=----------------------------------------------------------------
#= Global Case DataBase
#=----------------------------------------------------------------
%SIGNAL::case = ();
#= %SIGNAL::case = { <uppercase> => <case-sensitive> }
#  define_case( $prefix, $signal );
#  get_case( $prefix, $signal );

#=----------------------------------------------------------------
#= Global Signals Database
#=----------------------------------------------------------------
%SIGNAL::sigcomps = ();
%SIGNAL::sigdesc = ();
%SIGNAL::sigcfg = ();
%SIGNAL::globsig = ();
%SIGNAL::sighash = ();
#= %SIGNAL::sigcomps = { <signal> => { <comp> => 1 } }
#= %SIGNAL::sigdesc =  { <signal> => <description> }
#= %SIGNAL::sigcfg  =  { <signal> => [ configuration ] }
#= %SIGNAL::globsig =  { <SigComp> => <global signal name> }
#= %SIGNAL::sighash =  { <global signal name> => { <SigComp> => 1 } }

#  define_sigcase( $signal );
#    Takes a local signal with optional leading tilde
#    returns global case, even if collision occurred.
#    returns undef if signal is out of spec.
#  get_sigcase( $sigcomp );
#    preserves prefix, component, etc.
#  define_sigdesc( $signal, $desc );
#    Defines $signal case, $desc.
#  define_locsig( $signal, $comp );
#    Defines comp, signal case.
#    ?? Should this take a sigcomp? Perhaps don't assume case?
#  equate_signals( $sigcomp1, $sigcomp2 );
#    Assumes local sigs have already been defined.
#  rename_global( $global1, $global2 );

#=----------------------------------------------------------------
#= Components Database
#=----------------------------------------------------------------
%SIGNAL::comp = ();
%SIGNAL::comptype = ();
#= %SIGNAL::comp = { <comp> => { type => <comptype>,
#=                               desc => <description>,
#=                               base => <baseaddr>,
#=                               alias => { <conn> => <alias> },
#=                               cable => { <conn> => <cable> } } }
#= %SIGNAL::comptype =
#=    { <comptype> => { desc  => <description>,
#=                      comps => ( <comp> ),
#=                      conns => ( <conn> ),
#=                      fdthr => { <group> => { <conn> => 1 } },
#=                      conn  => { <conn> =>
#=                                  { type => <conntype>,
#=                                    fdthr => <group>,
#=                                    desc => <description> } },
#=                      bufsch => <schmatic range> },
#=                      bufconns => <list of connectors> } }
# define_comptype( $comptype, $desc );
# define_conncomp( $conncomp, $conntype, $desc, $globname );
# define_comp( $comp, $comptype );
#    Defines comp case.
#    comptype may be null?
# define_compdesc( $comp, $desc );
#------
# Note: may want the following functionality
#  Update comptype desc quietly (txt2ili when reading PARTS)

#=----------------------------------------------------------------
#= Interconnect Database
#=----------------------------------------------------------------
%SIGNAL::cable = ();
%SIGNAL::conlocname = ();
#= %SIGNAL::cable = { <cable> => ( <conncomps> ) }
#= %SIGNAL::conlocname = { <alias> => <conncomp> }

#=----------------------------------------------------------------
#= Global Data
#=----------------------------------------------------------------
%SIGNAL::global = (
  'Organization' => "Anderson Group at Harvard University",
  'Experiment' => "Unspecified Experiment",
  'Exp' => "Unspecified" );
#= %SIGNAL::global = { <property> => <value> }
#=  <property>s are:
#=    Organization e.g. Harvard University Atmospheric Research Project
#=    Experiment   e.g. Radiometer Experiment
#=    Exp          e.g. Radiometer

@SIGNAL::slices = ( "A-C", "D", "E-M", "N-R", "S", "T-Z" );

# Global Properties
# set_global( Property, Value );
# get_global( Property );

my %casetype = ( 'S' => "Signal", 'C' => "Component",
	'T' => "Comp. Type", 'K' => "Cable" );
sub case_case {
  my ( $prefix, $signal, $define ) = @_;
  warn "$SIGNAL::context: Undefined case type: %prefix\n"
	unless $casetype{$prefix};
  my $lcsig = $signal;
  my $ucsig = "$prefix\U$signal";
  if ( $SIGNAL::case{$ucsig} ) {
	$lcsig = $SIGNAL::case{$ucsig};
	warn "${SIGNAL::context}: $casetype{$prefix} ",
		  "$signal collides, changed to $lcsig\n"
	  if ( $lcsig ne $signal ) && $define;
  } else {
	$SIGNAL::case{$ucsig} = $signal;
  }
  return $lcsig;
}
sub define_case {
  my ( $prefix, $signal ) = @_;
  return case_case( $prefix, $signal, 1 );
}
sub get_case {
  my ( $prefix, $signal ) = @_;
  return case_case( $prefix, $signal, 0 );
}
sub define_sigcase {
  my ( $signal ) = @_;
  if ( $signal =~ m/^(~?)(\$?[-+_\w]+)$/ ) {
	my ( $pre, $sig ) = ( $1, $2 );
	$sig = define_case( "S", $sig );
	$signal = "$pre$sig";
	return $signal;
  } else {
	warn "$SIGNAL::context: Signal \"$signal\" out of spec\n";
	return undef;
  }
}

# accepts a sigcomp.
sub get_sigcase {
  my ( $name ) = @_;
  $name =~ m/^(~?)(\$?[-+_\w]+)(\(([_\w]+)\))?$/ ||
	die "$SIGNAL::context: Signal $name out of spec in get_sigcase";
  my $signal = uc($2);
  my $prefix = $1 || "";
  my $comp = $4 || "";
  $signal = get_case( "S", $signal ) unless $signal =~ /^\$/;
  if ( $comp ) {
	$comp = get_case( "C", $comp );
	$comp = "($comp)";
  }
  $name = "$prefix$signal$comp";
  return $name;
}

sub define_sigdesc {
  my ( $signal, $desc ) = @_;
  $signal = define_sigcase($signal);
  $signal =~ s/^~//;
  if ( $SIGNAL::sigdesc{$signal} ) {
	my $old = $SIGNAL::sigdesc{$signal};
	if ( $old ne $desc ) {
	  warn "${SIGNAL::context}: Replacing description of $signal\n",
	   " Was: $old\n Now: $desc\n";
	}
  }
  $SIGNAL::sigdesc{$signal} = $desc;
}
sub get_sigdesc {
  my ( $sigcomp ) = @_;
  $sigcomp =~ m/^~?(\$?[-+_\w]+)(\(([_\w]+)\))?$/ ||
	die "$SIGNAL::context:get_sigdesc: Signal '$sigcomp' out of spec\n";
  $SIGNAL::sigdesc{$1};
}

sub define_locsig {
  my ( $signal, $comp ) = @_;
  $comp = get_case( 'C', $comp );
  $signal = define_sigcase($signal);
  $SIGNAL::sigcomps{$signal} = {} unless $SIGNAL::sigcomps{$signal};
  $SIGNAL::sigcomps{$signal}->{$comp} = 1;
  # Default global name is the fully qualified name
  my $sigcomp = "$signal($comp)";
  unless ( $SIGNAL::globsig{$sigcomp} ) {
	if ( $SIGNAL::sighash{$sigcomp} ) {
	  warn "${SIGNAL::context}: Global signal $sigcomp already defined!\n";
	  return;
	}
	$SIGNAL::globsig{$sigcomp} = $sigcomp;
	$SIGNAL::sighash{$sigcomp} = { $sigcomp => 1 };
  }
}

#----------------------------------------------------------------
# $sigcomp[12] must be defined local signals before calling
# equate_signals
#----------------------------------------------------------------
sub equate_signals {
  my ( $sigcomp1, $sigcomp2 ) = @_;
  $sigcomp1 = get_sigcase($sigcomp1);
  $sigcomp2 = get_sigcase($sigcomp2);
  #----------------------------------------------------------------
  # Get global names for $sigcomp1, $sigcomp2
  # For each member of $global2, add to $global1
  # Delete $global2
  #----------------------------------------------------------------
  my ( $global1, $global2 );
  ( $global1 = $SIGNAL::globsig{$sigcomp1} ) ||
	die "$SIGNAL::context: Local signal $sigcomp1 not defined";
  ( $global2 = $SIGNAL::globsig{$sigcomp2} ) ||
	die "$SIGNAL::context: Local signal $sigcomp2 not defined";
  unless ( $global1 eq $global2 ) {
	foreach my $sigcomp ( keys %{$SIGNAL::sighash{$global2}} ) {
	  $SIGNAL::globsig{$sigcomp} = $global1;
	  $SIGNAL::sighash{$global1}->{$sigcomp} = 1;
	}
	delete $SIGNAL::sighash{$global2};
  }
}
sub rename_global {
  my ( $global1, $global2 ) = @_;
  unless ( $global1 && $global2 ) {
	warn "${SIGNAL::context}: Invalid arguments '$global1,$global2'",
		 " to rename_global()\n";
  }
  unless ( $global1 eq $global2 ) {
	if ( $SIGNAL::sighash{$global2} ) {
	  warn "${SIGNAL::context}: Cannot rename $global1 to $global2\n";
	  return;
	}
	foreach my $sigcomp ( keys %{$SIGNAL::sighash{$global1}} ) {
	  $SIGNAL::globsig{$sigcomp} = $global2;
	}
	$SIGNAL::sighash{$global2} = $SIGNAL::sighash{$global1};
	delete $SIGNAL::sighash{$global1};
  }
}


sub define_comptype {
  my ( $comptype, $desc ) = @_;
  $comptype = define_case( "T", $comptype );
  unless ( $SIGNAL::comptype{$comptype} ) {
	my $ct = $SIGNAL::comptype{$comptype} = {};
	$ct->{'conns'} = [];
	$ct->{'conn'} = {};
	$ct->{'comps'} = [];
  }
  my $ct = $SIGNAL::comptype{$comptype};
  warn "$SIGNAL::context:$comptype: Warning: ",
		"Changing Desc from $ct->{'desc'} to $desc\n"
	if ( $ct->{'desc'} && ( $ct->{'desc'} ne $desc ) );
  $ct->{'desc'} = $desc;
}

sub define_comp {
  my ( $comp, $comptype ) = @_;
  $comp = define_case( "C", $comp );
  $comptype = define_case( "T", $comptype );
  unless ( $SIGNAL::comptype{$comptype} ) {
	warn "$SIGNAL::context: ",
	  "Comptype '$comptype' undefined when defining comp '$comp'\n";
	define_comptype( $comptype, "" );
  }
  if ( $SIGNAL::comp{$comp} ) {
	if ( $SIGNAL::comp{$comp}->{type} ne $comptype ) {
	  warn "$SIGNAL::context: ",
		"Attempt to redefine comptype for '$comp' from ",
		"$SIGNAL::comp{$comp}->{type} to $comptype\n";
	}
  } else {
	$SIGNAL::comp{$comp} = {};
	$SIGNAL::comp{$comp}->{type} = $comptype;
	push( @{$SIGNAL::comptype{$comptype}->{'comps'}}, $comp );
  }
}
sub define_compdesc {
  my ( $comp, $desc ) = @_;
  $comp = define_case( "C", $comp );
  unless ( $SIGNAL::comp{$comp} ) {
	define_comp( $comp, $comp );
  }
  $SIGNAL::comp{$comp}->{'desc'} = $desc;
}
sub define_ctconn {
  my ( $comptype, $conn, $conntype, $desc ) = @_;
  my $ct = $SIGNAL::comptype{$comptype} || die;
  unless ( $ct->{'conn'}->{$conn} ) {
	$ct->{'conn'}->{$conn} = {};
	push( @{$ct->{'conns'}}, $conn );
  }
  my $ctd = $ct->{'conn'}->{$conn};
  if ( $desc ) {
    #### Reinstate this with comp-specific descriptions
	#warn "$SIGNAL::context:$comptype:$conn: ",
	#	  "Changing Desc from $ctd->{'desc'} to $desc\n"
	#  if ( $ctd->{'desc'} && ( $ctd->{'desc'} ne $desc ) );
	$ctd->{'desc'} = $desc;
  }
  if ( $conntype ) {
	warn "$SIGNAL::context: Changing conntype ",
		 "from $ctd->{type} to $conntype\n"
	  if ( $ctd->{'type'} && ( $ctd->{'type'} ne $conntype ) );
	$ctd->{'type'} = $conntype;
  }
}
sub define_conncomp {
  my ( $conncomp, $conntype, $desc, $globname ) = @_;
  my ( $conn, $comp ) = split_conncomp( $conncomp );
  return unless $conn;
  $conncomp = make_conncomp( $conn, $comp );
  unless( $SIGNAL::comp{$comp} &&
		  $SIGNAL::comp{$comp}->{type} ) {
	warn "$SIGNAL::context: Component $comp undefined\n";
	SIGNAL::define_comptype( $comp, '' );
	SIGNAL::define_comp( $comp, $comp );
  }
  my $comptype = $SIGNAL::comp{$comp}->{type};
  define_ctconn( $comptype, $conn, $conntype, $desc );
  
  #----------------------------------------------------------------
  # Process Globalname
  #----------------------------------------------------------------
  my ( $gconn, $gcomp ) = split_conncomp( $globname );
  $globname = make_conncomp( $gconn, $gcomp );
  if ( $globname ne $conncomp ) {
	$SIGNAL::comp{$comp}->{alias} = {}
	  unless defined $SIGNAL::comp{$comp}->{alias};
	$SIGNAL::comp{$comp}->{alias}->{$conn} = $globname;
  }
  $SIGNAL::conlocname{$globname} = $conncomp;
  my $cable = SIGNAL::get_cable_name($globname);
  $SIGNAL::comp{$comp}->{cable} = {}
	unless defined $SIGNAL::comp{$comp}->{cable};
  if ( $SIGNAL::comp{$comp}->{cable}->{$conn} ) {
	my $oldcable = $SIGNAL::comp{$comp}->{cable}->{$conn};
	die "$SIGNAL::context:$conncomp: ",
	  "Cable reassignment from '$oldcable' to '$cable'"
	  if $cable ne $oldcable;
  } else {
	$SIGNAL::comp{$comp}->{cable}->{$conn} = $cable;
	$SIGNAL::cable{$cable} = [] unless $SIGNAL::cable{$cable};
	push( @{$SIGNAL::cable{$cable}}, $conncomp );
  }
}

# Global Properties
# set_global( Property, Value );
# get_global( Property );
sub set_global {
  my ( $prop, $val ) = @_;
  $SIGNAL::global{$prop} = $val;
}
sub get_global {
  my ( $prop ) = @_;
  return $SIGNAL::global{$prop};
}

#----------------------------------------------------------------
# <signal> may have embedded spaces, which are replaced with '_'
#  Also at present any embedded '/' chars are replaced with '_'
#  and any embedded '.' chars are deleted, but in these two
#  cases, the translation is output for review.
#----------------------------------------------------------------
sub signal_from_txt {
  my ( $signal ) = @_;
  $signal = "" unless defined $signal;
  $signal =~ s/^\s*//; # leading spaces
  $signal =~ s/\s*$//; # trailing spaces
  return $signal unless $signal;
  my $oldsig = $signal;
  $signal =~ s/ /_/g;  # spaces -> '_' in signal
  my $refsig = $signal;
  $signal =~ s/\.//g;
  $signal =~ s|^[/\\](.*)$|~$1|;
  $signal =~ s|^(.*)[/\\]$|~$1|;
  $signal =~ s|/|_|g;
  $signal =~ m/^~?\$?[-+_\w]+$/ ||
	warn "$SIGNAL::context: Signal $oldsig out of spec\n";
  $signal ne $refsig &&
	warn "$SIGNAL::context: Translation: $oldsig -> ${signal}\n";
  return $signal;
}

sub save_signals {
  local $Data::Dumper::Indent = 1;
  local $Data::Dumper::Purity = 1;
  mkdirp( "net" );
  open(SAVESIG, ">net/SIGNAL.dat" ) ||
	die "Unable to create net/SIGNAL.dat\n";
  print SAVESIG Data::Dumper->Dump(
	[ \%SIGNAL::sigcomps, \%SIGNAL::globsig, \%SIGNAL::sighash,
	  \%SIGNAL::sigdesc, \%SIGNAL::case,
	  \%SIGNAL::comp, \%SIGNAL::comptype,
	  \%SIGNAL::global, \%SIGNAL::cable,
	  \%SIGNAL::conlocname, \%SIGNAL::sigcfg ],
	[ "*SIGNAL::sigcomps", "*SIGNAL::globsig", "*SIGNAL::sighash",
	  "*SIGNAL::sigdesc", "*SIGNAL::case",
	  "*SIGNAL::comp", "*SIGNAL::comptype",
	  "*SIGNAL::global", "*SIGNAL::cable",
	  "*SIGNAL::conlocname", "*SIGNAL::sigcfg" ]
  );
  close(SAVESIG) || warn "$SIGNAL::context: Error closing net/SIGNAL.dat\n";
}

sub load_signals {
  if ( open(SAVESIG, "<net/SIGNAL.dat" ) ) {
	seek( SAVESIG, 0, 2 );
	my $len = tell(SAVESIG);
	seek( SAVESIG, 0, 0 );
	my $buf;
	my $rv = read( SAVESIG, $buf, $len );
	if ( $rv ) {
	  $rv = eval( $buf );
	  if ( ! defined( $rv ) ) {
		die "Error in load_signals: $@\n";
	  } elsif ( ! $rv ) {
		die "Error returned in load_signals\n";
	  }
	} else {
	  warn "$SIGNAL::context: Error reading from net/SIGNAL.dat\n";
	}
	close(SAVESIG) ||
	  warn "$SIGNAL::context: Error closing net/SIGNAL.dat\n";
  }
}

#----------------------------------------------------------------
# define_pins( <pkg_type>, \@pins )
# Defines the @pins array based on the package type.
# Ultimately, this will follow a configurable heuristic,
# defaulting to using just the pins already defined.
#----------------------------------------------------------------
my %dp_warned;

sub define_pins {
  my ( $pkg_type_src, $pins ) = @_;

  my $pkg_type = $pkg_type_src;
  $pkg_type =~ s|/|_|g;
  if ( open_nets( *PINFILE{FILEHANDLE}, "pkg/$pkg_type.pkg" ) ) {
	@$pins = <PINFILE>;
	close PINFILE ||
	  warn "$SIGNAL::context: error closing pkg/$pkg_type.pkg\n";
	chomp @$pins;
  } elsif ( $pkg_type =~
	  m/^(AMP|AH|C|DB|E|HCA|HD|JP|M|MT|MTA|P|PAD|PH|SE|SIP|SW|WIR)-?(\d+)[MF]?$/i ) {
	@$pins = ( 1 .. $2 );
  } else {
	unless ( defined $dp_warned{$pkg_type_src} ) {
	  warn "$SIGNAL::context: Unable to resolve ",
		   "package type: \"$pkg_type_src\"\n";
	  $dp_warned{$pkg_type_src} = 1;
	}
	if ( @$pins ) {
	  mkdirp( "net/pkg" );
	  open( PINFILE, ">net/pkg/$pkg_type.pkg" ) ||
		die "$SIGNAL::context: Unable to create net/pkg/$pkg_type.pkg\n";
	  foreach my $pin ( @$pins ) {
		print PINFILE "$pin\n";
	  }
	  close PINFILE ||
		warn "$SIGNAL::context: Error closing net/pkg/$pkg_type.pkg\n";
	}
	# @$pins = sort @$pins;
  }
  my $npins = @$pins;
}

#----------------------------------------------------------------
# load_netlist interprets a PADS-style netlist, collection
# connector information.
# I should register connectors not yet defined
# Compare pkgtype of connectors with the present definitions
#----------------------------------------------------------------
#   Enters the signal/pin association into two data structures:
#    %conn = { <conn> => { <pin> => <signal> } }
#    %sig = { <signal> => { <conn.pin> => <conn.pin> } } (linked list
#        of links)
#    %other = { <conn> => <pkg_type> }
# (Actual updates are made via SIGNAL::define_pinsig() )
#----------------------------------------------------------------
# Returns the number of NETLIST files read ( 0, 1 or 2 )
# including sym/$comptype/NETLIST and/or comp/$comp/NETLIST.BACK.
# Stops reading after the first failure.
#----------------------------------------------------------------
# The special comptype value '#CABLE' indicates that $comp is
# a cable, not a component, and looks for cable/$comp/NETLIST*
# instead. If not found, it will take a stab and creating
# a netlist on the fly.
#----------------------------------------------------------------
sub load_netlist {
  my ( $comptype, $comp, $co, $sig, $other ) = @_;
  my %trans;
  my %trans2;
  my %trans3;
  my $iscable = ( $comptype eq '#CABLE' );
  my $typedir = $iscable ? "cable/$comp" : "sym/$comptype";
  my $transfile = "$typedir/NETLIST.NDC";
  my $nfiles = 0;
  my $ctconn;
  if ( $iscable ) {
	$ctconn = {};
	foreach my $conncomp ( @{$SIGNAL::cable{$comp}} ) {
	  my ( $conn, $ccomp, $globalalias, $cable, $def ) =
		get_conncomp_info( $conncomp );
	  $ctconn->{$globalalias} = $def;
	}
  } else {
	$ctconn = $SIGNAL::comptype{$comptype}->{conn};
  }

  load_netlist_trans( $transfile, \%trans );
  load_netlist_trans( "${transfile}2", \%trans2 );
  load_netlist_trans( "comp/$comp/NETLIST.NDC", \%trans3 )
	if $comp && ! $iscable;
#  local $SIGNAL::context = $transfile;
#  if ( open_nets( *NETLIST{FILEHANDLE}, "$SIGNAL::context" ) ) {
#	while (<NETLIST>) {
#	  chomp;
#	  my ( $signal, $alias ) = split;
#	  while ( defined $trans{$signal} ) {
#		$signal = $trans{$signal};
#	  }
#	  $trans{$alias} = $signal;
#	}
#	close NETLIST || warn "$SIGNAL::context: Error closing\n";
#  }
  my @netlists = ( "$typedir/NETLIST" );
  push( @netlists, "comp/$comp/NETLIST.BACK" ) if $comp && ! $iscable;
  foreach my $netlist ( @netlists ) {
	local $SIGNAL::context = $netlist;
	if ( open_nets( *NETLIST{FILEHANDLE}, "$SIGNAL::context" ) ) {
	  # Skip to the beginning of the PART section
	  while (<NETLIST>) {
		next if (/^$/);
		last if (/^\*PART\*/);
		last if (/^\*NET\*/);
	  }
  
	  # Pick out connector definitions
	  if ( /^\*PART\*/ ) {
		while (<NETLIST>) {
		  next if (/^$/);
		  last if (/^\*NET\*/);
		  if ( /^(\S+)\s+(([^@]+)@)?(\S+)\s*$/ ) {
			my ( $refdes, $symname, $pkg_type ) = ( $1, $3, $4 );
			{ my $pkg = $pkg_type;
			  if ( $pkg_type =~ s/[^-\w]/_/g ) {
				warn "$SIGNAL::context: pkg_type translated: ",
					"refdes $refdes $pkg ->	$pkg_type\n";
			  }
			}
			$symname = $pkg_type unless defined $symname;
			if ( defined $ctconn->{$refdes} ) {
			  my $conn = $ctconn->{$refdes};
			  warn "$SIGNAL::context: ",
				   "pkg_type changed: ",
				   "$refdes was $conn->{type} now $pkg_type\n"
				if $conn->{type} ne $pkg_type;
			} elsif ( $refdes =~ m/^J\d+$/ ) {
			  if ( $iscable ) {
				warn "$SIGNAL::context: ",
				  "conn not previously on cable: $refdes\n";
			  } else {
				warn "$SIGNAL::context: refdes previously undefined: ",
					  "$refdes\n";
				SIGNAL::define_ctconn( $comptype, $refdes, $pkg_type, "" );
			  }
			} elsif ( defined $other ) {
			  $other->{$refdes} = "$symname\@$pkg_type";
			}
		  }
		}
	  }

	  # Now we're at EOF or the beginning of *NET*
	  my $signal;
	  while (<NETLIST>) {
		next if (/^$/);
		last if (/^\*END\*/);
		chop;
		@_ = split;
		if (/^\*SIGNAL\*/) {
		  $signal = $_[1];
		  foreach my $trans ( \%trans, \%trans2, \%trans3 ) {
			while ( defined $trans->{$signal} && $trans->{$signal} ne $signal ) {
			  $signal = $trans->{$signal};
			}
		  }
		  $signal =~ s|^(.*)\\~|~$1_|;
		  $signal =~ s|\\|_|g;
		  $signal = SIGNAL::get_sigcase( $signal );
		} else {
		  foreach my $pin ( @_ ) {
			if ( $pin =~ m/^([^.]+)\.(.+)$/ &&
				 ( ( $comp && ! $iscable ) || defined $ctconn->{$1} ) ) {
			  my ( $conn, $pin ) = ( $1, $2 );
			  define_pinsig( $conn, $pin, $signal, $co, $sig );
			}
		  }
		}
	  }
	  close NETLIST || warn "$SIGNAL::context: Error closing\n";
	  $nfiles++;
	} else { last; }
  }
  
  # Now make up a netlist for cables
  if ( $nfiles == 0 && $iscable && scalar(keys %$ctconn) > 1 ) {
	local $SIGNAL::context = "cable/$comp/NETLIST";
	my %pinset;
	my %types;
	foreach my $ga ( keys %$ctconn ) {
	  my $pkg_type = $ctconn->{$ga}->{type} || die;
	  $types{$pkg_type} = 1;
	  my @pins;
	  define_pins( $pkg_type, \@pins );
	  if ( $pkg_type =~ m/^C-?(\d+)$/ ) {
		use integer;
		# shuffle cannons to flat cable order
		# Cannons have (n+1)/2 pins on top and (n-1)/2 on bottom
	    my $n = $1;
		my $m = ($n + 1) / 2;
		@pins = ( ( map { ( $_, $_+$m ) } (1 .. $m-1) ), $m );
	  }
	  $pinset{$ga} = [ @pins ];
	}
	if ( scalar(keys %types) > 1 ) {
	  warn "$SIGNAL::context:$comp: ",
		"Assuming Cable Order between ",
		join( ", ", keys %types ), "\n";
	}
	for ( my $pinno = 1; ; $pinno++ ) {
	  my $anypins = 0;
	  my $signal = "\$$pinno";
	  foreach my $ga ( %pinset ) {
		my $pin = shift( @{$pinset{$ga}} ) || next;
		$anypins = 1;
		define_pinsig( $ga, $pin, $signal, $co, $sig );
	  }
	  last unless $anypins;
	}
  }

  # Now massage the %sig structure to provide the
  # circular link-to list
  local $SIGNAL::context = "load_netlist:" .
	$iscable ? $comptype : "#CABLE:$comp";
  foreach my $signal ( keys %$sig ) {
	my @links = SIGNAL::get_links( $signal, $sig );
	my $links = $sig->{$signal};
	my $first = shift @links;
	if ( @links > 0 ) {
	  my $prev = $first;
	  foreach my $link ( @links ) {
		$links->{$link} = $prev;
		$prev = $link;
	  }
	  $links->{$first} = $prev;
	} else {
	  $links->{$first} = '';
	}
  }
  return $nfiles;
}

sub load_netlist_trans {
  my ( $transfile, $trans ) = @_;
  
  local $SIGNAL::context = $transfile;
  if ( open_nets( *NETLIST{FILEHANDLE}, "$SIGNAL::context" ) ) {
	while (<NETLIST>) {
	  chomp;
	  my ( $signal, $alias ) = split;
	  while ( defined $trans->{$signal} &&
			  $trans->{$signal} ne $signal ) {
		$signal = $trans->{$signal};
	  }
	  $trans->{$alias} = $signal;
	}
	close NETLIST || warn "$SIGNAL::context: Error closing\n";
  }
}

sub write_transfile {
  my ( $dir, $file, $trans ) = @_;
  my @renames = keys %$trans;
  my %src;
  my %dest;
  my %fwd;
  my %rev;
  if ( @renames > 0 ) {
	map { m/^(\w+):(\w+)$/ || die;
		  $src{$1}++; $dest{$2}++;
		  $fwd{$1} = $2;
		  $rev{$2} = $1;
	} @renames;
	my @srcerr = grep $src{$_} > 1, keys %fwd;
	my @desterr = grep $dest{$_} > 1, keys %rev;
	map ###########
	local $SIGNAL::context = "$dir/$file";
	my %rev;
	foreach my $sig ( keys %$trans ) {
	  my $tgt = $trans->{$sig};
	  if ( $rev{$tgt} ) {
		if ( $rev{$tgt}++ == 1 ) {
		  warn "$SIGNAL::context: ",
			"Multiple signals mapped to '$tgt'\n";
		}
	  } else {
		$rev{$tgt} = 1;
	  }
	}
	mkdirp($dir);
	open( OFILE, ">$SIGNAL::context" ) ||
	  die "$SIGNAL::context: Unable to write file\n";
	map { print OFILE "$trans->{$_} $_\n"; }
	  sort keys %$trans;
	close OFILE || warn "$SIGNAL::context: Error closing file\n";
	warn "$SIGNAL::context: Wrote component translation file\n";
  }
}

# define_pinsig( $conn, $pin, $signal, \%conn, \%sig )
#   Enters the signal/pin association into two data structures:
#    %conn = { <conn> => { <pin> => <signal> } }
#    %sig = { <signal> => { <conn.pin> => 1 } }
sub define_pinsig {
  my ( $conn, $pin, $signal, $co, $sig ) = @_;
  return unless $signal;
  local $SIGNAL::context = "$SIGNAL::context:$conn.$pin";
  $co->{$conn} = {} unless defined $co->{$conn};
  if ( defined $co->{$conn}->{$pin} ) {
	my $oldsig = $co->{$conn}->{$pin};
	if ( $oldsig ne $signal ) {
	  warn "$SIGNAL::context: Attempted signal redef from ",
		"$oldsig to $signal\n";
	}
  }
  $co->{$conn}->{$pin} = $signal;
  $sig->{$signal} = {} unless defined $sig->{$signal};
  $sig->{$signal}->{"$conn.$pin"} = 1;
}

sub SIGNAL::split_conncomp {
  my ( $conncomp ) = @_;
  if ( $conncomp =~ m/^([A-Za-z0-9]+(_\w+)?)_([A-Za-z][A-Za-z0-9]*)$/ ||
	   $conncomp =~ m/^([A-Za-z]+\d+)(([A-Za-z][A-Za-z0-9]*))$/ ) {
	my ( $conn, $comp ) = ( $1, $3 );
	$comp = define_case( "C", $comp );
	return ( $conn, $comp );
  } else {
	return undef;
  }
}

sub SIGNAL::make_conncomp {
  my ( $conn, $comp ) = @_;
  $comp =~ m/^[A-Za-z][A-Za-z0-9]*$/ ||
	die "$SIGNAL::context: Illegal Component name '$comp'\n";
  $conn =~ m/^[A-Za-z0-9]+(_\w+)?$/ ||
	die "$SIGNAL::context: Illegal Connector name '$conn'\n";
  $conn =~ m/^[A-Za-z]+[0-9]+$/ ? "$conn$comp" : "${conn}_$comp";
}

#  my ( $conn, $comp, $globalalias, $cable, $def ) =
sub SIGNAL::get_conncomp_info {
  my $conncomp = shift;
  my ( $conn, $comp ) = split_conncomp( $conncomp );
  return undef unless $conn;
  my $comptype = $SIGNAL::comp{$comp}->{type} ||
	die "get_conncomp_info: comptype undefined for '$conncomp'\n";
  my $globalalias = SIGNAL::get_global_alias($conncomp);
  my $cable = $SIGNAL::comp{$comp}->{cable}->{$conn};
  my $def = $SIGNAL::comptype{$comptype}->{conn}->{$conn};
  ( $conn, $comp, $globalalias, $cable, $def );
}

my %warned_no_addr;

sub SIGNAL::get_address {
  my ( $signal ) = @_;
  $signal = ( defined $SIGNAL::sighash{$signal} ?
	$signal : $SIGNAL::globsig{$signal} ) || return '';
  my ( $type, $addr, $comp );
  foreach my $locsig ( keys %{$SIGNAL::sighash{$signal}} ) {
	if ( $locsig =~ m/^_AD(.)[Xx]([0-9A-F]+)\((.*)\)$/ ) {
	  ( $type, $addr, $comp ) = ( $1, $2, $3 );
	  last;
	}
  }
  return '' unless $type && $addr && $comp;
  unless ( $SIGNAL::comp{$comp} ) {
	warn "Component undefined for address $addr\n";
	return '';
  }
  unless ( defined $SIGNAL::comp{$comp}->{base} &&
			$SIGNAL::comp{$comp}->{base} ne '' ) {
	unless ( $warned_no_addr{$comp} ) {
	  warn "No base address defined for component $comp\n";
	  $warned_no_addr{$comp} = 1;
	}
	return '';
  }
  my $base = hex $SIGNAL::comp{$comp}->{base};
  $addr = sprintf "%X", $base + hex $addr;
  "0x$addr";
}

sub SIGNAL::get_cable_name {
  my $alias = shift;
  my ( $aconn, $acomp ) = SIGNAL::split_conncomp( $alias );
  $aconn =~ s/^J(\d+(_\w+)?)$/$1/;
  "P$aconn";
}

sub SIGNAL::get_global_alias {
  my $conncomp = shift;
  my ( $conn, $comp ) = SIGNAL::split_conncomp($conncomp);
  $SIGNAL::comp{$comp}->{alias}->{$conn} || $conncomp;
}

BEGIN {
  if ( $^O eq "MSWin32" ) {
	eval "use Win32::Registry;";
  }
}
#---------------------------------------------------------------
# Resolve directory
#   First look for options on command line:
#---------------------------------------------------------------
sub siginit {
  my ( $logfile, $project_dir ) = @_;
  my $from_registry = 0;

  unless ( $project_dir && $^O eq "MSWin32") {
	sub getsubkey {
	  my ( $keys, $subkey ) = @_;
	  my $newkey;
	  $keys->[0]->Open( $subkey, $newkey ) || return 0;
	  unshift( @$keys, $newkey );
	  return 1;
	}
	my $keys = [ $main::HKEY_CURRENT_USER ];
	foreach my $subkey ( qw( Software HUARP Nets BaseDir ) ) {
	  goto KEY_NOT_FOUND unless getsubkey( $keys, $subkey );
	}
	my %vals;
	$keys->[0]->GetValues( \%vals ) || die "GetValues failed\n";
	$project_dir = $vals{''}->[2];
	$from_registry = 1 if $project_dir;
  
	KEY_NOT_FOUND:
	while ( my $key = shift(@$keys) ) {
	  $key->Close;
	}
  }

  use Cwd;
  $project_dir =~ s|[/\\]nets.ini$||i;
  $project_dir = getcwd if ( ! $project_dir ) && -r "nets.ini";

  die "Unable to locate project directory '$project_dir'\n"
	unless $project_dir && -d $project_dir;

  if ( $^O eq "MSWin32" && ! $from_registry ) {
	print "TXT2ILI: Changing to Project Dir $project_dir\n";
	sub createsubkey {
	  my ( $keys, $subkey ) = @_;
	  my $newkey;
	  $keys->[0]->Create( $subkey, $newkey ) || return 0;
	  unshift( @$keys, $newkey );
	  return 1;
	}
	my $keys = [ $main::HKEY_CURRENT_USER ];
	foreach my $subkey ( qw( Software HUARP Nets BaseDir ) ) {
	  die "Unable to create registry subkey $subkey\n"
		unless createsubkey( $keys, $subkey );
	}
	$keys->[0]->SetValue( '', REG_SZ, $project_dir ) ||
	  die "SetValue failed\n";
	while ( my $key = shift(@$keys) ) {
	  $key->Close;
	}
  }

  chdir( $project_dir ) || die "Unable to chdir $project_dir\n";
  $project_dir;
}

# returns a sorted array of links for the specified signal excluding
# the pins listed in @exclude and rotated so the first link is the
# next after $exclude[0].
sub get_links {
  my ( $signal, $sig, @exclude ) = @_;
  return () unless $signal;
  $signal = SIGNAL::get_sigcase($signal);
  my $links = $sig->{$signal};
  my @links = keys( %$links );
  @links =
	map { $_->[0] }
	sort { $b->[1] cmp $a->[1] ||
		   $b->[2] <=> $a->[2] ||
		   $b->[3] cmp $a->[3] ||
		   $b->[4] <=> $a->[4] ||
		   $b->[0] cmp $a->[0] }
	map {
	  $_ =~ m/^(.*\D)(\d*)\.(.*\D)?(\d*)$/ ||
		die "Could not parse link '$_'";
	  [ $_, $1, $2 || 0, $3 || '', $4 || 0 ];
	} @links;
  if ( @exclude > 0 ) {
	my $first = shift @exclude;
	if ( @exclude > 0 ) {
	  my %exclude = map { $_ => 1 } @exclude;
	  @links = grep ! $exclude{$_}, @links;
	}
	my @rest;
	while ( @links > 0 ) {
	  my $pin = shift @links;
	  last if $pin eq $first;
	  push( @rest, $pin );
	}
	push( @links, @rest );
  }
  @links;
}

#----------------------------------------------------------------
# Slices for signals
#----------------------------------------------------------------
sub pick_slice {
  my ( $signal ) = @_;
  foreach my $slice ( @SIGNAL::slices ) {
	return $slice if $signal =~ m/^~?[$slice]/i;
  }
  return "etc";
}

1;
