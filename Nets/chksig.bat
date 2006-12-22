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

# access registry to locate Nets project dir
# This code is used by drawsch.bat also. Should really go in a
# library routine.
my $nets_dir = '';
{ use Win32::Registry;

  my @keys;
  sub getsubkey {
    my ( $key, $subkey ) = @_;
    my $newkey;
    ${$key}->Open( $subkey, $newkey ) || return 0;
    push( @keys, $newkey );
    $$key = $newkey;
    return 1;
  }
  my $key = $main::HKEY_CURRENT_USER;
  foreach my $subkey ( qw( Software HUARP Nets BaseDir ) ) {
    die "Nets Project Directory is undefined (in Registry)\n"
      unless getsubkey( \$key, $subkey );
  }
  my %vals;
  $key->GetValues( \%vals ) || die "GetValues failed\n";
  $nets_dir = $vals{''}->[2];
  
  while ( $key = pop(@keys) ) {
    $key->Close;
  }
}
die "Unable to locate nets project directory\n"
  unless $nets_dir && -d $nets_dir && chdir $nets_dir;

my $logfile = "chksig";

open( LOGFILE, ">$logfile.err" ) ||
  die "Unable to open log file\n";

$SIG{__WARN__} = sub {
  print LOGFILE @_;
  warn @_;
};

sub LogMsg {
  print LOGFILE @_;
  print STDERR @_;
}

$SIG{__DIE__} = sub {
  warn @_;
  print STDERR "\nHit Enter to continue:";
  my $wait = <STDIN>;
  print STDERR "\n";
  exit(1);
};

LogMsg "Signal Check\n";


SIGNAL::load_signals();

my @sigs = sort keys %SIGNAL::sigcomps;
foreach my $signal ( @sigs ) {
  my %gn;
  foreach my $comp ( keys %{$SIGNAL::sigcomps{$signal}} ) {
	my $locsig = "$signal($comp)";
	my $gn = $SIGNAL::globsig{$locsig} || "UNDEFINED";
	$gn{$gn} = $locsig;
  }
  my @gn = sort keys %gn;
  if ( @gn > 1 ) {
	LogMsg "$signal incarnations:",
	  join( "\n  ", '', map( "$gn{$_} => $_", @gn )), "\n";
  } else {
	my $gn = $gn[0];
	$gn =~ s/\(.*\)$//;
	if ( $signal !~ /^\$/ && $gn ne $signal ) {
	  LogMsg "$signal:\n  $gn{$gn[0]} => $gn[0]\n";
	}
  }
}

__END__
:endofperl
