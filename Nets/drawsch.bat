@rem = '--*-Perl-*--
@echo off
perl -x -S %0 %1 %2 %3 %4 %5 %6 %7 %8 %9
goto endofperl
@rem ';
#! /usr/local/bin/perl
#line 8

# This is a new version of drawsym to work with the newer version
# of viewlogic via OLE. The old version's default reason for
# existence was to create a symbol for the component itself and
# secondarily for the connectors on the component. At the moment,
# I don't see a need to generate the component symbol (although
# that need could arise again if I try to draw the interconnect
# again...) so this version will draw the connector symbols and
# the schematics for those connectors only.

# Issues:
#   1. Startup up viewlogic once for all, once for each comp
#   or once for each connector. Run benchmarks to see how
#   performance goes...
#   2. How to handle projects: Need to know where the project
#   is going to store files so we can delete the old ones before
#   beginning the new ones.
#    The database should know the project name. Use project
#    manager to find the project directory or create the
#    project.

#__USAGE__
#%C <comp>
#   Generate a symbol and schematic for each connector on <comp>

use FindBin;
use lib "$FindBin::Bin";
use SIGNAL;
use NETSLIB qw(open_nets);
use Win32::OLE qw();
#use Win32::OLE::Variant;

# access registry to locate Nets project dir
my $nets_dir = SIGNAL::siginit('drawsch', 0 );
# { use Win32::Registry;
# 
#   my @keys;
#   sub getsubkey {
#     my ( $key, $subkey ) = @_;
#     my $newkey;
#     ${$key}->Open( $subkey, $newkey ) || return 0;
#     push( @keys, $newkey );
#     $$key = $newkey;
#     return 1;
#   }
#   my $key = $HKEY_CURRENT_USER;
#   foreach my $subkey ( qw( Software HUARP Nets BaseDir ) ) {
#     die "Nets Project Directory is undefined (in Registry)\n"
#       unless getsubkey( \$key, $subkey );
#   }
#   my %vals;
#   $key->GetValues( \%vals ) || die "GetValues failed\n";
#   $nets_dir = $vals{''}->[2];
#   
#   while ( $key = pop(@keys) ) {
#     $key->Close;
#   }
# }
die "Unable to locate nets project directory\n"
  unless $nets_dir && -d $nets_dir && chdir $nets_dir;
# 
# my $logfile = "drawsch";
# 
# open( LOGFILE, ">$logfile.err" ) ||
#   die "Unable to open log file\n";
# 
# $SIG{__WARN__} = sub {
#   print LOGFILE @_;
#   warn @_;
# };
# 
# sub LogMsg {
#   print LOGFILE @_;
#   print STDERR @_;
# }
# 
# $SIG{__DIE__} = sub {
#   warn @_;
#   print STDERR "\nHit Enter to continue:";
#   my $wait = <STDIN>;
#   print STDERR "\n";
#   exit(1);
# };
# 
# END {
#   if ( defined $SIG{__WARN__} ) {
# 	delete $SIG{__WARN__};
# 	delete $SIG{__DIE__};
# 	close LOGFILE;
# 	unlink( "$logfile.bak" );
# 	rename( "$logfile.err", "$logfile.bak" );
# 	open( IFILE, "<$logfile.bak" ) ||
# 	  die "Unable to read $logfile.bak";
# 	open( OFILE, ">$logfile.err" ) ||
# 	  die "Unable to rewrite $logfile.err";
# 	print OFILE
# 	  map $_->[0],
# 		sort { $a->[1] cmp $b->[1] || $a->[0] cmp $b->[0] }
# 		  map { $_ =~ m/:\s+(.*)$/ ? [ $_, $1 ] : [ $_, '' ] } <IFILE>;
# 	close OFILE;
# 	close IFILE;
#   }
# }


SIGNAL::load_signals();

if ( $SIGNAL::global{Draw_Components} ) {
  my ( @comps ) = split( /[,\s]+/, $SIGNAL::global{Draw_Components} );
  push( @ARGV, @comps );
}
die "Drawsch: No Components Requested\n" unless @ARGV > 0;
SIGNAL::LogMsg "DrawSch ", join( " ", @ARGV ), "\n";

