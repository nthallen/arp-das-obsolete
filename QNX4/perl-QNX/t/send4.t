BEGIN { $| = 1; print "1..5\n"; }
END {print "not ok 1\n" unless $loaded;}
use QNX;
$loaded = 1;
print "ok 1\n";

######################### End of black magic.

# Truncates the message when it is initially received and checks to see
# if the correct truncated message is received by the Send function.

$smsg = "Hello there";
$trunc_msg = "Hello";
$ssize = 11;
$rsize = 5;

if( ($pid=fork) == 0 )
{
	$rpid = QNX::Receive($pid, $rvmsg, $rsize);
	print $rpid == -1 ? "not ok 2" : "ok 2", "\n";
	$r = QNX::Reply($rpid, $rvmsg, $rsize);
	print $r == 0 ? "ok 3" : "not ok 3", "\n";
} else {
	$r = QNX::Send($pid, $smsg, $rmsg, $ssize, $rsize);
	print $r == 0 ? "ok 4" : "not ok 4", "\n";
	print $rmsg eq $trunc_msg ? "ok 5" : "not ok 5", "\n";
}
