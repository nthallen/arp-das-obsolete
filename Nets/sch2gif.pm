package sch2gif;

use GD;
use lib ".";
use VLSchem;
use UNIVERSAL qw(can);

# foreach my $i ( gdSmallFont, gdMediumBoldFont, gdTinyFont,
# gdLargeFont, gdGiantFont ) {
#   my ( $w, $h ) = ( $i->width, $i->height );
#   print "Font $w x $h\n";
# }

my %unknown_tag;

# Test code:
# VLSchem::AddLibrary( '' => "C:/design/ClONO2/Nets/template" );
# VLSchem::AddLibrary( harvard => "C:/wvoffice/harvlib" );
# VLSchem::AddLibrary( xc4000 => "C:/wvoffice/unified/xc4000" );
# my $rep = VLSchem::Load( 'sch', 'buf2128.1' );
# my $hc = sch2gif::draw( $rep );
# $hc->save( "buf2128.gif" ) || die;

sub gif {
  my $self = shift;
  $self->{im}->gif;
}

sub new {
  my ( $x, $y ) = @_;
  my $hc = {};
  $hc->{width} = $x;
  $hc->{height} = $y;
  $hc->{transforms} = [];
  my $im = $hc->{im} = new GD::Image( $x, $y );
  $hc->{black}  = $im->colorAllocate(  0,  0,  0);       
  $hc->{white}  = $im->colorAllocate(255,255,255);
  $hc->{red}    = $im->colorAllocate(255,  0,  0);      
  $hc->{blue}   = $im->colorAllocate(  0,  0,255);
  $hc->{yellow} = $im->colorAllocate(255,255,  0);
  $hc->{green}  = $im->colorAllocate(  0,255,  0);
  $hc->{cyan}   = $im->colorAllocate(  0,255,255);
  $hc->{sym} = '';
  $im->interlaced('true');
  bless $hc;
  $hc->begin_transform( 0, -$hc->{height}, 6, 1 );
  $hc;
}

# Returns 1 on success.
sub save {
  my ( $hc, $file ) = @_;
  if ( open( OFILE, ">$file" ) ) {
	binmode OFILE;
	print OFILE $hc->gif;
	return 1 if close OFILE;
	warn "Error closing GIF file '$file'\n";
  } else {
	warn "Unable to write GIF to output file '$file'\n";
  }
  return 0;
}

# my $sch = VLSchem::load( 'sch', 'buf2128.1' );
# my $hc = sch2gif::draw( $sch );
# Creates the sch2gif object and draws the specified schematic
sub draw {
  my $rep = shift;
  my $hc = sch2gif::new( @{$rep->{extents}}{'xmax', 'ymax'} );
  $hc->draw_sch( $rep, 0, 0 );
  $hc;
}

# draw_sch() is a lower-level interface, making it possible to
# draw just a portion of a schematic.
sub draw_sch {
  my ( $hc, $rep, $lo ) = @_;
  my $im = $hc->{im};
  $lo = 0 unless defined $lo;
  foreach my $i ( $lo .. @{$rep->{item}} ) {
	my $item = $rep->{item}->[$i];
	my $head = $item->[0];
	if ( $head =~ m/^(\w)\s/ ) {
	  my $tag = "$1";
	  my $cmd = "draw_$1";
	  if ( $hc->can($cmd) ) {
		$hc->$cmd( $item );
	  } else {
		$hc->unknown($head);
	  }
	}
  }
}

sub unknown {
  my ( $hc, $line, $text ) = @_;
  if ( $line =~ m/^(\w)\s/ ) {
	my $tag = $1;
	$text = "Unknown tag '$tag'" unless $text;
	unless ( $undefined{$tag} ) {
	  warn "$text\n";
	  $undefined{$tag} = 1;
	}
  } else {
	warn "Unexpected input: '$line'\n";
  }
}

