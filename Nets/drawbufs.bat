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
use CGI qw(-no_debug start_html end_html h2 center);
use SIGNAL;

my $nets_dir = SIGNAL::siginit( 'drawbufs', 0, shift @ARGV );
SIGNAL::LogMsg "DrawBufs $nets_dir:\n";
SIGNAL::load_signals();

die "No Buffer_Template_Dir defined in nets.ini\n" unless
  $SIGNAL::global{Buffer_Template_Dir};

# Pick up the gifpins database
my %gifpins;
use Fcntl;
use SDBM_File;
if ( $SIGNAL::global{gifs} ) {
  require sch2gif;
  tie( %gifpins, 'SDBM_File', 'net/gifpins',
		O_RDWR|O_CREAT, 0640);
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

# my $rep = VLSchem::Load( 'sch', 'template:buf2128.1' );
# my $bufsym = VLSchem::ResolveName('sym','harvard:ina2128.2');
# $bufsym->{decouple} = 'template:decouple.1';

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
		# warn "$datum->{name}: Signal unconnected: $name\n" unless $sig{$name};
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
  } elsif ( $item->[0] =~ m/^I/ ) {
	my $refdes = $dest->get_refdes( $item );
	# my ( $refdes ) = map { /^A.*REFDES=(\w+)$/ ? $1 : () } @$item;
	if ( $refdes && $datum->{value}->{$refdes} ) {
	  my $newval = $datum->{value}->{$refdes};
	  return 0 if $newval eq 'DELETE';
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
my $comp = 'MDP'; # kluge to specify component
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
die "Missing MAIN_REGION or DEC_REGION attributes in template:$1.1"
  unless defined $border->{main_region} &&
		 defined $border->{dec_region};
# $border->{main_region} =
#  { xmin=>50, ymin=>300, xmax=>1650, ymax=>2020 };
# $border->{dec_region} =
#  { xmin=>60, ymin=>60, xmax=>1100, ymax=>340 };

SIGNAL::load_netlist( $comptype, $comp, \%conn, \%sig );

my $sch = $border->Create( $minsheet, $maxsheet );

NETSLIB::mkdirp( "net/comp/$comp" );
open( AREAFILE, ">net/comp/$comp/areas.dat" ) ||
  die "$SIGNAL::context:$comp: Unable to open areas.dat\n";

foreach my $conn ( @conns ) {
  # figure out the pkg_type
  local $SIGNAL::context = "$conn";
  my $pkgtype = $SIGNAL::comptype{$comptype}->{conn}->{$conn}->{type};
  my @pins;
  SIGNAL::define_pins($pkgtype, \@pins);
  foreach my $pin (@pins) {
	my $signal = $conn{$conn}->{$pin};
	my $datum = configure_signal( $signal );
	if ( $datum ) {
	  my $rep = VLSchem::Load( 'sch', "template:$datum->{template}" );
	  next unless $rep;
	  my $lo = $sch->Copy( $rep, \&transform, $datum );
	  FixupLinks( $sch, $lo, $datum );
	  SIGNAL::LogMsg "Signal: Processed $signal\n";

	  if ( $SIGNAL::global{gifs} ) {
		my ( $w, $h ) = @{$rep->{extents}}{'xmax', 'ymax'};
		my $hc = sch2gif::new( @{$rep->{extents}}{'xmax', 'ymax'} );
		my ( $x, $y ) = @{$sch->{lastxy}};
		$hc->begin_transform( -$x, -$y, 0, 1 );
		my @output;
		$hc->html_options( \%gifpins, \@output, $comp, $signal );
		$hc->draw_sch( $sch, $lo );
		NETSLIB::mkdirp( "html/$comp" ) ;
		$hc->save( "html/$comp/$signal.gif" );
		print AREAFILE "GIF:$signal:$w,$h\n";
		foreach my $area ( @output ) {
		  $area =~ m/^([^:]+):([^:]+):(.*)$/ || die;
		  my ( $coords, $type, $line ) = ( $1, $2, $3 );
		  my $href;
		  if ( $type eq 'I' ) {
			my $refdes = $line;
			print AREAFILE "REFDES:$coords:$line\n";
		  } elsif ( $type eq 'A-LINKTO' && $line =~ m/^([^.]+\.\w+)/ ) {
			print AREAFILE "LINKTO:$coords:$1\n";
		  } elsif ( $type eq 'L' && $line !~ m|/| ) {
			print AREAFILE "SIGNAL:$coords:$line\n";
		  }
		}
		print AREAFILE "\n";
	  }
	}
  }
}

#my $autospare = 1;
#while ( $bufsym->{freeslots} ) {
#  $sch->Copy( $rep, \&transform, { name => "AUTOSP_$autospare" } );
#  $autospare++;
#}
$sch->Write();

untie %gifpins;

close AREAFILE ||
  warn "$SIGNAL::context:$comp: Error closing areas.dat\n";

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
  my @schpins;
  my %delitems;
  @schpins = $sch->list_pins( $lo );
  my $hi = $#{$sch->{item}};
  foreach my $i ( $lo .. $hi ) {
	my $item = $sch->{item}->[$i];
	my $head = $item->[0];
	if ( $head =~ m/^N/ ) {
	  my $label = $sch->get_label( $item );
	  my @lnks = SIGNAL::get_links( $label, \%sig, @schpins );
	  my $links = "No Link";
	  @lnks = ( @lnks[0,1], "Etc." ) if @lnks > 2;
	  $links = join( ", ", @lnks ) if @lnks > 0;
	  map {
		$_ =~ s/\sLINKTO=.*$/ LINKTO=$links/;
		if ( $links eq "No Link" && $label ) {
		  warn "$SIGNAL::context:$label: Signal unconnected\n";
		}
	  } grep m/^A.*LINKTO=/, @$item;
	} elsif ( $head =~ m/^I/ &&
			  $sch->get_attr( $item, 'VALUE' ) =~ /^DELETE/ ) {
	  $delitems{$i} = 1;
	}
  }
  if ( scalar(keys %delitems) > 0 ) {
	my @v = grep ! $delitems{$_}, ( 0 .. $hi );
	@{$sch->{item}} = @{$sch->{item}}[@v];
  }
}

sub configure_signal {
  my $signal = shift;
  if ( defined $SIGNAL::sigcfg{$signal} ) {
	my $cfg = $SIGNAL::sigcfg{$signal};
	my ( $sigtype, $addr, $conv, $gain, $vr, $rate, $bw, $therm,
		  $pu, $pub, $bufloc, $comment ) = @$cfg;
	if ( $sigtype eq 'AI' && $bufloc =~ m/^$comp:(.+)$/ ) {
	  my $bcfg = $1;
	  unless ( defined $SIGNAL::bufcfg{$bcfg} ) {
		warn "$SIGNAL::context: Unknown bufcfg: $bcfg\n";
		return undef;
	  }
	  my $bufc = $SIGNAL::bufcfg{$bcfg};
	  my $desc = $SIGNAL::sigdesc{$signal} || $signal;
	  $desc .= ", $rate Hz" if $rate;
	  $desc .= ", $bufc->{description}";
	  my $datum = {
		name => $signal,
		desc => $desc,
		template => $bufc->{template},
		value => $bufc->{value},
		label => $bufc->{label}
	  };
	  return $datum;
	}
  }
  return undef;
}


__END__
:endofperl
