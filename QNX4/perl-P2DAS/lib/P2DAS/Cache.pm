package P2DAS::Cache;
use strict;
use warnings;
use QNX;
use P2DAS;

use AutoLoader qw(AUTOLOAD);

our $pid = -1;
our $node = 0;
our $connected = 0;
our $disconnected = 0;

sub init {
  return if $connected;
  $node = shift if @_;
  my $copies;
  my $name = P2DAS::make_name('dacache', 0);
  $pid = QNX::name_locate( $node, $name, 0, $copies );
  if ( $pid == -1 ) {
    if ( ! $disconnected ) {
      warn "Unable to locate '$name'\n";
	  $disconnected = 1;
	}
  } else {
	$connected = 1;
	$disconnected = 0;
  }
}

1;
__END__

=head1 NAME

P2DAS::Cache - Interface to da_cache.

=head1 SYNOPSIS

  use P2DAS::Cache;

=head1 DESCRIPTION

P2DAS::Cache provides routines to connect with the da_cache
module of the Anderson Group Data Acquisition System.

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

sub writev {
  my ( $addr, $size, $data ) = @_;
  init() if $pid == -1;
  if ( $pid != -1 ) {
	my $msg = pack( "A4S2A$size", 'CAwv', $addr, $size, $data );
	QNX::Send($pid,$msg,$reply,length($msg),4);
  }
}