# Identify Viewlogic_Project_File from database
my $ProjFile = $SIGNAL::global{Viewlogic_Project_File} ||
  die "Viewlogic_Project_File is not defined\n";
-r $ProjFile || die "Cannot read Project File $ProjFile\n";

{ my $PM = Win32::OLE->new( 'Viewlogic.Document'
            # , sub { $_[0]->{Visible} = 1; }
       ) || die "Unable to start Viewlogic Project Manager\n";
  $PM->Open($ProjFile) || die "Unable to open Viewlogic Project File\n";
}
my $VDraw = Win32::OLE->new(
           'ViewDraw.Application'
           , sub { $_[0]->{Visible} = 1; }
       ) || die "Unable to start ViewDraw\n";
$VDraw->Initialize( $ENV{'WDIR'}, $ENV{'LM_LICENSE_FILE'}) ||
  die "Cannot initialize viewdraw\n";

# SetCurrentProject, GetPrimaryDirectory
#$VDraw->SetCurrentProject($ProjFile);
#{ my $PF = $VDraw->GetCurrentProject;
#  die "Unable to set Current Project: still $PF\n"
#    if $PF ne $ProjFile;
#}

# GetPrimaryDirectory method no longer exists
# my $ProjDir = $VDraw->GetPrimaryDirectory;
my $ProjDir = $ProjFile;
$ProjDir =~ s|[\\/][^\\/]+$||;
die "Unable to identify Viewlogic Primary Directory\n"
  unless $ProjDir && -d $ProjDir;
print "Viewlogic Primary Directory is $ProjDir\n";

my %padloc;

foreach my $comp ( @ARGV ) {
  my $desc;
  my $comptype;
  my $def_sht = 1;
  my $ct;
  if ( $SIGNAL::comp{$comp} ) {
    my $cd = $SIGNAL::comp{$comp};
    $comptype = $cd->{'type'};
    $desc = $cd->{'desc'};
    $ct = $SIGNAL::comptype{$comptype} ||
      die "Component \"$comp\" type \"$comptype\" is undefined\n";
    $desc = $ct->{'desc'} unless $desc;
  } else {
    $comptype = $comp;
    $ct = $SIGNAL::comptype{$comptype} ||
      die "Component \"$comp\" type \"$comptype\" is undefined\n";
    $desc = $ct->{'desc'};
    $comp = $ct->{'comps'}->[0];
    my $cd = $SIGNAL::comp{$comp} || die "No comp def!";
    $desc = $cd->{'desc'} unless $desc;
  }
  print "Comptype: $comptype\n  Desc: $desc\n";
  my $conns = $ct->{'conns'};
  foreach my $conn ( @$conns ) {
    my $conndef = $ct->{'conn'}->{$conn} ||
      die "Connector $comptype:$conn undefined\n";
    my $conndesc = $conndef->{'desc'} || '';
    my $pkg_type = $conndef->{'type'} || '';
    my $sheet = $conndef->{'schematic'} || '';
    unless ( $sheet ) {
      $sheet = "${comptype}_conn.$def_sht";
      $def_sht++;
      warn "No sheet defined for $comptype:$conn: using $sheet\n";
    }
    unless ( open_nets( *IFILE{FILEHANDLE},
                  "sym/$comptype/$conn.list" ) ) {
      warn "No listing found for $comptype:$conn\n";
      next
    }
    my @lines = <IFILE>;
    close IFILE || warn "Error closing sym/$comptype/$conn.list\n";
    chomp @lines;
    @lines = map { [ split /:/ ]; } @lines;
    my $lnpins = @lines;
    my @pins;
    my $npins = SIGNAL::define_pins( $pkg_type, \@pins );
    my $agree = ( $npins == $lnpins );
    warn "sym/$comptype/$conn.list has $lnpins, pkg_type has $npins\n"
      unless $npins == $lnpins;
    $npins = $lnpins if $lnpins < $npins;
    foreach my $i ( 0 .. $npins-1 ) {
      if ( $lines[$i]->[0] ne $pins[$i] ) {
        warn "Pin $i is \"$pins[$i]\" in pkg_type, ",
             "$lines[$i]->[0] in .list\n";
        $agree = 0;
      }
    }
    die "sym/$comptype/$conn.list conflicts with pkg_type $pkg_type\n"
      unless $agree;

    #----------------------------------------------------------------
    # Now we've passed the hurdles: Start drawing
    #----------------------------------------------------------------
    print "  Conn $conn: $sheet, $pkg_type, $conndesc\n";
	$VDraw->{'Visible'} = 0;
	drawsym( $VDraw, $comptype, $conn, \@pins, $pkg_type, $conndesc );
    
    #% generate the schematic
    drawsch( $VDraw, $comptype, $conn, $pkg_type, $sheet, \@lines,
           "$SIGNAL::global{Exp} $desc", "$conn: $conndesc" );
  }
}

