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
# Global Interconnect Resolution
#----------------------------------------------------------------
# We no longer need to gather data, since it's all in the
# database.

use strict;
use FindBin;
use lib "$FindBin::Bin";
use SIGNAL;
use NETSLIB qw(open_nets mkdirp );

$| = 1;

SIGNAL::siginit('globalic', 1, shift @ARGV );
SIGNAL::LogMsg "Global Interconnect ", join( " ", @ARGV ), "\n";


SIGNAL::load_signals();

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

foreach my $cable ( keys %SIGNAL::cable ) {
  local $SIGNAL::context = "globalic:$cable";
  my $connlist = $SIGNAL::cable{$cable};
  if ( @$connlist > 1 ) {
	# print "Processing cable $cable\n";

	#------------------------------------------------------------
	# %conns will be a hash (index on global alias) of hash refs
	# containing:
	# [ conn, comp, conntype, comptype, lines ]
	# where lines is an array of array refs containing
	# [ $pinname, $signal, $link-to ] (from *.list)
	# where lines is a hash of array refs containing
	# $pinname => [ $signal, $link-to ] (from *.list)
	#------------------------------------------------------------
	my %conns;
	foreach my $conncomp ( @$connlist ) {
	  my ( $conn, $comp, $globalalias, $ccable, $def ) =
		SIGNAL::get_conncomp_info( $conncomp );
	  die unless $ccable eq $cable;
	  my $comptype = $SIGNAL::comp{$comp}->{type};
	  $conns{$globalalias} = { conn => $conn, comp => $comp,
		conntype => $def->{type}, comptype => $comptype };
	}

	# Load the listings and define local signals here
	foreach my $ccc ( values %conns ) {
	  local $SIGNAL::context = "sym/$ccc->{comptype}/$ccc->{conn}.list";
	  unless ( open_nets( *IFILE{FILEHANDLE}, $SIGNAL::context ) ) {
		warn "$SIGNAL::context: No listing found\n";
		next;
	  }
	  my @lines = <IFILE>;
	  close IFILE || warn "$SIGNAL::context: Error closing\n";
	  chomp @lines;
	  my %lines = map {
		my ( $pin, @etc ) = split /:/;
		( $pin, [ @etc ] ); }
		  @lines;
	  foreach my $pin ( keys %lines ) {
		SIGNAL::define_locsig( $lines{$pin}->[0], $ccc->{comp} )
		  if $lines{$pin}->[0];
	  }
	  $ccc->{lines} = \%lines;
	}

	my %conn;
	my %sig;
	SIGNAL::load_netlist( '#CABLE', $cable, \%conn, \%sig );
	foreach my $signal ( keys %sig ) {
	  local $SIGNAL::context = "globalic:$cable:$signal";
	  my @pins = keys %{$sig{$signal}};
	  my $firstsig = '';
	  my @back;
	  foreach my $connpin ( @pins ) {
		local $SIGNAL::context = "globalic:$cable:$signal:$connpin";
		$connpin =~ m/^([\w:]+)\.(\w+)$/ || die;
		my ( $conn, $pin ) = ( $1, $2 );
		my $ccc = $conns{$conn} || die;
		my $signal = $ccc->{lines}->{$pin}->[0] || '';
		if ( $signal ) {
		  $signal = "$signal($ccc->{comp})";
		  if ( $firstsig ) {
			SIGNAL::equate_signals( $firstsig, $signal );
		  } else {
			$firstsig = "$signal";
		  }
		} else {
		  push( @back, [ $ccc->{comp}, $ccc->{conn}, $pin ] );
		}
	  }
	  if ( $firstsig ) {
		foreach my $back ( @back ) {
		  my ( $comp, $conn, $pin ) = @$back;
		  # back annotate to $ccc->{comp} $ccc->{conn}.$pinname
		  $backann{$comp} = [] unless $backann{$comp};
		  push( @{$backann{$comp}}, [ $conn, $pin, $firstsig ] );
		  
		}
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
# Go through backann and get global signal names: NOT!
# For each $comp, sort array by global signal name, then
# write NETLIST.BACK
foreach my $comp ( keys %backann ) {
  #foreach my $backann ( @{$backann{$comp}} ) {
  # if ( my $sig = $SIGNAL::globsig{$backann->[2]} ) {
  #	  $backann->[2] = $sig;
  #	}
  #}
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

SIGNAL::save_signals();
__END__
:endofperl
