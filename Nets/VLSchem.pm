package VLSchem;

# VLSchem::AddLibrary( $alias, $directory );
# $sch = VLSchem::Load( $type, $name );
# $sch = $template->Create( $sheet ); # Creates new sch from template
# $sch->Write;
#
# $sch = { filename => $filename, head => [], item => [] };
# also: { dim => [ xmin xmax ymin ymax ], extents => [ xmin xmax ymin ymax ] }
# Schematic object contains the following attributes:
#   filename: The name of the file that was loaded or will be written
#   name:     The key for %Schematic
#   head:     Array of lines that make up the schematics header
#   item:     Array of items, which are arrays of text rows
#             Index is currently not related to item number
#   extents:  { xmin=> xmax=> ymin=> ymax=> }
#   count:    current highest numbered item
#   key:      magic number associated with file basename
#   Decouple  Array of Names that need to be copied for decoupling on
#             this sheet
#   main_region hash ref of xmin, xmax, ymin, ymax for adding main
#             components
#   dec_region hash ref region for adding decouplers
#   margin    Current margin settings
#   x, y      Current insertion point (top left for next insertion)
#   border    The VLSchem that is the template
#   sheet     This sheet number
#  Attributes defined for symbols:
#   parts:    Number of slots (defined for symbols)
#   refdes:   Refdes of currently (relevant only when parts > 1 and freeslots > 0)
#   freeslots:    number of free slots remaining (next slot is parts - slots + 1)
#   decouple: Name of schematic to Copy for decoupling for this part

# Support Functions:
#   VLSchem::OpenProject( ProjectFile );
#   VLSchem::AddLibrary( alias, directory );
#   $sch = VLSchem::ResolveName( type, name, create );
#   $sch = VLSchem::Load( type, name );
#      type = 'sch' or 'sym'
# Methods:
#   $sch->SetMargin( region ); Sets margins for subsequent Copy
#        operations. region is one of the named regions:
#        'extents', 'main_region' or 'dec_region';
#   $sch->Create( sheet, maxsheet );
#       Create creates a new schematic in the same directory as the old one
#       with a different sheet number. It duplicates the contents of the
#       old schematic (template).
#   $sch->Copy( $src, \&transform, @data );
#       Copy the contents of the $src schematic into the $sch at the
#       current position. transform function, if defined, is called
#       for each item after repositioning but before REFDESs have been
#       updated. @data is passed through to the transform function.
#       Before the copying begins, the x,y position may be updated
#       to a location which provides enough room for the $src part.
#       This may even require writing out the sheet and starting
#       a new one. The attribute $sch->{lastxy} stores the x,y
#       position where the part is actually inserted. x is
#       updated after the copy to a position just to the right of
#       the insertion, regardless of space available there. Hence at
#       the end of a Copy, you are guaranteed to still be on the sheet
#       where the part was inserted.
#   $sch->Write;
#		Writes out the schematic to its file. Also writes out any
#		decoupling circuits that have been queued for the sheet
#		to the "decoupling region", dec_region.
#   $sch->find_comp( name );
#       Finds a component in the schematic by symbol name
#   $sch->add_component(...)
#   $sch->add_net(...)
#   $sch->add_text(...)
#   $sch->add_attribute( $comp, ...)
#   $sch->add_label( $comp, ...)
#   $sch->list_pins( [$lo [, $hi]] )
#     Returns an array of pins located starting from itemno $lo
#     (or 0) to $hi or the end of the schematic.
# Local methods:
#   transform_sheet


