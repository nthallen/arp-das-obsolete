BEGIN { $| = 1; print "1..7\n"; }
END {print "not ok 1\n" unless $loaded;}
use QNX;
$loaded = 1;
print "ok 1\n";

######################### End of black magic.

# Attach a proxy to the parent and trigger it with the child process.
# Check to make sure that the pid received from is correct and that 
# the message is correct.  Then detach the proxy.

$msg = 0;
$proxy = QNX::proxy_attach( 0, $msg, 1, -1 );
print $proxy == -1 ? "not ok 2" : "ok 2", "\n";

if ( fork ) {
	$pid = QNX::Receive( 0, $rmsg, 1 );
	print $pid == -1 ? "not ok 4" : "ok 4", "\n";
	print $pid == $proxy ? "ok 5" : "not ok 5", "\n";
	print $rmsg == $msg ? "ok 6" : "not ok 6", "\n";
	$r = QNX::proxy_detach($proxy);
	print $r == 0 ? "ok 7" : "not ok 7", "\n";
} else {
	print QNX::Trigger($proxy) == -1 ? "not ok 3" : "ok 3", "\n";
}
