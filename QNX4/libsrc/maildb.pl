#! /usr/local/bin/perl
#__USAGE
#%C	[options]
#	-v       verbose
#	-n       don't generate maildb.text
#	-t       don't distribute maildb.text to servers
#	-m       Mail configuration to each user
#	-u user  Only show info for specified user (implies -n)

# Process command-line args
require "getopts.pl";
Getopts('vnu:mt');
$opt_n = 1 if $opt_u && $opt_u ne "";

# The point of this script is to translate a source file
# (maildb.src) into another source file which the sendmail stuff can
# interpret. The maildb.src format is

# SrcFile : HostDef* UserDef*
# HostDef : ':' HostDesig <host> <hostfqdn>
# HostDesig : "Host" | "MailHost" | "ClientHost"
# UserDef : <lastname> <firstname> <username> <alias>+ <address>+
# <host> : unadorned hostname: e.g. "abigail"
# <hostfqdn> : Fully qualified domain name: e.g. "abigail.bp.espo..."
# <lastname> : e.g. "allen"
# <firstname> : e.g. "norton"
# <username>  : e.g. "nort"
# <alias> : \+[lfFu]+ | :\w+
# <address> : Anything that isn't an alias

# aliases may be abbreviated +[lfFu] where
#   l stands for lastname
#   f stands for firstname
#   F stands for firstname_lastname
#   u stands for username
# Other aliases are indicated by a leading colon: e.g. :TheBoss
# Anything after the <username> which doesn't begin with '+' or
# ':' is an address.

# Addresses referencing hosts for which a HostDef exists will
# have the host name expanded to the FQDN.

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
# specified, including delivery to more than one MailHost.

# When mail is to be delivered to user@host where host is either
# a MailHost or a ClientHost, the alias is translated to
# user.LOCAL@hostfqdn. This allows the host's sendmail configuration
# to defeat any further aliasing and deliver the mail to the user
# on that system. Without this, mail would get duplicated and
# potentially loop.

# In addition to getting the .LOCAL hack, MailHosts also get
# a copy of the alias database and have newuseraliases invoked.

my $mailsrc = "//1/usr/local/sendmail/maildb.src";
my $mailtgt = "//1/usr/local/sendmail/maildb.text";
my ( %servers, %clients );

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
  my ( $last, $first, $user, @line );
  next SRCLINE if /^\s*#/;
  next SRCLINE if /^\s*$/;
  chop;
  if ( /^\s*:\s*(([Mm]ail|[Cc]lient)?[Hh]ost)\s+(\S+)\s+(\S+)\s*$/ ) {
	my ( $kw, $host, $hostname ) = ( $1, $3, $4 );
	$host{ $host } = $hostname;
	$clients{ $hostname } = 'yes' if $kw =~ /[Mm]ail|[Cc]lient/;
	$servers{ $hostname } = 'yes' if $kw =~ /[Mm]ail/;
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
		if ( $field =~ m/^(\w+)\@([\w.-]+)$/ &&
			 ( $1 eq $user ) && $clients{$2} ) {
		  $field = "$1.LOCAL\@$2";
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
	  if ( ( $#addresses < 1 ) &&
		   ( $address =~ m/^(\w+).LOCAL\@([\w.-]+)$/ ) ) {
		if ( $servers{$2} && ! $clients{$2} ) {
		  $address = "$1\@$2";
		}
	  }
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
	  $address =~ s/[.]LOCAL// if $#addresses < 1;
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
	  $address =~ s/[.]LOCAL// if $#addresses < 1;
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
	if ( $opt_t ) {
	  print "Transfer suppressed to Server $server\n";
	} else {
	  my $sm = "/usr/local/sendmail";
	  $| = 1;
	  print "Transferring to Server $server\n";
	  my $rv = system( "rcp -p $mailtgt $server:$sm/useraliases" );
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
}
