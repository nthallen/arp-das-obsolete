#! /usr/local/bin/perl -w

package Makefile;
use strict;

sub read_makefile {
  my ( $macros, $filename ) =  @_;
  # print "read_makefile($filename)\n";
  open( MF, "<$filename" ) || die "Unable to open $filename\n";
  while (<MF>) {
	chomp;
	if ( /^([A-Za-z_0-9]+)=(.*)$/ ) {
	  $macros->{$1} = $2;
	} elsif ( /^([A-Za-z_0-9]+)\+=(.*)$/ ) {
	  my $rep = $macros->{$1} || "";
	  $macros->{$1} = "$rep $2";
	} elsif ( /^include\s+([\S]+)$/ ) {
	  my $pos = tell MF;
	  close MF || warn "Error closing $filename for include\n";
	  read_makefile( $macros, expand_macros( $macros, $1 ) );
	  open( MF, "<$filename" ) ||
		die "Error reopening $filename\n";
	  seek( MF, $pos, 0 ) ||
		die "Error seeking in $filename after reopening\n";
	}
  }
  close(MF) || warn "Error closing $filename\n";
}

sub expand_macros {
  my ( $macros, $val ) = @_;
  my ( %expanded );
  while ( $val =~ m/\$\(([A-Za-z_0-9]+)\)/ ) {
	my $var = $1;
	die "Infinite loop on macro $var\n"
	  if $expanded{$var};
	$expanded{$var} = 1;
	my $varval = $macros->{$var} || $ENV{$var} || "";
	$val =~ s/\$\($var\)/$varval/g;
  }
  return $val;
}

sub expand_macro {
  my ( $macros, $macro ) = @_;
  return expand_macros( $macros, "\$($macro)" );
}

sub deglob {
  my ( $string ) = @_;
  my ( @inputs ) = split(/\s+/, $string);
  my @outputs;
  foreach my $file ( @inputs ) {
	if ( $file =~ m/[*?\[\]]/ ) {
	  push( @outputs, glob($file) );
	} else {
	  push( @outputs, $file );
	}
  }
  return @outputs;
}

1;
