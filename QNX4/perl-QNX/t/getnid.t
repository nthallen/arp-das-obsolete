BEGIN { $| = 1; print "1..2\n"; }
END {print "not ok 1\n" unless $loaded;}
use QNX;
use POSIX;
$loaded = 1;
print "ok 1\n";

# check whether QNX:getnid() returns the machine network node number
# the same as command 'uname -n' does...

my $nid = QNX::getnid();
my ( $sysname, $nodename, $release, $version, $machine ) = POSIX::uname();
print $nodename eq $nid ? "ok " : "not ", "2\n";
