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
#!perl
#line 14
use FindBin;
use lib "$FindBin::Bin";
use VLSchem;
use sch2gif;
use CGI qw(-no_debug start_html end_html h2 center);

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

my $logfile = "drawbufs";

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

END {
  if ( defined $SIG{__WARN__} ) {
	delete $SIG{__WARN__};
	delete $SIG{__DIE__};
	close LOGFILE;
	unlink( "$logfile.bak" );
	rename( "$logfile.err", "$logfile.bak" );
	open( IFILE, "<$logfile.bak" ) ||
	  die "Unable to read $logfile.bak";
	open( OFILE, ">$logfile.err" ) ||
	  die "Unable to rewrite $logfile.err";
	print OFILE
	  map $_->[0],
		sort { $a->[1] cmp $b->[1] || $a->[0] cmp $b->[0] }
		  map { $_ =~ m/:\s+(.*)$/ ? [ $_, $1 ] : [ $_, '' ] } <IFILE>;
	close OFILE;
	close IFILE;
  }
}

LogMsg "DrawBufs $nets_dir:\n";

use SIGNAL;
SIGNAL::load_signals();

die "No Buffer_Template_Dir defined in nets.ini\n" unless
  $SIGNAL::global{Buffer_Template_Dir};

# Pick up the gifpins database
my %gifpins;
use Fcntl;
use SDBM_File;
if ( $SIGNAL::global{gifs} ) {
  tie( %gifpins, 'SDBM_File', 'net/gifpins',
		O_RDWR|O_CREAT, 0640);
  NETSLIB::mkdirp( "html/MDP" ) ;
}

# Identify Viewlogic_Project_File from database
my $ProjFile = $SIGNAL::global{Viewlogic_Project_File} ||
  die "Viewlogic_Project_File is not defined\n";
VLSchem::OpenProject( $ProjFile );
VLSchem::AddLibrary( template => $SIGNAL::global{Buffer_Template_Dir} );

# This script is customized for use in the ClONO2 project to generate
# buffers on the Main DP (MDP).
# Should be generalized to generate buffers for all boards listed
# under Draw_Bufs or Draw_Components

my $border = VLSchem::Load( 'sch', "template:main_dp.1" );
$border->{main_region} =
  { xmin=>50, ymin=>300, xmax=>1650, ymax=>2020 };
$border->{dec_region} =
  { xmin=>60, ymin=>60, xmax=>1100, ymax=>340 };

my $rep = VLSchem::Load( 'sch', 'template:buf2128.1' );
my $bufsym = VLSchem::ResolveName('sym','harvard:ina2128.2');
$bufsym->{decouple} = 'template:decouple.1';

# These two moved up here to be accessible from within transform()
my %conn;
my %sig;

