BEGIN {
  $| = 1;
  $ntests = 19;
  print "1..$ntests\n";
}
END { print "not ok 1\n" unless $loaded; }
use QNX;
$loaded = 1;
ok( 1, 1 );

######################### End of black magic.

sub ok {
  my ( $num, $cond, $info, $fatal ) = @_;
  
  if ( $cond ) {
	print "ok $num\n";
  } else {
	print "not ok $num";
	print " # $info" if $info;
	print "\n";
	exit 1 if $fatal;
  }
}

sub skip {
  my $reason = shift;
  foreach my $test ( @_ ) {
	print "ok $test # Skip $reason\n";
  }
}


my $prox_msg = 2;
my $size = 1;
my $nsize = 5;
my $name = "/test/vproxy";
my $rem_process = "$^X $0";
$rem_process =~ s/\.t$/.t2/;
my $msg = 0;
my $my_node = QNX::getnid();
ok( 2, $my_node > 0, "QNX::getnid() <= 0"  );
my $rem_node = $my_node;

$id = QNX::name_attach( 0, $name );
ok( 3, $id >= 0, "Unable to attach name", 1 );
$proxy = QNX::proxy_attach( 0, $prox_msg, $size, -1 );
ok( 4, $proxy > 0, "Unable to attach proxy", 1 );

# first check whether we can get a remote node to perform our test
if (!defined( $alive = QNX::net_alive() )) {
  print "ok 5 # Skip No network\n",
		"ok 6 # Skip No network\n";
} else {
  print "ok 5\n";

  @up = grep { $alive->[$_] && $_ != $my_node; } ( 1..$#$alive );
  if ( @up == 0 ) {
	print "ok 6 # Skip No remote nodes\n";
  } else {
	$rem_node = $up[0];
	print "ok 6\n";
  }
}
$rem_proxy = QNX::proxy_rem_attach( $rem_node, $proxy );
ok( 7, $rem_proxy != -1, "Unable to attach remote proxy" );

# Fork to form 2 processes.  On the child do a system call to 
# start running the remote process script on $rem_node.  Set up a name 
# for the parent and attach a proxy and a remote proxy on $rem_node.  
# Wait until the remote process sends the parent a message and then
# send the remote process the pid of the remote proxy.  Wait to receive
# a message from the proxy and make sure it is the correct message.  
# Then detach the name, the proxy, and the remote proxy.

my $child = fork;
if ( $child == 0 ) { # Child
	$r = system( "/usr/bin/on -n$rem_node $rem_process" );
	ok( 14, $r == 0, "Error spawning remote process" );
	exit 0;
}
$pid = QNX::Receive( 0, $rmsg, $size );
ok( 8, $pid != -1, "Error from Receive", 1 );
ok( 9, QNX::Reply( $pid, $rem_proxy, $nsize ) == 0,
		"Error from Reply", 1 );
$r = QNX::Receive( 0, $rmsg, $size );
waitpid( $child, 0 );
ok( 15, $r == $proxy, "Unexpected return from Receive", 1 );
ok( 16, $rmsg == $prox_msg, "Proxy message incorrect" );
ok( 17, QNX::proxy_rem_detach( $rem_node, $rem_proxy ) == 0,
	"Error from proxy_rem_detach" );
ok( 18, QNX::name_detach( 0, $id ) == 0,
	"Error from name_detach" );
ok( 19, QNX::proxy_detach( $proxy ) == 0,
	"Error from proxy_detach" );
