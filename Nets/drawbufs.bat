use lib "d:/nets";
use VLSchem;
use Win32::OLE qw();

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
  my $key = $HKEY_CURRENT_USER;
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

open( LOGFILE, ">drawbufs.err" ) ||
  die "Unable to open log file\n";

$SIG{__WARN__} = sub {
  print LOGFILE $_[0];
  warn $_[0];
};

$SIG{__DIE__} = sub {
  my $msg = $_[0];
  chomp $msg;
  print LOGFILE "$msg\n";
  print STDERR "$msg\nHit Enter to continue:";
  my $wait = <STDIN>;
  print STDERR "\n";
  exit(1);
};

sub logmsg {
  print @_;
  print LOGFILE @_;
}

logmsg "DrawBufs $nets_dir:\n";

use SIGNAL;
SIGNAL::load_signals();

die "No Buffer_Template_Dir defined in nets.ini\n" unless
  $SIGNAL::global{Buffer_Template_Dir};

VLSchem::AddLibrary( '' => "C:/design/clono2" );
VLSchem::AddLibrary( template => $SIGNAL::global{Buffer_Template_Dir} );
VLSchem::AddLibrary( harvard => "C:/wvoffice/harvlib" );

# This script is customized for use in the ClONO2 project to generate
# buffers on the Main DP (MDP).

my $border = VLSchem::Load( 'sch', "template:mdpbuf.1" );
$border->{main_region} =
  { xmin=>50, ymin=>300, xmax=>1650, ymax=>2020 };
$border->{dec_region} =
  { xmin=>60, ymin=>60, xmax=>1100, ymax=>340 };

my $rep = VLSchem::Load( 'sch', 'template:buf2128.1' );
my $bufsym = VLSchem::ResolveName('sym','harvard:ina2128.2');
$bufsym->{decouple} = 'template:decouple.1';


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
	foreach my $elt ( grep $item->[$_] =~ m/^L/, (1 .. $#$item) ) {
	  if ( $datum->{label} ) {
		foreach my $label ( keys %{$datum->{label}} ) {
		  $item->[$elt] =~ s/\sDATUM_$label$/ $datum->{label}->{$label}/;
		}
	  }
	  if ( $item->[$elt] =~ s/\sDATUM(\w*)$/ $datum->{name}$1/ ) {
		# Check to see if a PAD is required for signal
		# $datum->{name}$1
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

my $cfg = $SIGNAL::global{Buffer} ||
  die "No buffer configuration in Nets.ini\n";
$cfg =~ m/^(\w+):(.+)$/ || die;
my $comp = $1;
my @conns = split(',',$2);
my $comptype = $SIGNAL::comp{$comp}->{type} || die;
my %conn;
my %sig;
SIGNAL::load_netlist( $comptype, $comp, \%conn, \%sig );

my $sheet = 0;
my $sch = $border->Create( ++$sheet );

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
			warn "Signal $signal specifies therm but no pullup\n";
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
		  warn "Not configured for signal $signal\n";
		}
		if ( defined $value ) {
		  $sch->Copy( $rep, \&transform,
			{ name=>$signal, value => $value, desc => $desc } );
		  logmsg "Signal: $signal\n";
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
__END__