# html method records various details pertinent to HTML output
sub html_options {
  my ( $hc, $gifpins, $output, $comp, $signal ) = @_;
  $hc->{gifpins} = $gifpins;
  $hc->{htmlout} = $output;
  $hc->{comp} = $comp;
  $hc->{name} = $signal;
}

#     0 - unrotated
#     1 - rotated 90 degrees
#     2 - rotated 180 degrees
#     3 - rotated 270 degrees
#     4 - reflected left/right
#     5 - reflected left/right then rotated 90
#     6 - reflected up/down (left/right+180)
#     7 - reflected left/right + 270
sub transform {
  my ( $hc, $x, $y ) = @_;
  my @mats = ( [ 1, 0, 0, 1 ], [ 0, 1, -1, 0 ],
	[ -1, 0, 0, -1 ], [ 0, -1, 1, 0 ],
	[ -1, 0, 0, 1 ],  [ 0, -1, -1, 0 ],
	[1, 0, 0, -1 ],   [ 0,  1,  1, 0 ] );
  foreach my $trans ( @{$hc->{transforms}} ) {
	my ( $dx, $dy, $orient, $scale ) = @$trans;
	use integer;
	my ( $m11, $m12, $m21, $m22 ) = @{$mats[$orient]};
	$x += $dx;
	$y += $dy;
	my ( $nx, $ny ) = ( $x*$m11 + $y*$m21, $x*$m12 + $y*$m22 );
	( $x, $y ) = ( $nx, $ny );
  }
  ( $x, $y );
}

sub scale {
  my ( $hc, $v ) = @_;
  foreach my $trans ( @{$hc->{transforms}} ) {
	$v = $v * $trans->[3];
  }
  $v;
}

sub begin_transform {
  my ( $hc, $dx, $dy, $orient, $scale ) = @_;
  unshift(@{$hc->{transforms}}, [ $dx, $dy, $orient, $scale ] );
}

sub end_transform {
  my $hc = shift;
  shift @{$hc->{transforms}};
}

sub annotate {
  my ( $hc, $line ) = @_;
  chomp $line;
  my ( $type, $x, $y, $size, $orient, $origin, $text, $invert, $color );
#     A x y size orientation origin avisibility text
#     U x y size orientation origin avisibility text
#     L x y size orientation origin global visibility Inverted text
#     T x y size orientation origin text
  my $hdesc;
  if ( $line =~ m/^[AU]/ ) {
	my $vis;
	( $type, $x, $y, $size, $orient, $origin, $vis, $text ) =
	  split(/ /, $line, 8);
	return unless $vis;
	$text =~ m/^([^=]+)(=(.*))?$/ ||
	  warn "bad syntax for attribute: '$line'\n";
	my ( $name, $value ) = ( $1, $3 );
	if ( $vis > 1 ) {
	  $text = ( $vis == 2 ) ? $name : $value;
	}
	$color = 'yellow';
	$hdesc = "$type-$name:$value";
  } elsif ( $line =~ m/^L/ ) {
	my $vis, $global;
	( $type, $x, $y, $size, $orient, $origin, $global, $vis, $invert, $text ) =
	  split(/ /, $line, 10);
	return unless $vis;
	$color = 'white';
	$hdesc = "L:$hc->{sym}$text";
  } elsif ( $line =~ m/^T/ ) {
	( $type, $x, $y, $size, $orient, $origin, $text ) =
	  split(/ /, $line, 7);
	$color = 'green';
	$hdesc = "T:$text";
  } else {
	$hc->unknown($line);
	return;
  }
  return unless $text;
  # I'll pick the font to use
  my $font = gdSmallFont;
  my ( $w, $h ) = ( $font->width * length($text), $font->height );
  use integer;
  my $xorigin = (--$origin)/3;
  my $yorigin = 2 - ( $origin % 3 );
  my $dx = -($w * $xorigin) / 2;
  my $dy = -($h * $yorigin) / 2;
  $hc->begin_transform( $x, $y, 0, 1 );
  $hc->begin_transform( $dx, $dy, $orient, 1 );
  my ( $x1, $y1 ) = $hc->transform( 0, 0 );
  my ( $x2, $y2 ) = $hc->transform( $w, $h );
  $hc->end_transform;
  $hc->end_transform;
  my $im = $hc->{im};
  ( $x1, $x2 ) = ( $x2, $x1 ) if $x2 < $x1;
  ( $y1, $y2 ) = ( $y2, $y1 ) if $y2 < $y1;
  if ( $orient % 2 ) {
	# origin will be xmax, ymax. will draw up correct x by -$h
	$im->stringUp(gdSmallFont,$x2-$h,$y2,$text,$hc->{$color});
  } else {
	# origin will be xmin, ymax. correct y by +$h
	$im->string(gdSmallFont,$x1,$y2-$h,$text,
				$hc->{$color});
  }
  push( @{$hc->{htmlout}}, "$x1,$y1,$x2,$y2:$hdesc" )
	if $hc->{htmlout};
}

