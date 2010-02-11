#! /usr/bin/perl -w

my $year = shift @ARGV || die "No year specified\n";
-d "Logs/$year" || die "unable to locate log dir Logs/$year\n";
my @dirs = sort <Logs/$year/Run_*>;
my @runs;
for my $dir ( @dirs ) {
  if ( -d $dir && -f "$dir/index.html" ) {
    my $title = 'Unknown';
    my $status = 'Unknown';
    if (open(IDX, "<$dir/index.html")) {
      while (<IDX>) {
	if ( m/Final run status: (\w+)/ ) {
	  $status = $1;
	}
	if ( m|<title>([^<]+)</title>| ) {
	  $title = $1;
	}
      }
      close(IDX) || warn "Error closing $dir/index.html\n";
    } else {
      warn "Unable to read $dir/index.html\n";
    }
    $dir =~ m|/([^/]+)$| || die "Bad pattern\n";
    my $run = $1;
    push( @runs, "<tr><th><a href=\"$run/index.html\">$run</a></th><td>$title</td><td>$status</td></tr>\n" );
  }
}

open(IDX, ">Logs/$year/index.html") || die "Unable to write Logs/$year/index.html\n";
print IDX
  "<!DOCTYPE html PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\">\n",
  "<html>\n",
  "<head>\n",
  "<link href=\"../4xdaily.css\" rel=\"stylesheet\" type=\"text/css\">\n",
  "<title>Run Log $year</title>\n",
  "</head>\n",
  "<body>\n",
  "<h1>Run Log $year</h1>\n",
  "<table>\n",
  "<tr class=\"top\"><th>Run</th><th>Descriptions</th><th>Status</th></tr>\n",
  @runs,
  "</table>\n",
  "</body>\n",
  "</html>\n";
close IDX || warn "Error closing Logs/$year/index.html\n";

