package HTML;

my @html_refs = ( "index:Components", "master:Master",
				  "SIG_A-C:Signals" );
my @mon = ( "January", "February", "March", "April", "May",
         "June", "July", "August", "September", "October",
		 "November", "December" );
sub trailer {
  local( $subdir, $thisfile, $xtraref ) = @_;
  my @OUTPUT;
  push @OUTPUT, "<P>\n";
  push @OUTPUT, $xtraref if $xtraref;
  foreach ( @html_refs ) {
	local ( $file, $label ) = split( /:/ );
	if ( $file eq $thisfile ) {
	  push @OUTPUT, "[$label]\n";
	} else {
	  push @OUTPUT, "[<A HREF=${subdir}$file.html>$label</A>]\n";
	}
  }
  push @OUTPUT, "<P>\n";
  local( $sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst) =
	localtime;
  $year += 1900;
  push @OUTPUT, "Connector pin assignments by " .
	"<A HREF=\"mailto:terry\@huarp.harvard.edu\">" .
	"Terry Allen</A>.<BR>\n";
  push @OUTPUT, "Web listings compiled by " .
	"<A HREF=\"mailto:allen\@huarp.harvard.edu\">" .
	"Norton T. Allen</A> $mon[$mon] $mday, $year.</P>\n";
  push @OUTPUT, "<P>(c)$year Harvard University " .
   "<A HREF=http://www.arp.harvard.edu>" .
   "Anderson Group</A></P>\n";
  join '', @OUTPUT;
}