# $VDraw->{'Visible'} = 1;
$VDraw->Quit;
exit 0;

# Generate sym/$comp$refdes.1
sub drawsym {
  my ( $VDraw, $comp, $refdes, $pins, $pkg_type, $name ) = @_;
  my $n_pins = @$pins;
  my $height = $n_pins*20+10;
  unlink( "$ProjDir/sym/$comp$refdes.1" );
  $VDraw->Documents->Open("sym\\$comp$refdes.1") ||
    die "Open sym\\$comp$refdes.1 failed\n";
  my $doc = $VDraw->Documents->Item(1);
  # my $view = $VDraw->ActiveView;
  my $block = $VDraw->ActiveView->ActiveBlock;
  $block->{'SheetSize'} = 10; # Z size
  $block->SetZSheetSize( 40, $height );
  $block->{'SymbolType'} = 1; # Module
  { my $box = $block->AddBox( 0, 0, 20, $height ); }
  #----------------------------------------------------------------
  # Set the unattached attributes
  #----------------------------------------------------------------
  { my $attr = $block->AddAttribute(
      "PKG_TYPE=$pkg_type", 10, -20, 3 );
    $attr->{'Origin'} = 6;
  }
  { my $attr = $block->AddAttribute(
      "DEVICE=$pkg_type", 10, -30, 0 );
    $attr->{'Origin'} = 6;
  }
  { my $attr = $block->AddAttribute(
      "REFDES=J?", 10, $height, 3 );
    $attr->{'Origin'} = 6;
  }
  #----------------------------------------------------------------
  # Add the title
  #----------------------------------------------------------------
  { use integer;
    my $text = $block->AddText( $name, 10, $height/2 );
    $text->{'Orientation'} = 1;
    $text->{'Origin'} = 5;
  }

  my $pos = $height;  
  foreach my $pin ( @$pins ) {
    $pos -= 20;
    my $IVdPin = $block->AddPin( 20, $pos );
    $IVdPin->SetLocation( 20, $pos, 40, $pos );
    my $PinAttr = $IVdPin->AddAttribute( "#=$pin", 30, $pos, 3 );
    $PinAttr->{'Origin'} = 3;
    my $label = $pin =~ /^\d/ ? "conpin$pin" : $pin;
    my $IVdLabel = $IVdPin->AddLabel( $label, 30, $pos );
    $IVdLabel->{'Visible'} = 0;
    $IVdLabel->{'Origin'} = 2;
  }
  undef $block;
  $doc->Save;
  undef $doc;
  $VDraw->Documents->close;
  # Can we get rid of that symbol summary?
}

my %notconn = map { ( $_, 1 ) } qw( BLANK SPARE_STAT NC );

