BEGIN { $| = 1; print "1..6\n"; }
END {print "not ok 1\n" unless $loaded;}
use QNX;
$loaded = 1;
print "ok 1\n";

# get the process info for proc

$hrinfo = QNX::psinfo1( 1, 1 );
if (!defined( $hrinfo )) {  print "not ok 2\n"; }
else {
	print "ok 2\n";
	# see if it's really proc that was returned...
	if ($hrinfo->{'pid'} != 1) { print "not ok 3\n"; }
	else {
		print "ok 3\n";
		if ($hrinfo->{'type'} ne 'process') { print "not ok 4\n"; }
		else                                { print "ok 4\n"; }
		# now try to get some info on ourself
		$hrinfo = QNX::psinfo1( 1, $$ );
		if ($hrinfo->{'pid'}  != $$)        { print "not ok 5\n"; }
		else                                { print "ok 5\n"; }
		if ($hrinfo->{'name'} !~ m"perl")   { print "not ok 6\n"; }
		else                                { print "ok 6\n"; }
	}
}