# Viewlogic Schematic File Format:
#   File : Header Count Element* 'E'
#   Header :
#     V 50  ( Version? I've seen 51 )
#     K \d+ Name
#       Key derived from name.
#       No indication of sheet number.
#     Y 0   0 = sch, 1 = sym
#     D 0 0 w h   (Offset?) Width and Height
#     Z 1         (Sheet Size Code. 1=B, 10=Z)
#   Count :
#     i \d+  Index of last item
#   Element : Item | UAnnotate
#   Item : Component | Net | Pin | Arc | Box | Line | Circle
#   UAnnotate : Text | UAttr
#   Component : CompHead Annotate* Conn*
#   CompHead :
#     I itemno name sheet x y orientation scale '
#     (that's the symbol's sheet)
#   Annotate : Attr | Label
#   Attr :
#     A x y size orientation origin avisibility text
#   UAttr :
#     U x y size orientation origin avisibility text
#   Label :
#     L x y size orientation origin global visibility Inverted text
#   avisibility :
#     0 - invisible
#     1 - visible
#     2 - name
#     3 - value
#   orientation
#     0 - unrotated
#     1 - rotated 90 degrees
#     2 - rotated 180 degrees
#     3 - rotated 270 degrees
#     4 - reflected left/right
#     5 - reflected left/right then rotated 90
#     6 - reflected up/down (left/right+180)
#     7 - reflected left/right + 270
#   Conn : Connected | Unconnected
#   Connected :
#     C netno jointno pinno pin#
#     # netno and pinno are the itemno's associated with the net and pin
#     # The pin's itemno is the itemno within the symbol
#   Unconnected :
#     X pinindex pin#
#   Net : NetHead [Style] Joint* Segment*
#   NetHead :
#     N itemno   (aka netno)
#   Joint : # joints are implicitly numbered from 1
#     J x y JointCode
#   Segment : SegHead Annotate*
#   SegHead :  # That's 'S' for regular net, 'B' for bus
#     [SB] JointNo JointNo
#   JointCode :
#     1 Unconnected (box)
#     2 Connected to pin
#     3 Corner
#     5 T or more (dot)
#   Text :
#     T x y size orientation origin text
#   Pin :
#     P itemno x1 y1 x2 y2 0 Side Inverted
#   Side :
#     0 Top
#     1 Bottom
#     2 Left
#     3 Right
#   Inverted :
#     0 Not Inverted
#     1 Inverted
#   Style :
#     Q \d+ \d+ \d+  For specifying color, font, linestyle
#   Line :
#     l npts x1 y1 x2 y2 ...
#   Arc :
#     a x1 y1 xm ym x2 y2
#   Circle :
#     c x y radius
#   Box :
#     b x1 y1 x2 y2
my %libs;
my @liborder;
my %Schematic;
my %Refdes;    # latest refdes for each type

sub AddLibrary {
  my ( $alias, $directory ) = @_;
  $alias = lc $alias;
  push( @liborder, $alias );
  $libs{$alias} = $directory;
}

sub OpenProject {
  use Win32::OLE qw();
  my $ProjFile = shift;
  -r $ProjFile || die "Cannot read Project File $ProjFile\n";
  my $PM = Win32::OLE->new( 'Viewlogic.Document'
       ) || die "Unable to start Viewlogic Project Manager\n";
  $PM->Open($ProjFile) || die "Unable to open Viewlogic Project File\n";
  my $ProjDir = $PM->{PrimaryDirectory} ||
	die "Unable to get PrimaryDirectory\n";
  # LogMsg "Primary Directory is $ProjDir\n";
  VLSchem::AddLibrary( '' => $ProjDir );
  my $nlibs = $PM->GetNumLibraries;
  for ( my $i = 0; $i < $nlibs; $i++ ) {
	my $alias = $PM->GetLibraryAlias($i);
	my $path =  $PM->GetLibraryPath($i);
	my $type =  $PM->GetLibraryType($i);
	if ( $type == 2 ) {
	  warn "Library $alias:$path is a Megafile\n";
	} else {
	  VLSchem::AddLibrary( $alias => $path );
	  # LogMsg "Adding library $alias=$path\n";
	}
  }
}

