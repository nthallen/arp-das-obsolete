#! /usr/local/bin/perl
# Global Interconnect Resolution
#----------------------------------------------------------------
# We no longer need to gather data, since it's all in the
# database.

use FindBin;
use lib "$FindBin::Bin";
use SIGNAL;
use NETSLIB qw(open_nets mkdirp );

$| = 1;

BEGIN { SIGNAL::load_signals(); }
END { SIGNAL::save_signals(); }

#----------------------------------------------------------------
# For each Cable, load listings for each connector and equate the
# signals.
#
# This is where we need to deal with non 1:1 cables:
#  Let's try some heuristics:
#   If all connectors are the same type, assume 1:1
#   If one end is cannon and the other is euro, assume flat
#      cable connection.
#   If more than two, assume listings define interconnect
#----------------------------------------------------------------
# This mapping needs to be carried to the lister program somehow,
# or the lister program has to do the same legwork. Could make
# %SIGNAL::cable = { <cable> => { type => <type>, conns => ( ) }}
#----------------------------------------------------------------
# Back Annotation for unassigned pins (net/comp/$comp/NETLIST.BACK)
# %backann = { <comp> => [ [ $conn, $pinname, $firstsig ] ] }
#----------------------------------------------------------------
my %backann;

print "Globalic\n";
foreach my $cable ( keys %SIGNAL::cable ) {
  $connlist = $SIGNAL::cable{$cable};
  if ( @$connlist > 1 ) {
	# print "Processing cable $cable\n";

	#------------------------------------------------------------
	# @conns will be an array of hash refs containing
	# [ conn, comp, conntype, comptype, lines ]
	# where lines is an array of array refs containing
	# [ $pinname, $signal, $link-to ] (from *.list)
	#------------------------------------------------------------
	my @conns;
	foreach my $conncomp ( @$connlist ) {
	  $conncomp =~ m/^(\w+):(\w+)$/ ||
	  $conncomp =~ m/^(J\d+)(\D.*)$/ ||
	  die "Cable $cable conn $conncomp out of spec\n";
	  my ( $conn, $comp ) = ( $1, $2 );
	  my $comptype = $SIGNAL::comp{$comp}->{type} ||
		die "comptype undefined for cable $cable, conncomp $conncomp\n";
	  my $conndef = $SIGNAL::comptype{$comptype}->{conn}->{$conn};
	  # print "  $conncomp  $comp $conn $conndef->{type}\n";
	  push( @conns, { conn => $conn, comp => $comp,
		conntype => $conndef->{type}, comptype => $comptype } );
	}

	# Load the listings and define local signals here
	foreach my $ccc ( @conns ) {
	  $SIGNAL::context = "sym/$ccc->{comptype}/$ccc->{conn}.list";
	  unless ( open_nets( *IFILE{FILEHANDLE}, $SIGNAL::context ) ) {
		warn "$SIGNAL::context: No listing found\n";
		next;
	  }
	  my @lines = <IFILE>;
	  close IFILE || warn "$SIGNAL::context: Error closing\n";
	  chomp @lines;
	  @lines = map { [ split /:/ ]; } @lines;
	  foreach my $line ( @lines ) {
		SIGNAL::define_locsig( $line->[1], $ccc->{comp} ) if $line->[1];
	  }
	  $ccc->{lines} = \@lines;
	}
	
	foreach my $ccc ( @conns ) {
	  goto not_one_to_one if $ccc->{conntype} ne $conns[0]->{conntype};
	}
	# print "  $cable is one to one\n";
	goto equate;

	not_one_to_one:
	goto octopus unless
	  @conns == 2 &&
	  ( ( $conns[0]->{conntype} =~ m/^C-\d+$/ &&
		  $conns[1]->{conntype} =~ m/^E-\d+$/ ) ||
		( $conns[1]->{conntype} =~ m/^C-\d+$/ &&
		  $conns[0]->{conntype} =~ m/^E-\d+$/ ) );
	print "  $cable is flat between cannon and euro\n";
	foreach my $ccc ( @conns ) {
	  if ( $ccc->{conntype} =~ m/^C-(\d+)$/ ) {
		use integer;
		# shuffle cannons to flat cable order
		# Cannons have (n+1)/2 pins on top and (n-1)/2 on bottom
	    my $n = $1;
		my $m = ($n + 1) / 2;
		my @v = ( ( map { ( $_, $_+$m ) } (0 .. $m-2) ), $m-1 );
		my @newlines = @{$ccc->{lines}}[ @v ];
		$ccc->{lines} = \@newlines;
		# print "ccc->{lines} is ", ref $ccc->{lines}, " ref\n";
	  }
	}

	equate:
	# I have two or more connectors for which I need to 
	# equate signals.
	#  For each pin, 
	my $pin = -1;
	while ( 1 ) {
	  $pin++;
	  my $firstsig = '';
	  my $anypins = '';
	  foreach my $ccc ( @conns ) {
		if ( $ccc->{lines} && $pin < @{$ccc->{lines}} ) {
		  $anypins = 'y';
		  my ( $pinname, $signal ) = @{$ccc->{lines}->[$pin]};
		  if ( $signal ) {
			$signal = "$signal($ccc->{comp})";
			if ( $firstsig ) {
			  SIGNAL::equate_signals( $firstsig, $signal );
			} else {
			  $firstsig = "$signal";
			}
		  }
		}
	  }
	  last unless $anypins;
	  next unless $firstsig;
	  foreach my $ccc ( @conns ) {
		if ( $ccc->{lines} && $pin < @{$ccc->{lines}} ) {
		  my ( $pinname, $signal ) = @{$ccc->{lines}->[$pin]};
		  unless ( $signal ) {
			# back annotate to $ccc->{comp}$ccc->{conn}.$pinname
			$backann{$ccc->{comp}} = [] unless $backann{$ccc->{comp}};
			push( @{$backann{$ccc->{comp}}},
			  [ $ccc->{conn}, $pinname, $firstsig ] );
		  }
		}
	  }
	}
	next;

	octopus:
	print "  $cable is an octopus\n";
	# Need to create a little netlist. Use hash of hashes
	# %netlist => { <sig> => { <comp> => 1 } }
	my %netlist;
	foreach my $ccc ( @conns ) {
	  my $comp = $ccc->{comp};
	  foreach my $pin ( @{$ccc->{lines}} ) {
		my $sig = $pin->[1];
		if ( $sig ) {
		  $netlist{$sig} = {} unless $netlist{$sig};
		  $netlist{$sig}->{$comp} = 1;
		}
	  }
	}
	foreach my $sig ( keys %netlist ) {
	  my @comps = keys %{$netlist{$sig}};
	  my $first = shift @comps;
	  foreach my $comp ( @comps ) {
		SIGNAL::equate_signals( "$sig($first)", "$sig($comp)" );
	  }
	}
  }
}
print "Done Equating\n";