sub draw_T {
  my ( $hc, $item ) = @_;
  $hc->annotate( $item->[0] );
}

sub draw_N {
  my ( $hc, $item ) = @_;
  my $im = $hc->{im};
  my @joint = ( [ 0, 0 ] );
  
  foreach (@$item) {
	if ( m/^J\s(\d+)\s(\d+)\s(\d+)$/ ) {
	  my ( $x, $y, $code ) = ( $1, $2, $3 );
	  ( $x, $y ) = $hc->transform($x, $y);
	  push( @joint, [ $x, $y ] );
	  if ( $code == 1 ) {
		# draw a box
		my $dx = 4;
		$im->filledRectangle($x-$dx, $y-$dx, $x+$dx, $y+$dx,
			  $hc->{red} );
	  } elsif ( $code == 5 ) {
		# draw a dot
		my $r = 10;
		$im->arc( $x, $y, $r, $r, 0, 360, $hc->{red} );
		$im->fill( $x, $y, $hc->{red} );
	  }
	} elsif ( m/^[SB]\s(\d+)\s(\d+)$/ ) {
	  my ( $x1, $y1 ) = @{$joint[$1]};
	  my ( $x2, $y2 ) = @{$joint[$2]};
	  $im->line($x1,$y1,$x2,$y2,$hc->{red});
	  $hc->unknown( $_, "Bus misrendered" ) if /^B/;
	} elsif ( m/^[AL]\s/ ) {
	  $hc->annotate( $_ );
	} elsif ( m/^N/ ) {
	  # nothing
	} else {
	  $hc->unknown( $_ );
	}
  }
}

sub draw_I {
  my ( $hc, $item ) = @_;
  my $im = $hc->{im};
  my %override;
  my %pinnums;
  my ( $type, $itemno, $name, $sheet, $x, $y, $orient, $scale )
	= split(/ /, $item->[0]);
  $hc->begin_transform( $x, $y, 0, 1 );
  $hc->begin_transform( 0, 0, $orient, $scale );
  my $sym = VLSchem::Load( 'sym', "$name.$sheet" );
  my $refdes = '';
  foreach my $line ( @$item ) {
	if ( $line =~ m/^A(\s-?\d+){6}\s(\w+)(=(.*))?$/ ) {
	  $override{$2} = 1;
	  $refdes = $4 if $2 eq 'REFDES';
	} elsif ( $line =~ m/^(C\s\d+\s\d+|X)\s(\d+)\s(\S+)$/ ) {
	  $pinnums{$2} = $3;
	}
  }
  if ( $refdes && $hc->{htmlout} ) {
	my ( $x1, $y1 ) = $hc->transform( @{$sym->{extents}}{'xmin','ymin'} );
	my ( $x2, $y2 ) = $hc->transform( @{$sym->{extents}}{'xmax','ymax'} );
	( $x1, $x2 ) = ( $x2, $x1 ) if $x1 > $x2;
	( $y1, $y2 ) = ( $y2, $y1 ) if $y1 > $y2;
	push( @{$hc->{htmlout}}, "$x1,$y1,$x2,$y2:I:$refdes" );
  }
  #----------------------------------------------------------------
  # override and pinnums change how the symbol appears
  #----------------------------------------------------------------
  $hc->{override} = \%override;
  $hc->{pinnums} = \%pinnums
	unless grep $_->[0] =~ m/^U.*\sPINOFF/, @{$sym->{item}};
  $hc->{sym} = $refdes ? "$refdes/" : "\$I$itemno/";
  $hc->draw_sch( $sym );
  $hc->{sym} = '';
  delete $hc->{override};
  delete $hc->{pinnums};
  $hc->end_transform;
  $hc->end_transform;
  if ( $hc->{gifpins} && $hc->{comp} ) {
	my $gifpins = $hc->{gifpins};
	my $comp = $hc->{comp};
	my $name = $hc->{name};
	foreach my $pin ( values %pinnums ) {
	  $gifpins->{"$comp:$refdes.$pin"} = $name;
	}
  }
  foreach my $line ( @$item ) {
	$hc->annotate( $line ) if $line =~ m/^[AL]/;
  }
}