# made a minor mod to allow names that don't end in \.\d+ but rather
# \.\w+ to allow ".template". Actually, that was a bad idea...
sub ResolveName {
  my ( $type, $name, $create ) = @_;
  $type =~ m/^sch|sym$/ || die "Invalid type in ResolveName\n";
  $name =~ m/^((\w+):)?([-+\w]+\.\w+)$/ ||
	die "Could not parse name in ResolveName: '$name'\n";
  $name = lc $name;
  my $lib = $2 ? lc($2) : ( $create ? '' : undef );
  my $sym = lc $3;
  unless ( $lib || $create ) {
	foreach my $libr ( @liborder ) {
	  if ( -f "$libs{$libr}/$type/$sym" ) {
		$lib = $libr;
		last;
	  }
	}
	die "File '$type/$sym' not found\n" unless defined $lib;
  }
  warn "Library '$lib' not defined\n" unless $libs{$lib};
  $name = "$lib:$sym";
  unless ( defined $Schematic{"$type/$name"} ) {
	my $file = "$libs{$lib}/$type/$sym";
	my $sch = $Schematic{"$type/$name"} = { filename => "$file", name => "$type/$name" };
	bless $sch;
  }
  $Schematic{"$type/$name"};
}

sub Load {
  my ( $type, $name ) = @_;
  my $sch = ResolveName( $type, $name );
  return $sch if defined $sch->{head};
  my $file = $sch->{filename};
  unless ( open( IFILE, "<$file" )) {
	warn "Unable to load $type $name\n";
    return undef;
  }
  $sch->{head} = [];
  $sch->{item} = [];
  while (<IFILE>) {
    if ( m/^i\s(\d+)$/ ) {
      $sch->{count} = $1;
      last;
    }
	if ( m/^K\s(\d+\s\w+)$/ ) {
	  $sch->{key} = $1;
	} elsif ( m/^D\s(\d+)\s(\d+)\s(\d+)\s(\d+)/ ) {
	  $sch->{extents} = { xmin=>$1, ymin=>$2, xmax=>$3, ymax=>$4 };
	}
    push( @{$sch->{head}}, $_ );
  }
  my $item;
  while (<IFILE>) {
    last if m/^E/;
    if ( m/^[ITNUPlabc]/ ) {
      $item = [];
      push( @{$sch->{item}}, $item );
      push( @$item, $_ );
    } else {
      die "No item for: $_\n" unless $item;
      push( @$item, $_ );
    }
  }
  close(IFILE) || warn "Error closing $file\n";
  if ( $type eq 'sym' ) {
	my @parts = grep /^U.*\sPARTS=\d+$/, map @$_, @{$sch->{item}};
	if ( @parts == 1 && $parts[0] =~ /^U.*\sPARTS=(\d+)$/ ) {
	  $sch->{parts} = $1;
	} else {
	  $sch->{parts} = 1;
	}
	# warn "Symbol $sch->{name}: PARTS=$sch->{parts}\n";
  }
  foreach my $comp ( map $_->[0], grep $_->[0] =~ m/^I/,  @{$sch->{item}} ) {
	$comp =~ m/^I\s\d+\s((\w+:)?[-+\w]+)\s(\d+)\s/ ||
	  die "Could not parse component definition: '$comp'\n";
	my $sym = Load( "sym", "$1.$3" );
  }
  return $sch;
}

sub SetMargin {
  my ( $sch, $region ) = @_;
  $sch->{margin} = $sch->{$region};
  $sch->{x} = $sch->{margin}->{xmin};
  $sch->{y} = $sch->{margin}->{ymax};
}

sub transform_sheet {
  my ( $dest, $item ) = @_;
  $item->[0] =~ s/^(U.*\s\@SHEET=)\d+$/$1$dest->{sheet}/;
  1;
}

