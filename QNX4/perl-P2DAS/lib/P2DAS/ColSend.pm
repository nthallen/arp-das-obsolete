package P2DAS::ColSend;
use strict;
use warnings;
use QNX;
use P2DAS;

our $pid = -1;
our $connected = 0;
our $disconnected = 0;

sub Init {
  my ( $class, $datum, $size ) = @_;
  my $self = bless { name => $datum }, $class;
  my $copies = 0;
  my $name = P2DAS::make_name('dg', 1);
  $pid = QNX::name_locate( 0, $name, 0, $copies );
  if ( $pid == -1 ) {
    if ( ! $disconnected ) {
      warn "Unable to locate '$name'\n";
      $disconnected = 1;
    }
  } else {
    warn "Re-established connection to '$name'\n"
      if $disconnected;
    $connected = 1;
    $disconnected = 0;
    my $initmsg = "\xF8I$datum\0";
    my $rep;
    if ( QNX::Send($pid,$initmsg,$rep,length($initmsg),5) == 0 ) {
      my ( $stat, $nop, $sz, $id ) = unpack( "CCSC", $rep );
      if ( $stat == 0 ) {
	$self->{id} = $id;
	$self->{size} = $size < $sz ? $size : $sz;
	$self->{msgsize} = $self->{size} + 5;
	$self->{hdr} = pack( "a2SC", "\xF8S", $self->{size}, $id );
      }
    } else {
      warn "Error connecting to dg\n";
    }
  }
  return $self;
}

# Return 1 if successful
sub Send {
  my ( $self, $data ) = @_;
  if ( $self->{hdr} ) {
    my $msg = "$self->{hdr}$data";
    my $rep;
    if ( QNX::Send($pid,$msg,$rep,$self->{msgsize},1) == 0 ) {
      my $rv = unpack( "C", $rep );
      return 1 if $rv == 0;
    } else {
      warn "Error sending to dg\n";
      delete $self->{hdr};
    }
  }
  return 0;
}

1;
__END__

=head1 NAME

P2DAS::ColSend - Interface for Sending Data to Collection

=head1 SYNOPSIS

  use P2DAS::ColSend;
  $colsend = P2DAS::ColSend->Init( "DataName" );
  $colsend->Send( $data );

=head1 DESCRIPTION

P2DAS::ColSend provides routines to send data to the Collection
module of the Anderson Group Data Acquisition System.

=head2 EXPORT

None by default.



=head1 SEE ALSO

QNX

=head1 AUTHOR

Norton Allen, E<lt>allen@huarp.harvard.eduE<gt>

=head1 COPYRIGHT AND LICENSE

Copyright (C) 2003 by Norton Allen

This library is free software; you can redistribute it and/or modify
it under the same terms as Perl itself, either Perl version 5.8.2 or,
at your option, any later version of Perl 5 you may have available.


=cut
