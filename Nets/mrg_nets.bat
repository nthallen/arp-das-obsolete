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
#line 14
# mrg_nets net1 net2 [...]
# rewrites NETLIST combining the specified nets under the name
# of the first net.

my $n_nets = @ARGV;
if ( $n_nets < 2 ) {
  die "usage: mrg_nets net1 net2 [...]\n";
}

my $sigpat = join( '|', map "\Q$_\E", @ARGV );
my $sigline = '';
my @pins;

open( INET, "<NETLIST" ) ||
  die "Unable to read NETLIST\n";
open( ONET, ">NETLIST.new" ) ||
  die "Unable to write NETLIST.new\n";

while (<INET>) {
  while ( m/^\*SIGNAL\*\s($sigpat)\s/i ) {
	$sigline = $_ if $1 =~ m/^\Q$ARGV[0]\E$/i;
	while (<INET>) {
	  last if m/^(\*SIGNAL\*\s.*)?$/;
	  push( @pins, $_ );
	}
	if ( --$n_nets == 0 ) {
	  die "No sigline!\n" unless $sigline ne '';
	  die "No pins!" unless @pins;
	  print ONET $sigline, @pins;
	}
  }
  print ONET if $_;
}

if ( $n_nets > 0 ) {
  die "Did not find all the nets!\n";
}

close INET || die "Error closing NETLIST\n";
close ONET || die "Error closing NETLIST.new\n";
unlink( "NETLIST.bak" );
rename( "NETLIST", "NETLIST.bak" ) || die "Unable to rename NETLIST\n";
rename( "NETLIST.new", "NETLIST" ) || die "Unable to rename NETLIST.new\n";


__END__
:endofperl
