# Before `make install' is performed this script should be runnable with
# `make test'. After `make install' it should work as `perl test.pl'

######################### We start with some black magic to print on failure.

# Change 1..1 below to 1..last_test_to_print .
# (It may become useful if the test is moved to ./t subdirectory.)

BEGIN { $| = 1; print "1..7\n"; }
END {print "not ok 1\n" unless $loaded;}
use QNX;
$loaded = 1;
print "ok 1\n";

######################### End of black magic.

# Locate parent and child through the names attached to them by 
# name_attach and check to make sure the correct pid is returned.  
# Also check the copies value to make sure it is 1.  Then test 
# name_detach as well.

$copies = 3;
$size = 81;
$parent_name = "QNXtest/parent";
$child_name = "QNXtest/child";

if( ($n=fork) == 0 )
{
	$n_id = &QNX::name_attach( 0, $child_name );
	$pid = &QNX::name_locate( 0, $parent_name, $size, $copies );
	print $pid == getppid ? "ok 3" : "not ok 3", "\n";
	print $copies == 1 ? "ok 5" : "not ok 5", "\n";
	$r = &QNX::name_detach( 0, $n_id ); 
	print $r == 0 ? "ok 7" : "not ok 7", "\n";
}else{
	$m_id = &QNX::name_attach( 0, $parent_name );
	$pid = &QNX::name_locate( 0, $child_name, $size, $copies );
	print $pid == $n ? "ok 2" : "not ok 2" , "\n";
	print $copies == 1 ? "ok 4" : "not ok 4", "\n";
	$s = &QNX::name_detach( 0, $m_id );
	print $s == 0 ? "ok 6" : "not ok 6", "\n";
}
