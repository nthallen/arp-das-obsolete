BEGIN { $| = 1; print "1..3\n"; }
END {print "not ok 1\n" unless $loaded;}
use QNX;
$loaded = 1;
print "ok 1\n";

######################### End of black magic.

# Send a message to a bogus pid.  Check to make sure send fails and 
# that errno is set to the correct value.

$msg = "Hello there";
$size = 20;
$errno_val = 3;

$r = QNX::Send(8902387, $msg, $msg, $size, $size);
print $r == -1 ? "ok 2" : "not ok 2", "\n";
print QNX::errno == $errno_val ? "ok 3" : "not ok 3", "\n";

