package QNX;

use strict;
use vars qw($VERSION @ISA @EXPORT @EXPORT_OK);

require Exporter;
require DynaLoader;
require AutoLoader;

@ISA = qw(Exporter DynaLoader);
@EXPORT_OK = qw();
$VERSION = '0.05';

bootstrap QNX $VERSION;

# Preloaded methods go here.

# Autoload methods go after =cut, and are processed by the autosplit program.

1;
__END__

=head1 NAME

QNX - Perl extension for Watcom QNX C library

=head1 SYNOPSIS

	QNX::Send($pid, $send_msg, $receive_msg, $size_send, $size_receive)
	QNX::Receive($pid, $receive_msg, $size_receive)
	QNX::Reply($pid, $reply_msg, $size_reply)
	QNX::name_attach($nid, $name)
	QNX::name_detach($nid, $name_id)
	QNX::name_locate($nid, $name, $size, $copies)
	QNX::proxy_attach($pid, $data, $size, $priority)
	QNX::proxy_detach($pid)
	QNX::proxy_rem_attach($nid, $proxy_pid)
	QNX::proxy_rem_detach($nid, $vproxy_pid)
	QNX::Trigger($pid)
	QNX::vc_attach($nid, $pid, $length, $flags)
	QNX::vc_detach($vid)
	QNX::psinfo1($proc_pid, $pid)
	QNX::net_alive()
	QNX::getnid()

	QNX::errno()

=head1 DESCRIPTION


These functions provide access to some QNX-specific functions.

Unless otherwise noted, they have the same usage as the corresponding
C functions.

=head1 FUNCTIONS

=over 8

=item name_locate

	$pid = QNX::name_locate($nid, $name, $size, $copies)

The '$copies' variable passed into the function is the actual variable
in which the number of copies will be stored.

=item psinfo1

psinfo1 is a version of the QNX C library function qnx_psinfo() that
returns only a subset of what qnx_psinfo() can give.

Usage:
   $hashref = QNX::psinfo1( $proc_pid, $pid );

psinfo1 returns a reference to a hash describing the process '$pid' or
the next highest process number if '$pid' doesn't exist.

This behaviour is the same as qnx_psinfo() and allows the caller to scan
the complete task list sequentally (see following example).

Because of this, the caller should check the value returned by
$hashref->{'pid'} if it was trying to query a specific process.

The value of '$proc_pid' should be either the value $QNX::PROC_PID 
or the pid of a VC that connect to an instance of Proc on some remote
node.  For more details on this see the description of qnx_psinfo().

The hash will define the following keys (and values) for ALL process types:

     pid      - the pid of the process found.
     type     - the type of process as one of the following strings:
                'process', 'proxy' or 'vc'.
     priority - the process priority.

The hash will define the following keys for processes:

     name     - the name of the process.
     state    - the current execution state of the process as one of the
                following strings: DEAD, READY, SEND, RECEIVE, REPLY,
                    HELD, SIGNAL, WAIT or SEM.
     father   - the process-id of the father.
     son      - the process-id of the son.
     brother  - the process-id of the brother.

The hash will define the following keys for proxies:

     count    - the number of counts against the proxy

The hash will define the following keys for VC (virtual circuits):

     local_pid   - the process that owns this VC. 
     remote_pid  - the process that's connected to this VC on the remote node. 
     remote_vid  - the Virtual Circuit Id corresponding to this VC on the
                   remote node. 
     remote_nid  - which node the VC points to. 

Example of code to scan the whole task list on the current node:

	my ($psinfo, $proc_pid, $pid ) = (0, $QNX::PROC_PID, 1);
	for (;;) {
		$psinfo = QNX::psinfo1( $proc_pid, $pid );
		last if (!defined($psinfo));
		if ($$psinfo{'type'} eq "process") {
			print "$$psinfo{'pid'} $$psinfo{'name'} " .
				"pri = $$psinfo{'priority'} $$psinfo{'state'} " .
				"father = $$psinfo{'father'} son = $$psinfo{'son'} " .
				"brother = $$psinfo{'brother'}\n";
		}
		elsif ($$psinfo{'type'} eq "proxy") {
			print "$$psinfo{'pid'} proxy count = $$psinfo{'count'} " .
				"pri=$$psinfo{'priority'}\n";
		}
		elsif ($$psinfo{'type'} eq "vc") {
			print "$$psinfo{'pid'} " .
				"pri = $$psinfo{'priority'} " .
				"local_pid = $$psinfo{'local_pid'} " .
				"remote_pid = $$psinfo{'remote_pid'} " .
				"remote_nid = $$psinfo{'remote_nid'} " .
				"remote_vid = $$psinfo{'remote_vid'}\n";
		else {
			print "ERROR:Invalid process type: '$$psinfo{'type'}'\n";
		}
		$pid = $$psinfo{'pid'}+1;
	}

=item net_alive

    $arrayref = QNX::net_alive();

This function is similar to the QNX C function qnx_net_alive().
It returns a reference to an array that contains the status of each
node (computer) in the network as either 0 or 1 for down/up respectively.

Contrary to qnx_net_alive(), net_alive() returns 'undef' if 'Net' is
not running on the current machine (e.g. Net is not registered locally as
'qnx/net').  This behaviour is similar to the QNX 'alive' command.

The first element of the array returned by net_alive() is never used.
The node statuses start at index 1 up to the number of nodes (e.g. the
number of QNX licenses on your machine).

=item errno

QNX::errno() returns a stored value of the errno variable. When
any of these functions returns an error, the value of errno is
saved and can be retrieved by calling QNX::errno().

=head1 AUTHOR

This module was originally created by

  Jocelyn Rodgers <jrodgers@fas.harvard.edu>
 
It is currently maintained by

  Norton Allen <allen@huarp.harvard.edu>

Significant contributions have been provided by

  Marc Lupien <marclupien@hotmail.com> Montreal Subway Transit.

=head1 SEE ALSO

perl(1).

=cut
