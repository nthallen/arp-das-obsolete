package P2DAS;

use 5.006;
use strict;
use warnings;

require Exporter;
use AutoLoader qw(AUTOLOAD);

our @ISA = qw(Exporter);

our %EXPORT_TAGS = ( 'all' => [ qw(
	
) ] );

our @EXPORT_OK = ( @{ $EXPORT_TAGS{'all'} } );

our @EXPORT = qw(
	
);

our $VERSION = '0.01';

1;
__END__
# Below is stub documentation for your module. You'd better edit it!

=head1 NAME

P2DAS - Perl extension for Anderson Group Data Acquisition System

=head1 SYNOPSIS

  use P2DAS;
  $name = make_name( 'dacache', 0 );

=head1 DESCRIPTION

P2DAS provides routines to connect with the Anderson Group
Data Acquisition System.

=head2 EXPORT

None by default.



=head1 SEE ALSO

QNX

If you have a web site set up for your module, mention it here.

=head1 AUTHOR

Norton Allen, E<lt>allen@huarp.harvard.eduE<gt>

=head1 COPYRIGHT AND LICENSE

Copyright (C) 2003 by Norton Allen

This library is free software; you can redistribute it and/or modify
it under the same terms as Perl itself, either Perl version 5.8.2 or,
at your option, any later version of Perl 5 you may have available.


=cut

sub make_name {
  my ( $base, $global ) = @_;
  my $company = 'huarp';
  my $exp = '';
  if ( defined $ENV{Experiment} ) {
	$exp = "/$ENV{Experiment}";
  } else {
	$global = 0;
  }
  my $name = join '',
	$global ? '/' : '',
	$company,
	$exp,
	"/$base";
  return $name;
}