sub draw_U {
  # if overrides are present, check to see if this one is.
  my ( $hc, $item ) = @_;
  if ( $hc->{override} ) {
	$item->[0] =~ m/^U(\s-?\d+){6}\s(\w+)(=.*)?$/ ||
	  die "U pattern failed: '$item->[0]'\n";
	return if $hc->{override}->{$2};
  }
  $hc->annotate( $item->[0] );
}

sub draw_l {
  my ( $hc, $item ) = @_;
  my $im = $hc->{im};
  my $line = $item->[0];
  my $row = 0;
  chomp $line;
  my ( $type, $npts, $x0, $y0, @pts ) = split(/ /, $line );
  ( $x0, $y0 ) = $hc->transform( $x0, $y0 );
  while ( --$npts > 0 ) {
	@pts = split(' ',$item->[++$row]) unless @pts;
	my $x1 = shift(@pts);
	my $y1 = shift(@pts);
	( $x1, $y1 ) = $hc->transform( $x1, $y1 );
	$im->line( $x0, $y0, $x1, $y1, $hc->{green} );
	( $x0, $y0 ) = ( $x1, $y1 );
  }
}

sub draw_P {
#     P itemno x1 y1 x2 y2 0 Side Inverted
  my ( $hc, $item ) = @_;
  my $im = $hc->{im};
  my $line = $item->[0];
  my $itemno;
  chomp $line;
  if ( $line =~
		m/^P\s(\d+)\s(-?\d+)\s(-?\d+)\s(-?\d+)\s(-?\d+)\s0\s(\d+)\s(\d+)$/ ) {
	my ( $x1, $y1, $x2, $y2, $side, $inv );
	( $itemno, $x1, $y1, $x2, $y2, $side, $inv ) =
		( $1, $2, $3, $4, $5, $6, $7 );
	( $x1, $y1 ) = $hc->transform( $x1, $y1 );
	( $x2, $y2 ) = $hc->transform( $x2, $y2 );
	$im->line( $x1, $y1, $x2, $y2, $hc->{cyan} );
	warn "Inverted pin not supported\n" if $inv;
  } else {
	warn "Pin doesn't match pattern: '$line'\n";
  }
  foreach my $line ( @$item ) {
	$line =~ s/^(A.*)\s0\s#=.*$/$1 3 #=$hc->{pinnums}->{$itemno}/
	  if $hc->{pinnums} && $hc->{pinnums}->{$itemno};
	$hc->annotate( $line ) if $line =~ m/^[AL]/;
  }
}