#----------------------------------------------------------------
# $dest is the schematic. $item has already been added, although
#   the REFDESs haven't been reset, etc.
# $item = [ ] array of schematic lines
# Store datum information in hash:
# $datum = {
#   name => datum name
#   label => { DATUM_LO => HDP1_LO }
#   value => { REFDES => 75K }
# }
#----------------------------------------------------------------
sub transform {
  my ( $dest, $item, $datum ) = @_;
  if ( $item->[0] =~ m/^N/ ) {
	my $label;
	foreach my $elt ( grep $item->[$_] =~ m/^L/, (1 .. $#$item) ) {
	  if ( $datum->{label} ) {
		foreach my $label ( keys %{$datum->{label}} ) {
		  $item->[$elt] =~ s/\s$label$/ $datum->{label}->{$label}/;
		}
	  }
	  if ( $item->[$elt] =~ m/\sDATUM(\w*)$/ ) {
		my $suffix = $1;
		my $name = SIGNAL::get_sigcase( "$datum->{name}$suffix" );
		$item->[$elt] =~ s/\sDATUM(\w*)$/ $name/;
		warn "$datum->{name}: Signal unconnected: $name\n" unless $sig{$name};
		# Check to see if a PAD is required for signal
		# $datum->{name}$1
	  }
	  if ( $item->[$elt] =~ m/\s([-+\w]+)$/ ) {
		if ( $label ) {
		  warn "$datum->{name}: Conflicting labels: '$label' and '$1'\n"
			if $1 ne $label;
		} else {
		  $label = $1;
		}
	  } else {
		warn "$datum->{name}: Unable to parse label: '$item->[$elt]'\n";
	  }
	}
	if ( $label ) {
	  my @links =
		grep $item->[$_] =~ m/^A.*\sLINKTO=/, (1 .. $#$item);
	  if ( @links > 0 ) {
		$label = SIGNAL::get_sigcase($label);
		my @lnks = keys %{$sig{$label}};
		if ( @lnks > 2 ) {
		  @lnks = ( @lnks[0,1], "Etc." );
		}
		my $links = $sig{$label} ? join ', ', @lnks :
			"No Link";
		foreach my $elt ( @links ) {
		  $item->[$elt] =~ s/\sLINKTO=.*$/ LINKTO=$links/;
		}
	  }
	}
  } elsif ( $item->[0] =~ m/^I/ ) {
	my ( $refdes ) = map { /^A.*REFDES=(\w+)$/ ? $1 : () } @$item;
	if ( $refdes && $datum->{value}->{$refdes} ) {
	  map s/^(A.*VALUE=).*$/$1$datum->{value}->{$refdes}/, @$item;
	}
  } elsif ( $item->[0] =~ m/^T(\s\d+){5}\sDESCRIPTION/ ) {
	my $desc = $datum->{desc} || '';
	return 0 unless $desc ne '';
	$item->[0] =~ s/^(T(\s\d+){5})\sDESCRIPTION.*$/$1 $desc/;
  }
  1;
}

# my $cfg = $SIGNAL::global{Buffer} ||
#  die "No buffer configuration in Nets.ini\n";
my $comp = 'MDP';
my $comptype = $SIGNAL::comp{$comp}->{type} || die;
my $schrange = $SIGNAL::comptype{$comptype}->{bufsch} ||
  die "No schematic specified for comp '$comp'\n";
my $schconns = $SIGNAL::comptype{$comptype}->{bufconns} ||
  die "No connector defs for comp '$comp'\n";
my @conns = split(',',$schconns);

$schrange =~ m/^(\w+)\.(\d+)(-(\d+))?$/ ||
  die "Did not understand schematic range '$schrange'\n";
my $minsheet = $2;
my $maxsheet = $4 || $2;

my $border = VLSchem::Load( 'sch', "template:$1.1" );
$border->{main_region} =
  { xmin=>50, ymin=>300, xmax=>1650, ymax=>2020 };
$border->{dec_region} =
  { xmin=>60, ymin=>60, xmax=>1100, ymax=>340 };

SIGNAL::load_netlist( $comptype, $comp, \%conn, \%sig );

my $sch = $border->Create( $minsheet, $maxsheet );

foreach my $conn ( @conns ) {
  # figure out the pkg_type
  my $pkgtype = $SIGNAL::comptype{$comptype}->{conn}->{$conn}->{type};
  my @pins;
  SIGNAL::define_pins($pkgtype, \@pins);
  foreach my $pin (@pins) {
	my $signal = $conn{$conn}->{$pin};
	if ( defined $SIGNAL::sigcfg{$signal} ) {
	  my $cfg = $SIGNAL::sigcfg{$signal};
	  my ( $sigtype, $addr, $conv, $gain, $vr, $rate, $bw, $therm,
			$pu, $pub, $bufloc, $comment ) = @$cfg;
	  if ( $sigtype eq 'AI' && $bufloc eq $comp ) {
		my $value;
		my $desc = $SIGNAL::sigdesc{$signal} || $signal;
		$desc .= ", $rate Hz" if $rate;
		if ( $therm ) {
		  $desc .= ", T$therm";
		  if ( $pu ) {
			$desc .= ", $pu Pullup";
			$value = { R1 => $pu, R4 => 'SHORT' };
		  } else {
			warn "$conn.$pin: Signal specifies therm but no pullup: $signal\n";
		  }
		} elsif ( $vr =~ m/^0-10v$/i ) {
		  $desc .= ", $vr";
		  $value = { R3 => 'SHORT', R5 => '688K' };
		} elsif ( $vr =~ m/^Vref$/i ) {
		  $desc .= ", Unity Gain";
		  $value = {};
		} elsif ( $vr =~ m/^0-5V$/i ) {
		  $desc .= ", $vr";
		  $value = { R3 => 'SHORT', R2 => '220K', R5 => '1M' };
		} else {
		  warn "$conn.$pin: Not configured for signal: $signal\n";
		}
		if ( defined $value ) {
		  my $datum = {
			name => $signal,
			value => $value,
			desc => $desc,
			label => {}
		  };
		  if ( $comment =~ m/\bLO=(\w+)\b/ ) {
			$datum->{label}->{DATUM_LO} = $1;
			$datum->{desc} .= ", LO=$1";
		  }
		  if ( $rate eq '1/16' ) {
			warn "$conn.$pin: Signal 1/16 Hz: $signal\n";
		  }
		  my $lo = scalar(@{$sch->{item}});
		  $sch->Copy( $rep, \&transform, $datum );
		  # FixupLinks( $sch, $lo, $datum );
		  LogMsg "Signal: Processed $signal\n";

		  if ( $SIGNAL::global{gifs} ) {
			my ( $w, $h ) = @{$rep->{extents}}{'xmax', 'ymax'};
			my $hc = sch2gif::new( @{$rep->{extents}}{'xmax', 'ymax'} );
			my ( $x, $y ) = @{$sch->{lastxy}};
			$hc->begin_transform( -$x, -$y, 0, 1 );
			my @output;
			$hc->html( \%gifpins, \@output, 'MDP', $signal );
			$hc->draw_sch( $sch, $lo );
			$hc->save( "html/MDP/$signal.gif" );
			if ( open( OFILE, ">html/MDP/$signal.html" ) ) {
			  print OFILE
				start_html(
				  '-title' => "$SIGNAL::global{Exp} MDP $signal Buffer",
				  '-author' => "allen\@huarp.harvard.edu"
				),
				"\n",
				center(
				  h2( "$SIGNAL::global{Experiment}<BR>MDP $signal Buffer" )),
				"\n",
				"<IMG SRC=\"$signal.gif\" WIDTH=$w HEIGHT=$h BORDER=0 ",
				  "ALT=\"\">\n",
				end_html;
			  close OFILE ||
				warn "$SIGNAL::context: error closing $signal.html\n";
			} else {
			  warn "$SIGNAL::context: ",
				"Unable to write to $signal.html\n";
			}
		  }
		}
	  }
	}
  }
}

my $autospare = 1;
while ( $bufsym->{freeslots} ) {
  $sch->Copy( $rep, \&transform, { name => "AUTOSP_$autospare" } );
  $autospare++;
}
$sch->Write();

untie %gifpins;

# FixupLinks is called after each Copy is completed.
# $sch is the target schematic
# $lo is the index of the first item in the Copy operation
# $datum is the datum structure as defined above.
#----------------------------------------------------------------
# For my purposes here, I need to:
#   1. Identify all the pins which exist in this copy ($refdes.$pin)
#   2. Fixup any LINKTO attributes by {
#        Finding all the links from the netlist
#        Eliminating any links in this copy
#        Truncating the list after 2
#      }
sub FixupLinks {
  my ( $sch, $lo, $datum ) = @_;
}

__END__
:endofperl