# Adds the specified symbol with appropriate annotations to
# the current window (which oughta be a Schematic window).
# Does not open or close the file.
# Does not open, but closes the window
sub drawsch {
  my ( $VDraw, $comptype, $refdes, $pkg_type, $sheet, $pins, $T1, $T2 ) = @_;

  my $n_pins = @$pins;
  my $schfile = "$ProjDir/sch/$sheet";

  unlink($schfile);
  $VDraw->Documents->Open("sch\\$sheet") ||
    die "Open sch\\$sheet failed\n";
  my $doc = $VDraw->Documents->Item(1);
  my $view = $VDraw->ActiveView;
  my $block = $view->ActiveBlock;

  # Determine the sheet size
  my ( $W, $H, $B, $SheetName, $x_base, $y_base, $TX, $TY ) =
      pick_sheet( $n_pins );
  $block->{'SheetSize'} = 10; # Z size
  $block->SetZSheetSize( $W, $H );
  { my $component =
        $block->AddComponent( "$SheetName.1", 0, 0 ) ||
      die "AddComponent( $SheetName.1 ) failed\n";
    undef $component;
  }
  # Add sheet text
  { my $text = $block->AddText( $T1, $W-$B-$TX, $B+$TY );
    $text->{'Origin'} = 6;
    $text->{'Size'} = 20;
  }
  { my $text = $block->AddText( $T2, $W-$B-$TX, $B+$TY-40 );
    $text->{'Origin'} = 6;
    $text->{'Size'} = 15;
  }
  use integer;
  { my $component =
        $block->AddComponent( "$comptype$refdes", $x_base, $y_base ) ||
        #$block->AddComponent( { SymbolName => "$comptype$refdes",
        #                        Locationx => $x_base,
        #                        Locationy => $y_base } ) ||
      die "AddComponent( $comptype$refdes ) failed\n";
    $component->{'Refdes'} = $refdes;
    undef $component;
  }
  undef $block;
  undef $view;
  $doc->Save;
  $doc->Close( 0, "sch\\$sheet");

  # Now we leave viewlogic and head off on our own:
  my %pindef = map { ( $_->[0], $_ ) } @$pins;
  my $sch = load_sch( $schfile );
  my $conn = find_comp( $sch, "$comptype$refdes" ) ||
    die "Component $comptype$refdes not found\n";
  $conn->[0] =~ m/^I\s\d+\s\w+\s\d+\s(\d+)\s(\d+)\s/ ||
    die "Corrupted CompHead: $conn->[0]\n";
  warn "x_base($x_base,$1) y_base($y_base,$2)\n"
    unless $x_base == $1 && $y_base == $2;
  foreach my $line ( @$conn ) {
    if ( $line =~ m/^X\s(\d+)\s(\w+)$/ ) {
      # my ( $pinindex, $pinno ) = ( $1, $2 );
      # Temporary kluge: need to figure out how this works with
      # non-numeric pin numbers
      my ( $pinindex, $pinno ) = ( $1, $1 );
      my $x = $x_base + 40;
      my $y = $y_base + ($n_pins-$pinindex)*20 + 10;
      my $padx = $x + 400;
      my $net = add_net( $sch );
      my $netid = $sch->{count};
      push( @$net, "J $x $y 2\n" );
      $line = "C $netid 1 $pinindex $pinno\n";
      my $signal = '';
      my $link_to = '';
      my $pinline = $pindef{$pinno};
      $signal = $pinline->[1] if $pinline && $pinline->[1];
      $link_to = $pinline->[2] if $pinline && $pinline->[2];
      if ( $link_to =~ /\b((E\d+)|PAD)\b/ ) {
        my $padrefdes = $1;
        if ( $padrefdes eq "PAD" ) {
          $padrefdes = "E?";
          warn "$comptype:$refdes.$pinno: Warning: Unspecified PAD\n";
        } elsif ( defined $padloc{"$comp:$padrefdes"} ) {
		  warn "$comp:$conn:$pinno: $padrefdes also referenced at ",
			  $padloc{"$comp:$padrefdes"}, "\n";
		} else {
		  $padloc{"$comp:$padrefdes"} = "$conn:$pinno";
		}
        my $pad = add_component( $sch, "PIN", 1,
                  $padx+40, $y-20, 4, 1 );
        my $padid = $sch->{count};
        add_attribute( $pad, $padx+30, $y, 15, 4, 8, 3,
            "REFDES=$padrefdes" );
        push( @$net, "J $padx $y 2\n" );
        # Add the connection to the pad
        push( @$pad, "C $netid 2 2 1\n" );
      } else {
        push( @$net, "J $padx $y 1\n" );
        add_text( $sch, $padx+30, $y, 15, 0, 2, $link_to )
          if $link_to;
      }
      push( @$net, "S 2 1\n" );
      my $inv = ( $signal =~ s/^~// ) ? 1 : 0;
      add_label( $net, $x+20, $y, 15, 0, 3, 0, 1, $inv, "\U$signal" )
        if $signal;
    }
  }
  write_sch( $sch, $schfile );
}

sub pick_sheet {
  my ( $npins ) = @_;

  my $CH = $npins * 20 + 10 + 30 + 30;
  my $CW = 40 + 200 + $maxfanout + 80;
  my $found = 0;
  
  # Want an array of options, each listing
  #  W, H, B, RV, SheetName
  #  W, H, B are the Width, Height and Border dimensions
  #  RV is 1 or 0 depending on whether there is a revisions block
  #  SheetName is the SheetName

  # Borders on A and B are really 25, but I don't want to split
  # hairs
  my @sheets = (
    [ 850,  1100, 30, 0, 140, 100, "ASHEET" ],
    [ 1100, 1700, 30, 1, 140, 100, "BSHEETP" ],
    [ 1700, 2200, 50, 1, 175, 150, "CSHEET" ]
  );
  my ( $TBW, $TBH ) = ( 550, 250 );
  my ( $RVW, $RVH ) = ( 425, 130 );

  my @configs = (
    [ $TBH, $RVH, 0 ],
    [ $TBH, 0, $RVW ],
    [ 0, 0, $TBW ]
  );

  my ( $W, $H, $B, $SheetName, $CX, $CY, $TX, $TY );

  SHEET:
  foreach my $sheet (@sheets) {
    my ( $rw, $rh, $RV );
    ( $W, $H, $B, $RV, $TX, $TY, $SheetName ) = @$sheet;
  
    foreach $config ( @configs ) {
      my ( $bot, $top, $rt ) = @$config;
      $top *= $RV;
      my $rw = $W - 2*$B - $rt;
      my $rh = $H - 2*$B - $bot - $top;
      if ( $CW <= $rw && $CH <= $rh ) {
        $CX = $B + 30 + 10*int((5+$rw-$CW)/20);
        $CY = $B + 30 + $bot + 10*int((5+$rh-$CH)/20);
        # print "bot=$bot top=$top rt=$rt\n";
        $found = 1;
        last SHEET;
      }
    }
  }
  die "No sheet large enough!\n" unless $found;

  return ( $W, $H, $B, $SheetName, $CX, $CY, $TX, $TY );
}

# Viewlogic Schematic File Format:
#   File : Header Count Element* 'E'
#   Header :
#     V 50  ( Version? )
#     K \d+ Name
#       Key derived from name.
#       No indication of sheet number.
#     Y 0   ?
#     D 0 0 w h   (Offset?) Width and Height
#     Z 1         (Sheet Size Code. 1=B)
#   Count :
#     i \d+  Index of last item
#   Element : Item | UAnnotate
#   Item : Component | Net
#   UAnnotate : Text | UAttr
#   Component : CompHead Annotate* Conn*
#   CompHead :
#     I itemno name sheet x y orientation scale '
#     (that's the symbol's sheet)
#   Annotate : Attr | Label
#   Attr :
#     A x y size orientation origin visibility text
#   UAttr :
#     U x y size orientation origin visibility text
#   Label :
#     L x y size orientation origin global visibility inverted text
#   Conn : Connected | Unconnected
#   Connected :
#     C netno jointno pinindex pin#
#   Unconnected :
#     X pinindex pin#
#   Net : NetHead Joint* Segment*
#   NetHead :
#     N itemno   (aka netno)
#   Joint :
#     J x y JointCode
#   Segment : SegHead Annotate*
#     S JointNo JointNo
#   JointCode :
#     1 Unconnected
#     2 Connected to pin
#     3 Corner
#     5 T
#   Text :
#     T x y size orientation origin text

sub load_sch {
  my ( $file ) = @_;
  open( IFILE, "<$file" ) ||
    return undef;
  my $sch = { head => [], item => [] };
  while (<IFILE>) {
    if ( m/^i\s(\d+)$/ ) {
      $sch->{count} = $1;
      last;
    }
    push( @{$sch->{head}}, $_ );
  }
  my $item;
  while (<IFILE>) {
    last if m/^E/;
    if ( m/^[ITN]/ ) {
      $item = [];
      push( @{$sch->{item}}, $item );
      push( @$item, $_ );
      undef $item if m/^T/;
    } else {
      die "No item for: $_\n" unless $item;
      push( @$item, $_ );
    }
  }
  close(IFILE) || warn "Error closing $file\n";
  return $sch;
}

sub write_sch {
  my ( $sch, $file ) = @_;
  open( OFILE, ">$file" ) ||
    die "Unable to write file $file\n";
  print OFILE @{$sch->{head}}, "i $sch->{count}\n";
  foreach my $item ( @{$sch->{item}} ) {
    print OFILE @$item;
  }
  print OFILE "E\n";
  close OFILE || warn "Error closing $file\n";
}

sub find_comp {
  my ( $sch, $name ) = @_;
  foreach my $item ( @{$sch->{item}} ) {
    $item->[0] =~ m/^I\s\d+\s$name\s/ && return $item;
  }
  return undef;
}

sub add_component {
  my ( $sch, $name, $sheet, $x, $y, $orient, $scale ) = @_;
  my $id = ++$sch->{count};
  my $comp = [ "I $id $name $sheet $x $y $orient $scale '\n" ];
  push( @{$sch->{item}}, $comp );
  return $comp;
}

sub add_net {
  my ( $sch ) = @_;
  my $id = ++$sch->{count};
  my $net = [ "N $id\n" ];
  push( @{$sch->{item}}, $net );
  return $net;
}

sub add_attribute {
  my ( $comp, $x, $y, $size, $orient, $origin, $vis, $attr ) = @_;
  push( @$comp, "A $x $y $size, $orient $origin $vis $attr\n" );
}

sub add_label {
  my ( $comp, $x, $y, $size, $orient,
      $origin, $glob, $vis, $inv, $label ) = @_;
  push( @$comp,
    "L $x $y $size $orient $origin $glob $vis $inv $label\n" );
}


sub add_text {
  my ( $sch, $x, $y, $size, $orient, $origin, $text ) = @_;
  push( @{$sch->{item}}, [ "T $x $y $size $orient $origin $text\n" ] );
}

__END__
# This is my attempt to use Viewlogic to add nets:

    # my $connpins = $component->GetConnections ||
    #   die "No pins on connector!\n";
    # my $ncpins = $connpins->Count;
    # foreach my $cpno ( 1 .. 1 ) { # $ncpins
    #   use integer;
    #   my $connection = $connpins->Item($cpno);
    #   my $connpin = $connection->GetCompPin;
    #   my $ploc = $connpin->GetLocation;
    #   my $x = $ploc->{'X'}+200;
    #   my $y = $ploc->{'Y'}+0;
    #   my $pno = $connpin->{'Number'};
    #   my $null = Variant(VT_NULL, undef);
    #   my $buswire = Variant(BT_BOOL, 0);
    #   warn "buswire or null failed\n" unless $null && $buswire;

    #   my $padcomp =
    #       $block->AddComponent( "LPIN", $x, $y-20 ) ||
    #     die "AddComponent( PIN ) failed\n";
    #   my $padpin = $padcomp->GetConnections->Item(1) ||
    #     die "Could not get padpin\n";
    #   my $net =
    #     $block->AddNet( 0, 0, 100, 100, $connpin, $padpin, 0 ) ||
    #     $block->AddNet( 0, 0, 100, 100, $connpin, $padpin, $buswire ) ||
        # $block->AddNet( $x, $y, 0, 0, $connpin, $null, $buswire ) ||
        # $block->AddNet( 0, 0, $x, $y, $null, $connpin, $buswire ) ||
        # $block->AddNet( $x, $y, 0, 0, $null, $connpin, $buswire ) ||
        # $block->AddNet( { comppin1 => $connpin, locationx2 => $x,
        #                  locationy2 => $y, busorwire => $buswire } ) ||
    #     warn "AddNet failed for pin $pno ( $x, $y )\n";
      # my $err = $block->LastError; print "Last Error: $err\n";
      # print "    Pin $cpno \"$pno\"\n";
    # }
    # undef $connpins;

:endofperl