sub draw_c {
#     c x y radius
# GD::Image::arc(cx,cy,width,height,start,end,color)
  my ( $hc, $item ) = @_;
  my $im = $hc->{im};
  my $line = $item->[0];
  chomp $line;
  if ( $line =~ m/^c\s(-?\d+)\s(-?\d+)\s(\d+)$/ ) {
	my ( $x, $y, $r ) = ( $1, $2, $3 );
	( $x, $y ) = $hc->transform( $x, $y );
	$r = $hc->scale($r) * 2;
	$im->arc( $x, $y, $r, $r, 0, 360, $hc->{green} );
  } else {
	warn "Circle did not match pattern: '$line'\n";
  }
}

sub angle {
  my ( $x0, $y0, $x, $y ) = @_;
  my ( $dx, $dy ) = ( $x-$x0, $y-$y0 );
  if ( $dx == 0 ) {
	return $dy >= 0 ? 180 : 0;
  } elsif ( $dy == 0 ) {
	return $dx >= 0 ? 90 : 270;
  } elsif ( $dx == $dy ) {
	return $dx >= 0 ? 135 : 315;
  } elsif ( $dx == -$dy ) {
	return $dx >= 0 ? 45 : 225;
  } else {
	my $a = (atan2( $dy,$dx ) * 180 / 3.1415926536);
	$a += 360 if $a < 0;
	return $a;
  }
}

sub draw_a {
  my ( $hc, $item ) = @_;
  my $im = $hc->{im};
  my $line = $item->[0];
  chomp $line;
  if ( $line =~ m/^a((\s-?\d+){6})$/ ) {
	my ( $xa, $ya, $xb, $yb, $xc, $yc ) =
	  split(' ', $1);
	( $xa, $ya ) = $hc->transform( $xa, $ya );
	( $xb, $yb ) = $hc->transform( $xb, $yb );
	( $xc, $yc ) = $hc->transform( $xc, $yc );
	my $xab = ($xa + $xb)/2;
	my $yab = ($ya + $yb)/2;
	my $xbc = ($xb + $xc)/2;
	my $ybc = ($yb + $yc)/2;
	my $vxab = $ya - $yb;
	my $vyab = $xb - $xa;
	my $vxbc = $yb - $yc;
	my $vybc = $xc - $xb;
	my $den = ($vxbc*$vyab-$vybc*$vxab);
	if ( $den != 0 ) {
	  my $s = (($ybc-$yab)*$vxab + ($xab-$xbc)*$vyab) / $den;
	  my $cx = $xbc + $s * $vxbc;
	  my $cy = $ybc + $s * $vybc;
	  my $d = 2*sqrt(($xa-$cx)*($xa-$cx) + ($ya-$cy)*($ya-$cy));
	  my $aa = angle( $cx, $cy, $xa, $ya );
	  my $ab = angle( $cx, $cy, $xb, $yb );
	  my $ac = angle( $cx, $cy, $xc, $yc );
	  ( $aa, $ac ) = ( $ac, $aa ) if $aa > $ab || $ab > $ac;
	  $im->arc($cx,$cy,$d,$d,$aa,$ac,$hc->{green});
	} else {
	  warn "Degenerate arc: '$line'\n";
	}
  } else {
	warn "Arc did not match pattern: '$line'\n";
  }
}

sub draw_b {
  my ( $hc, $item ) = @_;
  my $im = $hc->{im};
  my $line = $item->[0];
  chomp $line;
  if ( $line =~ m/^b((\s-?\d+){4})$/ ) {
	my ( $x1, $y1, $x2, $y2 ) = split(' ', $1);
	( $x1, $y1 ) = $hc->transform( $x1, $y1 );
	( $x2, $y2 ) = $hc->transform( $x2, $y2 );
	$im->rectangle($x1,$y1,$x2,$y2,$hc->{green});
  } else {
	warn "Box did not match pattern: '$line'\n";
  }
}

1;

__END__
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
#     A x y size orientation origin visibility text
#   UAttr :
#     U x y size orientation origin visibility text
#   Label :
#     L x y size orientation origin global visibility Inverted text
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
#   SegHead :
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
