BEGIN { $| = 1; print "1..3\n"; }
END {print "not ok 1\n" unless $loaded;}
use QNX;
$loaded = 1;
print "ok 1\n";

$my_node = QNX::getnid();
print $my_node > 0 ? "ok" : "not ok", " 2\n";
if ( defined( $alive = QNX::net_alive() )) {
	print $alive->[$my_node] ? "ok" : "not ok", " 3\n";
} else {
	print "ok 3 # Skip Running Standalone - no Net\n";
}

