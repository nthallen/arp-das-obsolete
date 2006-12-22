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

use strict;
use FindBin;
use lib "$FindBin::Bin";
use SIGNAL;
# use NETSLIB qw(open_nets find_nets mkdirp);

$| = 1;

my ( $conncomp ) = shift @ARGV;
die "Must specify conncomp\n" unless $conncomp;
my ( $conn, $comp ) = SIGNAL::split_conncomp( $conncomp );
SIGNAL::siginit( '', 0 );
# The following is in lieu of actually reading in the database
# If we do decide to read it in, don't do this.
SIGNAL::define_comptype( $comp, "Dummy desc" );
SIGNAL::define_comp( $comp, $comp );

my %conn;
my %sig;
my %visited;

SIGNAL::load_netlist( $comp, $comp, \%conn, \%sig );

my $sigwidth = max( map length, keys %sig );

open( OFILE, ">$comp.def" ) ||
  die "Unable to create $comp.def\n";

print OFILE "Component $comp cable link-tos\n\n";

{ my $conntype = $SIGNAL::comptype{$comp}->{conn}->{$conn}->{type} ||
	die;
  my @pins;
  print OFILE "Connector $conn $conntype:\n\n";
  SIGNAL::define_pins( $conntype, \@pins );
  my $pinwidth = max( map length, @pins );
  my $hdwidth = $sigwidth + $pinwidth + 5;
  foreach my $pin ( @pins ) {
	my $sig = $conn{$conn}->{$pin} || "";
	printf OFILE "%${pinwidth}s: %-${sigwidth}s:  ", $pin, $sig;
	my $col = $hdwidth;
	if ( $sig ) {
	  my $siglist = $sig{$sig} || die;
	  my $start = "$conn.$pin";
	  $visited{$start} = 1;
	  for ( my $crnt = $siglist->{$start};
			$crnt && $crnt ne $start;
			$crnt = $siglist->{$crnt} ) {
		if ( $col + length($crnt) > 70 ) {
		  print OFILE "\n", " " x $hdwidth;
		  $col = $hdwidth;
		}
		print OFILE " $crnt";
		$col += 1 + length($crnt);
		$visited{$crnt} = 1;
	  }
	}
	print OFILE "\n";
  }
}
foreach my $conn ( @{$SIGNAL::comptype{$comp}->{conns}} ) {
  my $conntype = $SIGNAL::comptype{$comp}->{conn}->{$conn}->{type} ||
	die;
  my @pins;
  print OFILE "\nConnector $conn $conntype:\n\n";
  SIGNAL::define_pins( $conntype, \@pins );
  my $pinwidth = max( map length, @pins );
  foreach my $pin ( @pins ) {
	my $start = "$conn.$pin";
	next if $visited{$start};
	my $sig = $conn{$conn}->{$pin} || "";
	if ( $sig ) {
	  printf OFILE "%${pinwidth}s: %-${sigwidth}s:  ", $pin, $sig;
	  my $siglist = $sig{$sig} || die;
	  $visited{$start} = 1;
	  for ( my $crnt = $siglist->{$start};
			$crnt && $crnt ne $start;
			$crnt = $siglist->{$crnt} ) {
		print OFILE " $crnt";
		$visited{$crnt} = 1;
	  }
	  print OFILE "\n";
	}
  }
}

sub max {
  my $val = shift;
  for ( @_ ) {
	$val = $_ if $_ > $val;
  }
  $val;
}

__END__
:endofperl
