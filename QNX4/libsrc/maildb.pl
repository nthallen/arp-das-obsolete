#! //2/usr/local/bin/perl
#__USAGE
#%C	[options]
#	-v       verbose
#	-n       don't generate maildb.text
#	-m       Mail configuration to each user
#	-u user  Only show info for specified user (implies -n)

# Process command-line args
require "getopts.pl";
Getopts('vnu:m');
$opt_n = 1 if $opt_u && $opt_u ne "";

# The point of this script is to translate a source file (such as
# mkal.src) into another source file which the sendmail stuff can
# interpret. The current mkal.src format is

#   <lastname> <firstname> <username> <alias> [ <alias> ... ]
# 	  <address> [ <address> ]

# (all on one line). some aliases may be abbreviated +[lfFu] where
#     l stands for lastname
# 	f stands for firstname
# 	F stands for firstname_lastname
# 	u stands for username

# It is important that any usernames the user has be listed as
# aliases, since mail sent by them to other systems may be reply'ed
# to. It is dangerous if one user uses the username of another user
# as an alias unless of course the purpose is to intercept mail.
# The username is always included as an alias, but the +u
# designation is given to allow use of the username as the primary
# mail address.

# This system is based on the assumption that each person has only
# one username no matter how many accounts they have on different
# systems. It also assumes that identifying mailnames for
# non-existant users is not a problem.

# Addresses don't begin with + or :. More than one address may be
# specified, but delivery to more than one mail server is not
# allowed. The current mail servers are bottesini and molly.

# Mail to the field is also a tricky subject yet to be fully
# resolved. abigail is a mail server also, but it will have a
# somewhat different configuration. Using sendmail's "STICKYHOST"
# option, mail to unadorned usernames will be forwarded to
# huarp.harvard.edu for resolution, using the "home" user database
# to determine forwarding. mail explicitly addressed to abigail
# will be delivered at abigail. By this approach, it would be
# possible to list on bottesini:

#   nort:maildrop  nort@bottesini # or \nort: whatever works
#   nort:maildrop  nort@abigail

# On abigail, we would not want to have duplicate maildrops, since
# that would result in looping. On the other hand, we might still
# need to provide forwarding for users with accounts on abigail who
# are *not* in the field. In other words, it might be reasonable to
# trade off mail forwarding to the field with mail forwarding from
# the field, but the timing is important.

# So field configuration is handled differently.

$mailsrc = "//1/usr/local/sendmail/maildb.src";
$mailtgt = "//1/usr/local/sendmail/maildb.text";

foreach ( "bottesini.harvard.edu",
		  "molly.harvard.edu" ) {
  $servers{$_} = 'yes';
}

open( SRC, "<$mailsrc" ) ||
  die "Unable to open $mailsrc\n";

unless ( $opt_n ) {
  open( TGT , ">$mailtgt" ) ||
	die "Unable to open output file $mailtgt\n";
}

sub newalias {
  my ( $alias, $user ) = @_;
  if ( defined $alias{$alias} ) {
	die "Alias $alias used by users $user and $alias{$alias}\n"
	  if ( $alias{$alias} ne $user );
	return 0; # not a new alias
  } else {
	$alias{$alias} = $user;
	return 1;
  }
}

