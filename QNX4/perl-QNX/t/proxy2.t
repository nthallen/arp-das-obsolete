BEGIN { $| = 1; print "1..7\n"; }
END {print "not ok 1\n" unless $loaded;}
use QNX;
$loaded = 1;
print "ok 1\n";

######################### End of black magic.

# Try to trigger a proxy on the parent that has already been detached.
# Then communicate with parent through send/receive to make sure that
# the proxy trigger was unsuccessful because the proxy was detached,
# not because the parent had trouble receiving.

$msg = 26;
$size = 2;

$proxy = QNX::proxy_attach( 0, 0, 0, -1 );
print $proxy == -1 ? "not ok 2" : "ok 2", "\n";
print QNX::proxy_detach($proxy) == 0 ? "ok 3" : "not ok 3", "\n";

if ( $child=fork ) {
	$pid = QNX::Receive( 0, $rvmsg, $size );
	print $pid==$child ? "ok 5" : "not ok 5", "\n";
	QNX::Reply( $pid, $rvmsg, $size );
} else {
	print QNX::Trigger($proxy) == -1 ? "ok 4" : "not ok 4", "\n";
	$r = QNX::Send(getppid, $msg, $rmsg, $size, $size);
	print $r == 0 ? "ok 6" : "not ok 6", "\n";
	print $rmsg == $msg ? "ok 7" : "not ok 7", "\n";
}
