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
use FindBin;
use lib "$FindBin::Bin";
use VLSchem;

# Strategy: Given a symbol file and an optional list of label/pin
# associations, add # attributes or alter existing # attributes
# to match the new association.
# repin symbolfile [pindeffile]

my $projdir = "C:/design/CIMS";
my ( $symfile, @pinfile ) = @ARGV;
my %lbl2pin;
my %pin2lbl;
my $defpinno = 1;

foreach my $pinfile ( @pinfile ) {
  open( IFILE, "<$projdir/$pinfile" ) ||
	die "Unable to open pinfile \"$projdir/$pinfile\"\n";
  while (<IFILE>) {
	if ( /place instance -?(\w+)_pad\s+:\sP(\d+)\s;/ ) {
	  my ( $label, $pin ) = ( $1, $2 );
	  if ( defined $lbl2pin{$label} &&
		   $lbl2pin{$label} ne $pin ) {
		warn "Label \"$label\" assigned to both $lbl2pin{$label}",
		  " and $pin\n";
	  } else {
		$lbl2pin{$label} = $pin;
	  }
	  if ( defined $pin2lbl{$pin} &&
			$pin2lbl{$pin} ne $label ) {
		warn "Pin $pin labeled both $pin2lbl{$pin} and $label\n";
	  } else {
		$pin2lbl{$pin} = $label;
	  }
	}
  }
}

print "Modifying sym/$symfile\n";
VLSchem::AddLibrary( '' => $projdir );
my $sym = VLSchem::Load( 'sym', $symfile ) ||
  die;

foreach my $comp ( grep $_->[0] =~ /^P/, @{$sym->{item}} ) {
  my $label = $sym->get_label( $comp );
  my @label;
  my @number;
  if ( $label =~ m/^(\w+)\[(\d+):(\d+)\]$/ ) {
	my ( $pre,$from,$to) = ( $1, $2, $3 );
	@label = map "$pre$_", ( $from .. $to );
  } else {
	@label = ( $label );
  }
  foreach my $label ( @label ) {
	my $number;
	if ( $lbl2pin{$label} ) {
	  $number = $lbl2pin{$label};
	} else {
	  while ( $pin2lbl{$defpinno} ) {
		$defpinno++;
	  }
	  $number = $lbl2pin{$label} = $defpinno;
	  $pin2lbl{$number} = $label;
	  warn "Using default pin $number for label $label\n";
	}
	push( @number, $number );
  }
  my $number = join( ',', @number );
  my $pin = $sym->get_attr( $comp, "#" );
  if ( $pin ) {
	next if $pin eq $number;
	foreach ( @$comp ) {
	  s/^(A .+ #=).*$/$1$number/;
	}
  } else {
	# Pick label location based on pin def
	$comp->[0] =~ m/^P \d+ (-?\d+) (-?\d+) (-?\d+) (-?\d+) 0 (\d) /
	  || die "Pin def did not match: $comp->[0]\n";
	my ( $x1, $y1, $x2, $y2, $side ) = ( $1, $2, $3, $4, $5 );
	my ( $x, $y, $orient, $origin );
	if ( $side == 2 ) { # left
	  $x = $x1 > $x2 ? $x1 : $x2;
	  $y = $y1;
	  die "y values don't match: $comp->[0]\n" unless $y1 == $y2;
	  $orient = 0;
	  $origin = 9;
	} elsif ( $side == 3 ) { # right
	  $x = $x1 > $x2 ? $x2 : $x1;
	  $y = $y1;
	  die "y values don't match: $comp->[0]\n" unless $y1 == $y2;
	  $orient = 0;
	  $origin = 3;
	} elsif ( $side == 0 ) { # top
	  $y = $y1 > $y2 ? $y2 : $y1;
	  $x = $x1;
	  die "x values don't match: $comp->[0]\n" unless $x1 == $x2;
	  $orient = 1;
	  $origin = 3;
	} elsif ( $side == 1 ) { # bottom
	  $y = $y1 > $y2 ? $y1 : $y2;
	  $x = $x1;
	  die "x values don't match: $comp->[0]\n" unless $x1 == $x2;
	  $orient = 3;
	  $origin = 3;
	} else {
	  die "Pin side = $side not yet supported\n";
	}
	$sym->add_attribute( $comp, $x, $y, 10, $orient,
                        $origin, 3, "#=$number" )
  }
}

$sym->Write;
