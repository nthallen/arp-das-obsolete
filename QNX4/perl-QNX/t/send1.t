BEGIN { $| = 1; print "1..5\n"; }
END {print "not ok 1\n" unless $loaded;}
use QNX;
$loaded = 1;
print "ok 1\n";

######################### End of black magic.

#  Standard Send/Receive sequence.  Check to make sure that the correct
#  message was sent and received and replied with.

$smsg = "Hello there";
$size = 11;

if ( ($pid=fork) == 0 ) {
	$rpid = QNX::Receive($pid, $rvmsg, $size);
	print $rpid == -1 ? "not ok 2" : "ok 2", "\n";
	$r = QNX::Reply($rpid, $rvmsg, $size);
	print $r == 0 ? "ok 3" : "not ok 3", "\n";
} else {
	$r = QNX::Send($pid, $smsg, $rmsg, $size, $size);
	print $r == 0 ? "ok 4" : "not ok 4", "\n";
	print $rmsg eq $smsg ? "ok 5" : "not ok 5", "\n";
}