# Create creates a new schematic in the same directory as the old one
# with a different sheet number. It duplicates the contents of the
# old schematic (template)
sub Create {
  my ( $template, $sheet, $maxsheet ) = @_;
  my $name = $template->{name};
  $name =~ m!^(sch|sym)/([-+\w:]+)\.\w+$! ||
	die "Did not understand old name '$name'\n";
  my $type = $1;
  $template->{key} =~ m/^\d+\s(\w+)$/ || die "Missing key\n";
  my $sch = ResolveName( $type, "$1.$sheet", 1 );
  die "$sch->{name} already defined\n" if defined $sch->{head};
  $sch->{head} = [ @{$template->{head}} ];
  $sch->{item} = [];
  $sch->{count} = 0;
  $sch->{border} = $template;
  $sch->{sheet} = $sheet;
  $sch->{key} = $template->{key};

  $sch->{extents} = $template->{extents};
  $sch->{main_region} = $template->{main_region}
	if $template->{main_region};
  $sch->{dec_region} = $template->{dec_region}
	if $template->{dec_region};

  $sch->SetMargin('extents');
  $sch->Copy( $template, \&transform_sheet );
  $sch->SetMargin('main_region');
  $sch->{maxsheet} = $maxsheet if $maxsheet;
  return $sch;
}

