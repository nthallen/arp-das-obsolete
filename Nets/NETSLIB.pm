package NETSLIB;

use Exporter ();
@ISA = qw(Exporter);
@EXPORT_OK = qw(open_nets @NETSLIB find_nets mkdirp);

BEGIN {
  @NETSLIB = ( "net" );
  if ( $INC{'NETSLIB.pm'} ) {
	my $dir = $INC{'NETSLIB.pm'};
	$dir =~ s|/NETSLIB.pm$|| && push( @NETSLIB, $dir );
  }
}

# Search for the specified file in the NETSLIB path.
# Returns the path if found or ""
sub find_nets {
  my ( $file ) = @_;
  return $file if -r $file;
  return '' if $file =~ m|^/|;
  foreach my $dir ( @NETSLIB ) {
	-r "$dir/$file" && return "$dir/$file";
  }
  return "";
}

sub open_nets {
  my ( $fhref, $file ) = @_;
  my $path = find_nets( $file );
  if ( $path ne "" ) {
	if ( open( $fhref, "<$path" ) ) {
	  return 1;
	} else {
	  warn "ERROR: open($path) failed\n";
	}
  }
  return 0;
}

sub mkdirp {
  my ( $path ) = @_;
  my ( @nodes ) = split(/\//, $path );
  my $dir;
  while ( @nodes > 0 ) {
	$dir .= shift(@nodes);
	-d $dir || mkdir( $dir, 0775 ) ||
	  die "Unable to create directory $dir\n";
	$dir .= '/';
  }
}

1;
