#! ./perl

BEGIN { $| = 1; print "1..6\n"; }
END {print "not ok 1\n" unless $loaded;}
use QNX;
$loaded = 1;
print "ok 1\n";

######################### End of black magic.

# Name_locate a non_existent name. 
# Then attach that name to this process. Run name_locate again.
# Try to detach the incorrect name_id. Then detach the correct one.
# Once again try to name_locate the detached name.

$name = "QNXtest/noname";
$copies = 3;
$size = 81;

$pid = QNX::name_locate( 0, $name, $size, $copies );
print $pid == -1 ? "ok 2" : "not ok 2", "\n";

$id = QNX::name_attach( 0, $name );
$pid = QNX::name_locate( 0, $name, $size, $copies );
print $pid == -1 ? "not ok 3" : "ok 3", "\n";

print QNX::name_detach( 0, $id+1 ) == -1 ? "ok 4" : "not ok 4", "\n";

print QNX::name_detach( 0, $id ) == 0 ? "ok 5" : "not ok 5", "\n";
$pid = QNX::name_locate( 0, $name, $size, $copies );
print $pid == -1 ? "ok 6" : "not ok 6", "\n";