# Copy the contents of the $src schematic into the $dest at the specified location
sub Copy {
  my ( $dest, $src, $transform, @data ) = @_;
  $dest->{Decouple} = [] unless defined $dest->{Decouple};
  my ( $dx, $dy ) = @{$src->{extents}}{'xmax','ymax'};
  if ( $dest->{x} + $dx > $dest->{margin}->{xmax} ) {
	$dest->{x} = $dest->{margin}->{xmin};
	$dest->{y} -= $dy;
  }
  my ( $x, $y ) = @$dest{'x', 'y'};
  $y -= $dy;
  if ( $y < $dest->{margin}->{ymin} ) {
	$dest->Write();
	if ( $dest->{maxsheet} && $dest->{maxsheet} <= $dest->{sheet} ) {
	  die "Schematic sheet allocation exhausted\n";
	}
	$_[0] = $dest = $dest->{border}->Create( $dest->{sheet} + 1,
							$dest->{maxsheet} );
	$dest->Copy( $src, $transform, @data ) ||
	  die "Ran out of room at top!\n";
	return;
  }
  $dest->{lastxy} = [ $x, $y ];

  my %itemmap;
  foreach my $item ( @{$src->{item}} ) {
	my $newitem = [];
	#----------------------------------------------------------------
	# Handle basic transformations of item number and position
	#----------------------------------------------------------------
	foreach my $oldline ( @$item ) {
	  my $line = $oldline;
	  if ( $line =~ m/^[INC]\s(\d+)/ ) {
		my $olditemno = $1;
		my $newitemno = defined($itemmap{$olditemno}) ? $itemmap{$olditemno} :
					  ++$dest->{count};
		$itemmap{$olditemno} = $newitemno;
		$line =~ s/^([INC]\s)(\d+)/$1$newitemno/;
	  }
	  if ( $line =~ m/^([AULJTc]|I\s\d+\s(\w+:)?[-+\w]+\s\d+)\s(\d+)\s(\d+)\s/ ) {
		$line = "$1 " . ($3 + $x) . " " . ($4 + $y) . " $'";
	  } elsif ( $line =~ m/^([ab]|l\s\d+)(.*)$/ ) {
		my $newline = $1;
		my $rem = $2;
		while ( $rem =~ s/^\s(\d+)\s(\d+)// ) {
		  $newline .= " " . ($1+$x) . " " . ($2+$y);
		}
		$line = "$newline\n";
	  }
	  push( @$newitem, $line );
	}
	#----------------------------------------------------------------
	# Provide custom transformation for attributes, labels
	# (Do this BEFORE refdes remapping)
	#----------------------------------------------------------------
	if ( (! defined $transform) ||
		  &$transform($dest, $newitem, @data ) ) {
	  push( @{$dest->{item}}, $newitem );
	
	  #----------------------------------------------------------------
	  # Now address Item insertion, REFDES, decoupling, etc.
	  #----------------------------------------------------------------
	  if ( $newitem->[0] =~ m/I\s\d+\s((\w+:)?[-+\w]+)\s(\d+)\s/ ) {
		my $sym = ResolveName( 'sym', "$1.$3" );
		my ( $refdesline ) = grep m/^A\s.*\sREFDES=/, @$newitem;
		if ( defined $refdesline ) {
		  my $newrefdes;
		  if ( $sym->{parts} > 1 ) {
			if ( $sym->{freeslots} ) {
			  $newrefdes = $sym->{refdes};
			} else {
			  # allocate a new part
			  $sym->{freeslots} = $sym->{parts};
			}
			my $slot = $sym->{parts} - $sym->{freeslots}--;
			# remap pins for slot:
			# Look through $sym for "P itemno" items, and for each extract the
			# A .* #=\d+(,\d+)*$ attribute
			# create a hash $sym->{pins}->{pinitemno}->[slot] = pinno
			unless ( defined $sym->{pins} ) {
			  my $def = $sym->{pins} = {};
			  foreach my $pin ( grep $_->[0] =~ m/^P/, @{$sym->{item}} ) {
				$pin->[0] =~ m/^P\s(\d+)\s/ || die;
				my $pinitemno = $1;
				my ( $pinnos ) = map { m/^A\s.*\s#=(\d+(,\d+)*)$/ ? $1 : () } @$pin;
				die unless $pinnos;
				$def->{$pinitemno} = [ split(',', $pinnos) ];
			  }
			}
			# now look through newitem looking for ^[CX] records and reassigning
			# the pinno to that appropriate for the slot
			map s/^([CX].*\s(\d+)\s)\d+$/$1$sym->{pins}->{$2}->[$slot]/,
			  @$newitem;
		  }
		  unless ( $newrefdes ) {
			# pick an appropriate REFDES
			$refdesline =~ m/\sREFDES=(\D+)([?]|\d+)$/ || die "refdesline: '$refdesline'\n";
			my $refroot = $1;
			$Refdes{$refroot}++;
			$newrefdes = "$refroot$Refdes{$refroot}";
			$sym->{refdes} = $newrefdes;
			# queue a decoupling schematic if required
			if ( $sym->{decouple} ) {
			  push( @{$dest->{Decouple}}, $sym->{decouple} );
			}
		  }
		  map s/(REFDES=).*$/$1$newrefdes/, @$newitem;
		}
	  }
	}
  }
  $dest->{x} += $dx;
}


sub Write {
  my ( $sch ) = @_;

  # Throw in any outstanding decouplers:
  $sch->SetMargin('dec_region');
  while ( my $dec = shift @{$sch->{Decouple}} ) {
	my $dsch = VLSchem::Load( 'sch', $dec );
	$sch->Copy( $dsch ) || die "Ran out of room decoupling!\n";
  }
  
  open( OFILE, ">$sch->{filename}" ) ||
    die "Unable to write file $sch->{filename}\n";
  print OFILE @{$sch->{head}}, "i $sch->{count}\n";
  foreach my $item ( @{$sch->{item}} ) {
    print OFILE @$item;
  }
  print OFILE "E\n";
  close OFILE || warn "Error closing $sch->{filename}\n";
  warn "$sch->{filename} written\n";
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

sub add_text {
  my ( $sch, $x, $y, $size, $orient, $origin, $text ) = @_;
  push( @{$sch->{item}}, [ "T $x $y $size $orient $origin $text\n" ] );
}

use UNIVERSAL qw(isa);
sub add_attribute {
  my $comp = shift;
  $comp = shift if isa($comp,'VLSchem');
  my ( $x, $y, $size, $orient, $origin, $vis, $attr ) = @_;
  push( @$comp, "A $x $y $size, $orient $origin $vis $attr\n" );
}

sub add_label {
  my $comp = shift;
  $comp = shift if isa($comp,'VLSchem');
  my ( $x, $y, $size, $orient,
      $origin, $glob, $vis, $inv, $label ) = @_;
  push( @$comp,
    "L $x $y $size $orient $origin $glob $vis $inv $label\n" );
}


# my $refdes = $sch->get_refdes( $comp );
sub get_refdes {
  my $comp = shift;
  $comp = shift if isa($comp,'VLSchem');
  my @refs = map { /^A.*REFDES=(.*)$/ ? $1 : (); } @$comp;
  warn "Component contains multiple REFDESs\n" if @refs > 1;
  shift @refs;
}

# my $label = $sch->get_label( $comp );
sub get_label {
  my $comp = shift;
  $comp = shift if isa($comp,'VLSchem');
  my @labels = map { /^L(\s[-+]?\d+){8}\s(.*)$/ ? $2 : (); } @$comp;
  warn "Component contains multiple labels\n" if @labels > 1;
  shift @labels;
}

# @pins = $sch->list_pins( [ $lo [ , $hi ] ] );
sub list_pins {
  my ( $sch, $lo, $hi ) = @_;
  my @schpins;
  $lo = 0 unless $lo;
  $hi = @{$sch->{item}} unless $hi;
  foreach my $i ( $lo .. $hi ) {
	my $item = $sch->{item}->[$i];
	my $head = $item->[0];
	if ( $head =~ m/^I\s/ ) {
	  my ( $refdes ) = map { /^A.*REFDES=(\w+)$/ ? $1 : () } @$item;
	  if ( $refdes ) {
		map { if ( /^(C\s\d+\s\d+|X)\s\d+\s(\S+)$/ ) {
				push( @schpins, "$refdes.$2" );
			} } @$item;
	  }
	}
  }
  @schpins;
}

1;

__END__
Plan for buffer drawing:
  Inputs:
	Signal Order: can be derived from {
	  cmdtm
	  list of connector order
	  connector listings
	}
	Signal database (to decide whether to add pads)
	Output sheet number(s)
	Output REFDES ranges?
	Annotation Template    <oproj>.template
	Replication Template   <duptype>
	Symbol Definitions
	Decoupling Schematics  <use symbol attributes? or my own table>
	
  Phase I:
	Inputs:
	  List of Signals
	  Output sheet number(s)
	  Annotation Template: Contains sheet, border, etc.
	  Replication Template(s): Contains circuit for single signal
	  Decoupling Template(s):

	Read Replication Template schematic {
	  Read symbols for components
	  Identify which components have multiple slots
	  Identify which components require decoupling {
		Read decoupling template schematic
		Read symbols to identify whether multiple slots are required
	  }
	}
	Identify maximum,minimum coordinates of template
	while (signals remain) {
	  create a new sheet
	  read Annotation template
	  while (signals remain ) {
		last unless there's room for another template
		add replication template to schematic {
		  for each component {
			if there is an open slot, use it
			otherwise allocate a new component, checking to see if
			decoupling is required.
		  }
		  signal names
		  Component values, pkg_types, refdes
		  x,y position
		}
	  }
	  write out sheet
	}

  Read cmdtm to identify signals which need buffering {
	These are analog inputs (type AI) which have buffers located on the
	target board. Signal hereby known as NAME
	Possibly identify alternate grounds, etc.
  }
  Refer to Signal database to see if signal and derivatives are defined
  on the board
  Sort based on connector placement for inputs NAME_HI
  
  Read ground translation list
  Read pad identification list

  Read template file
  Identify all derivative signal names
  Identify dimensions of template and number of signals n
  Identify items
  Identify REFDES patterns m/^(.*\D)(\d+)$/ (increment in tandem?)
  
  Read Layout Template {
	Add those components to new schematic with suitable mods
	(sheet number, etc.)
  }
  For each n signals, invoke template {
	itemno's need to have base value added
	y and possible x coordinates need to adjusted
	Attributes VALUE, PKG_TYPE, REFDES need to be handled
	Labels NAME(\S*) need to be transformed {
	  may need to identify pullup voltage name, etc.
	}
	Add pads for unconnected signals {
	  Find net with appropriate label
	  Locate Joint which is unconnected
	  Locate Segment which references that joint
	  Determine the direction from that joint
	  Add a new item (the pad) at the unconnected joint
	  Change the joint's code from unconnected (1) to connected (2)
	  By convention, the pad's number should be identified somewhere, so they
	  don't change.
	}
  }
  