# After all the signals have been equated, we need to find
# canonical global names for each. To do this, we need to sort
# each set according to the rules in global and pick the first
# best name.

my %comp_rank;
if ( $SIGNAL::global{comp_rank} ) {
  my @comps = grep /./, split /[,\s]+/, $SIGNAL::global{comp_rank};
  for ( my $i = 0; $i < @comps; $i++ ) {
    $comp_rank{$comps[$i]} = @comps - $i;
  }
}

foreach my $group ( keys %SIGNAL::sighash ) {
  my @locsigs = keys %{$SIGNAL::sighash{$group}};
  # Now I'd like to sort these to list the best
  # Candidates first, i.e. global names
  # names beginning with $ or _AD cannot be global
  # Would like to consider names in a defined order
  # (Mainly, use MDP id first)
  # So: sort by
  #   Whether signal begins with $ or _AD
  #   Whether signal is definitely local or not
  #   Whether it's on a preferred board or not
  #   Alphabetically
  my @ssigs = map {
	  $_ =~ m/^([^(]+)\(([^)]+)\)$/ ||
		die "$SIGNAL::context: LocSig '$_' out of spec in sort\n";
	  my ($sig,$comp) = ($1,$2);
	  my $isspecial = ( $sig =~ m/^(\$|_AD)/ ) ? 1 : 0;
	  # $isspecial = 1 if $comp =~ m/^WDP/; # Temporary Kluge
	  my $islocal = $isspecial;
	  if ( ! $islocal ) {
		foreach my $comp ( keys %{$SIGNAL::sigcomps{$sig}} ) {
		  defined $SIGNAL::globsig{"$sig($comp)"} || die;
		  unless ( $SIGNAL::globsig{"$sig($comp)"} eq $group ) {
			$islocal = 1;
		  }
		}
	  }
	  [ $islocal, $comp_rank{$comp} || 0, $sig, $comp, $isspecial ];
	} @locsigs;
  @ssigs =
	sort { $a->[4] <=> $b->[4] || # ! isspecial
		   $a->[0] <=> $b->[0] || # ! islocal
		   $b->[1] <=> $a->[1] || # component rank
		   $a->[2] cmp $b->[2] || # signal name (then comp)
		   $a->[3] cmp $b->[3] } @ssigs;
  my $newname = $ssigs[0]->[0] ? "$ssigs[0]->[2]($ssigs[0]->[3])" :
				$ssigs[0]->[2];
  if ( $newname ne $group ) {
	SIGNAL::rename_global( $group, $newname );
  }
}

# Now do the real back annotation
# Go through backann and get global signal names
# For each $comp, sort array by global signal name, then
# write NETLIST.BACK
foreach my $comp ( keys %backann ) {
  foreach my $backann ( @{$backann{$comp}} ) {
	if ( my $sig = $SIGNAL::globsig{$backann->[2]} ) {
	  $backann->[2] = $sig;
	}
  }
  my @list = sort { $a->[2] cmp $b->[2] ||
					$a->[0] cmp $b->[0] ||
					$a->[1] cmp $b->[1] } @{$backann{$comp}};
  mkdirp( "net/comp/$comp" );
  $SIGNAL::context = "net/comp/$comp/NETLIST.BACK";	
  print "Writing $SIGNAL::context\n";
  open( BACKANN, ">$SIGNAL::context" ) ||
	die "$SIGNAL::context: unable to open\n";
  print BACKANN "*NET*\n";
  my $currsig = '';
  foreach ( @list ) {
	my ( $conn, $pinname, $sig ) = @$_;
	if ( $currsig ne $sig ) {
	  $currsig = $sig;
	  print BACKANN "\n*SIGNAL* $currsig 12\n";
	}
	print BACKANN "$conn.$pinname ";
  }
  print BACKANN "\n";
  close( BACKANN ) || warn "$SIGNAL::context: Error closing\n";
}
