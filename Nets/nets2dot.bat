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
use NETSLIB qw(open_nets mkdirp find_nets);

my $nets_dir = SIGNAL::siginit( "nets2dot", 0, shift @ARGV );
SIGNAL::LogMsg "Nets2dot: $nets_dir\n";

$SIGNAL::context = 'Nets2dot';
SIGNAL::load_signals();

open( DOT, ">inter.dot" ) ||
  die "Unable to write dot file\n";

print DOT
  "graph G {\n",
  "  size=\"8,10\"; ratio=fill;\n",
  "  node [ shape=record ];\n";

foreach my $comp ( sort keys %SIGNAL::comp ) {
  my $compdef = $SIGNAL::comp{$comp};
  my $comptype = $SIGNAL::comptype{$compdef->{type}};
  my $conns = $comptype->{conns};
  if ( @$conns ) {
	print DOT
	  "  $comp [ label=\"$comp|",
	  join( "|", map "<$_>$_", @{$comptype->{conns}} ),
	  "\" ];\n";
  }
}

foreach my $cable ( values %SIGNAL::cable ) {
  print DOT "  ",
	join( " -- ",
	  map { my ( $conn, $comp ) = SIGNAL::split_conncomp($_);
	        "$comp:$conn"; } @$cable ),
	";\n";
}

print DOT "}\n";
close DOT;

__END__
:endofperl