SRCLINE:
while (<SRC>) {
  my @aliases;
  my @addresses;
  my $toservers=0;
  my ( $last, $first, $user, @line );
  next SRCLINE if /^\s*#/;
  chop;
  if ( /^\s*:([Mm]ail)?[Hh]ost/ ) {
	my ( $kw, $host, $hostname ) = split;
	$host{ $host } = $hostname;
	$servers{ $hostname } = 'yes' if $1 =~ /[Mm]ail/;
	next SRCLINE;
  }
  if ( /^\s*~List\s*/ ) {
	( $last, $first, @line ) = split;
	$user = $first;
  } else {
	( $last, $first, $user, @line ) = split;
  }
  print "not initialized?\n" unless @line;
  foreach my $field (@line) {
	if ( $field =~ s/^\+// ) {
	  while ( $field =~ s/^(.)// ) {
		my $alias;
		$alias = $user if $1 eq "u";
		$alias = $first if $1 eq "f";
		$alias = $last if $1 eq "l";
		$alias = ucfirst $first . "_" . ucfirst $last if $1 eq "F";
		unless ( defined $alias ) {
		  die "Undefined alias code +$1 in $mailsrc at $.\n";
		}
		push( @aliases, $alias ) if ( newalias( $alias, $user ) );
	  }
	} elsif ( $field =~ s/^:// ) {
	  push( @aliases, $field ) if newalias( $field, $user );
	} else {
	  if ( $field =~ /\@/ ) {
		if ( $field =~ /\@(\w+)$/ ) {
		  if ( defined $host{$1} ) {
			$field =~ s/\@(\w+)$/\@$host{$1}/;
		  } else {
			$field =~ s/\@(\w+)$/\@$1.harvard.edu/;
		  }
		}
		$field = "$user$field" if $field =~ m/^\@/;
		if ( $field =~ m/^(\w+)\@([\w.]+)$/ && "$1" eq "$user" &&
			defined $servers{$2} ) {
		  $field = "$1.LOCAL\@$2";
		  $toservers++;
		}
	  }
	  push( @addresses, $field );
	}
  }
  next SRCLINE if defined $opt_u && $opt_u ne $user;
  warn "No addresses defined for user $user\n" if $#addresses < 0;
  push( @aliases, $user ) if ( newalias( $user, $user ) );
  unless ( $opt_n ) {
	printf TGT "%-30s%s\n", "$user:mailname", "$aliases[0]"
	  unless $last eq "~List";
	printf TGT "%-30s", "$user:maildrop";
	my $comma = 0;
	foreach my $address ( @addresses ) {
	  $address =~ s/[.]LOCAL// if ( $toservers < 2 );
	  print TGT "," if $comma++ > 0;
	  print TGT "$address";
	}
	print TGT "\n";
	foreach my $alias ( @aliases ) {
	  printf TGT "%-30s%s\n", "$alias:maildrop", "$user"
		unless $user eq $alias;
	}
  }
  if ( $opt_v ) {
	print "User $user is ", ucfirst $first, " ", ucfirst $last, "\n";
	print "Addresses:\n";
    foreach my $address ( @addresses ) {
	  $address =~ s/[.]LOCAL// if $toservers < 2;
	  print "    $address\n";
	}
	print "Outgoing Alias:\n";
	print "    From: $aliases[0]\@huarp.harvard.edu\n";
	print "Accepting Incoming Aliases:\n";
    foreach my $alias ( @aliases ) {
	  print "    $alias\@huarp.harvard.edu\n";
	}
	print "\n";
  }
  if ( $opt_m && $last ne "~List" ) {
	open( PIPE, "|//1/usr/local/sendmail/sendmail $user" );
	print PIPE "Subject: Mail Aliases\n";
	print PIPE ucfirst $first, " ", ucfirst $last, ",\n\n";
	print PIPE "  Your mail will henceforth be delivered to the\n",
			   "  following address(es):\n\n";
    foreach my $address ( @addresses ) {
	  $address =~ s/[.]LOCAL// if $toservers < 2;
	  print PIPE "    $address\n";
	}
	print PIPE "\n  Your outgoing mail will henceforth be addressed\n";
	print PIPE "\n    From: $aliases[0]\@huarp.harvard.edu\n";
	print PIPE "\n  You will receive mail addressed to any of the\n",
	           "  following aliases:\n\n";
    foreach my $alias ( @aliases ) {
	  print PIPE "    $alias\@huarp.harvard.edu\n";
	}
	print PIPE
	  "\n  If you would like to change any of your aliases, including your",
	  "\n  outgoing address, or change where your mail is delivered,",
	  "\n  please let Norton or David know.\n";
	close(PIPE);
  }
}
close SRC;
unless ( $opt_n ) {
  close TGT;
  foreach $server ( keys %servers ) {
	my $sm = "/usr/local/sendmail";
	$| = 1;
	print "Transferring to Server $server\n";
	my $rv = system( "rcp $mailtgt $server:$sm/useraliases" );
	if ( $rv/256 >= 1 ) {
	  warn "Status $rv returned by rcp\n";
	} else {
	  $rv = system( "rsh $server $sm/newuseraliases" );
	  if ( $rv/256 >= 1 ) {
		warn "Status $rv returned by rsh\n";
	  } else {
		print "Transfer successful to Server $server\n";
	  }
	}
  }
}
