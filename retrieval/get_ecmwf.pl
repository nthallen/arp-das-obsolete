#! /usr/bin/perl -w
use LWP::UserAgent;
use URI::URL;
use HTTP::Cookies;
use HTTP::Request::Common;
use CGI qw(escapeHTML);

# ./get_ecmwf.pl 1;1989 temp

my %parameter_keys = (
  "Cloud cover" => 248,
  "Cloud ice water content" => 247,
  "Cloud liquid water content" => 246,
  "Divergence" => 155,
  "Geopotential" => 129,
  "Ozone mass mixing ratio" => 203,
  "Potential vorticity" => 60,
  "Relative humidity" => 157,
  "Specific humidity" => 133,
  "Temperature" => 130,
  "U velocity" => 131,
  "V velocity" => 132,
  "Vertical velocity" => 135,
  "Vorticity (relative)" => 138 );

#my @levels = qw(1000 925 850);
my @levels = qw(1000  975  950  925  900  875  850  825  800  775
   750  700  650  600  550  500  450  400  350  300  250  225  200
   175  150  125  100  70  50  30  20  10  7  5  3  2  1);

my @params = (
    _date_start_date => '1989-01-01',
    _date_end_date => '2009-11-30',
    _date_month_only => '0',
    _date_start => '1989-01-01',
    _date_end => '2009-11-30',
    _date_choice => '3',
    time => '00:00:00',
    time => '06:00:00',
    time => '12:00:00',
    time => '18:00:00',
    class => 'ei',
    dataset => 'interim_daily',
    step => '0',
    levtype => 'pl',
    "Retrieve NetCDF" => "Retrieve NetCDF" );

# Create a logging directory
my $resultfile;
my $rundir;
my $reqnum;
my $datestr = '';
my $paramstr = '';

sub initialize_logs {
  my ( $datestr, $paramstr ) = @_;
  $datestr =~ m|^(\d+)/(\d+)$| || die "Bad pattern in initialize_logs()\n";
  my $month = $1;
  my $year = $2;
  -d $year || mkdir $year || die "Unable to create result directory '$year'\n";
  -d "$year/Logs" || mkdir "$year/Logs" || die "Unable to create log directory '$year/Logs'\n";
  $resultfile = sprintf( "$year/%s.4xdaily.$year-%02d.nc", lc($paramstr), $month );
  $resultfile =~ s/ //g;
  my $runidx = 0;
  do {
    ++$runidx;
    $rundir = sprintf("$year/Logs/Run_%02d", $runidx);
  } while -e $rundir;
  print "Run number $runidx\n";
  mkdir $rundir || die "Unable to create run directory '$rundir'\n";
  open(INDEX, ">$rundir/index.html") || die "Unable to write $rundir/index.html\n";
  print INDEX <<EOF;
<!DOCTYPE html PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">
<html>
<head>
EOF
  print INDEX
    "<title>Run $runidx Request Log</title>\n",
    "</head>\n",
    "<body>\n",
    "<h1>Run $runidx: ecmwf retrieval: ", escapeHTML("$datestr, $paramstr"), "</h1>\n";
  $| = 1;
  print INDEX <<EOF;
<table>
<tr><th>#</th><th>request</th><th>start</th><th>duration</th><th>status</th><th>result</th></tr>
EOF
  my $indexpos = tell INDEX;
  print INDEX <<EOF;
</table>
</body>
</html>
EOF
  seek INDEX, $indexpos, 0;
  $reqnum = 1;
}

# $response = log_request( $ua, $request );
# Strategy:
#   $rundir/index.html lists each request, start time and duration and
#     provides links to display the request, response headers and response content
sub log_request {
  my ( $ua, $req, @opts ) = @_;
  my $index = "$rundir/index.html";
  
  my $reqlog = sprintf("req_%02d.html", $reqnum);
  open REQ, ">$rundir/$reqlog" || die "Cannot write request log '$rundir/$reqlog'\n";
  print REQ <<EOF;
<!DOCTYPE html PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">
<html>
<head>
EOF
  $| = 1;
  print REQ "<title>Request $reqnum: $datestr $paramstr</title>\n",
    "</head>\n",
    "<body>\n",
    "<h1>Request $reqnum: $datestr $paramstr</h1>\n",
    "<h2>Request:</h2>\n",
    "<pre>\n",
    escapeHTML($req->as_string),
    "</pre>\n";
  my $logpos = tell REQ;
  print REQ  
    "</body>\n",
    "</html>\n";
  seek(REQ,$logpos,0);
  my $starttime = time;
  my $response = $ua->request($req, @opts);
  my $endtime = time;
  my $dt = $endtime-$starttime;
  my $elapsed = sprintf("%02d:%02d", $dt/60, $dt%60);
  my $content = @opts ? '' : sprintf("cont_%02d.html", $reqnum);
  if ( $content) {
    open CONTENT, ">$rundir/$content" || die "Unable to write content to '$rundir/$content'\n";
    print CONTENT $response->decoded_content;
    close CONTENT || warn "Error closing content file '$rundir/$content'\n";
  }
  
  print REQ
    "<h2>Response:</h2>\n",
    "<table>\n",
    "<tr><th>Time:</th><td>", scalar(localtime($starttime)), "</td></tr>\n",
    "<tr><th>Duration:</th><td>$elapsed</td></tr>\n",
    "<tr><th>Status:</th><td>",
    $content ? "<a href=\"$content\">" : '',
    $response->status_line,
    $content ? "</a>" : '',
    "</td></tr>\n",
    "<tr><th valign=\"top\">Headers:</th><td><pre>", escapeHTML($response->headers->as_string),
    "</pre></td></tr>\n",
    "</table></body></html>\n";
  close REQ || warn "Error closing request file '$reqlog'\n";

  $| = 1;  
  # Now add a line to the main index
  print INDEX
    "<tr><td>$reqnum</td><td><a href=\"$reqlog\">",
    $req->method,
    "</a></td>",
    "<td>", scalar(localtime($starttime)), "</td><td>$elapsed</td>",
    "<td>", $response->code, "</td><td>",
    $content ? "<a href=\"$content\">content</a>" : '',
    "</td></tr>\n";
  $logpos = tell INDEX;
  print INDEX <<EOF;
</table>
</body>
</html>
EOF
  seek INDEX, $logpos, 0;
  
  ++$reqnum;
  return $response;
}

# $response = log_get( $ua, $url, $referer );
# $response = log_get( $ua, $url, $referer, $content_filename );
sub log_get {
  my ( $ua, $url, $referer, @opts ) = @_;
  my @ref;
  push @ref, referer => $referer if $referer;
  my $request = HTTP::Request::Common::GET( $url, @ref );
  return log_request( $ua, $request, @opts );
}

sub new_url {
  my ( $oldurl, $newurl ) = @_;
  unless ( $newurl =~ m|^https?://| ) {
    $oldurl =~ m|^(https?://[^/]*)((?:(?:/.*)?/)?)[^/]*$| ||
      die "Bad pattern in redirect\n";
    my $oldhost = $1;
    my $oldpath = $2 || '/';
    if ( $newurl =~ m|^/| ) {
      $newurl = "$oldhost$newurl";
    } else {
      $newurl = "$oldhost$oldpath$newurl";
    }
  }
  return $newurl;
}

sub log_redirect {
    my ( $ua, $response ) = @_;
    die "redirect on non-redirect\n" unless $response->is_redirect;
    my $oldurl = $response->request->uri;
    my $newurl = $response->header('Location');
    $newurl = new_url($oldurl, $newurl);
    print "Following redirect to $newurl\n";
    $response = log_get( $ua, $newurl, $oldurl );
}

sub log_newlink {
  my ( $ua, $resp, $newurl, @opts ) = @_;
  my $oldurl = $resp->request->uri;
  $newurl = new_url( $oldurl, $newurl );
  print "Following link to $newurl\n";
  return log_get( $ua, $newurl, $oldurl, @opts );
}

sub find_link {
  my ( $resp, $text ) = @_;
  my $src = $resp->decoded_content;
  my $link;
  while ( $src =~ m|<a href="([^"]+)">$text</a>|g ) {
    if ( $link && $link ne $1 ) {
      warn "find_link($text) matched multiple links\n";
      return '';
    }
    $link = $1;
  }
  return $link;
}

for my $arg ( @ARGV ) {
  if ( $arg =~ m|^(\d+)/(\d{4})$| ) {
    if ( $1 > 0 && $1 <= 12 && $2 >= 1989 ) {
      # specify the month and year:
      push( @params, _date_year_month => "$1;$2" );
      die "More than one date specified\n" if $datestr;
      $datestr = "$1/$2";
    } else {
      die "Invalid date specified: '$arg'\n";
    }
  } else {
    my @matches = grep m/^$arg/i, keys %parameter_keys;
    if ( @matches == 0 ) {
      die "Unknown parameter: '$arg'\n";
    } elsif ( @matches > 1 ) {
      die "Ambiguous parameter: '$arg' matches: " . join( ", ", map "'$_'", @matches) . "\n";
    } else {
      my $param = $parameter_keys{$matches[0]};
      for my $level ( @levels ) {
        push( @params, levelist_param => "$level;$param.128" );
      }
      die "More than one parameter specified\n" if $paramstr;
      $paramstr = $matches[0];
    }
  }
}

die "No date specified\n" unless $datestr;
die "No parameter specified\n" unless $paramstr;
print "Requested $datestr $paramstr\n";

initialize_logs( $datestr, $paramstr );

my $ua = LWP::UserAgent->new;
$ua->cookie_jar({ file => "cookies.ecmwf", autosave => 1 });
my $request;

# Request pressure-level page:
# http://data-portal.ecmwf.int/data/d/interim_daily/levtype=pl/
# using what cookies we have.
my $host = "http://data-portal.ecmwf.int";
print "Submitting initial request:\n";
my $response = log_get( $ua, "$host/data/d/interim_daily/levtype=pl" );
if ( ! $response->is_success ) {
  die "Initial request failed: $response->status_line\n";
}

# If response includes "In order to retrieve data from this server, you first have to accept the"
# then post form to /data/d/license/interim/ and parse the result for cookies, writing them to a file
if ( $response->content =~ m/In order to retrieve data from this server, you first have to accept the/ ) {
  print "Submitting license acceptance:\n";
  my $path = '/data/d/license/interim/';
  $request = HTTP::Request::Common::POST(
    "$host$path",
    [ dataset => 'interim',
    referer => "http://data-portal.ecmwf.int/data/d/interim_daily/",
    '_name' => "Norton Allen",
    '_email' => 'allen@huarp.harvard.edu',
    '_organisation' => 'Harvard University',
    '_country' => 'United States',
    Accept => 'Accept' ]);
  $response = log_request( $ua, $request );
  if ( $response->is_redirect ) {
    $response = log_redirect( $ua, $response );
  } elsif ( $response->is_success ) {
    die "Expected redirect after license acceptance: Got success\n";
  } else {
    die "License acceptance failed: ". $response->status_line . "\n";
  }
  if ( $response->content =~ m/In order to retrieve data from this server, you first have to accept the/ ) {
    die "License acceptance request succeeded, but still see verbiage.\n";
  }
  # $ua->cookie_jar->extract_cookies( $response );
  # $ua->cookie_jar->save;
  print "License granted\n";
} else {
  print "We apparently already have our license cookie installed.\n";
}

# Now format data request for one month
$request = HTTP::Request::Common::POST( "$host/data/d/interim_daily/levtype=pl/", \@params );
$response = log_request( $ua, $request );

# Then I get a confirmation page, which I need to parse and follow
  die "Expected redirect after submission\n" unless $response->is_redirect;
  $response = log_redirect( $ua, $response );
  die "Failure after post and redirect\n" unless $response->is_success;

  die "Did not find the content I was looking for\n" unless  
    $response->content =~ m|The\snetcdf\swill\sbe\sdone\susing\sthe\sfollowing\sattributes:
    .* <a\shref="([^"]*)">Now</a>|xs;
    
  $response = log_newlink( $ua, $response, $1 );
  
  # Then I will need to handle refreshes until the request is completed
  while ( $response->is_success ) {
    if ( $response->content =~
            m|<meta\s+http-equiv="Refresh"\s+content="(\d+);\s*([^"]+)"\s*>| ) {
      print "Refresh: $1; $2\n";
      sleep $1;
      $response = log_newlink( $ua, $response, $2 );
    } elsif ( $response->header('Refresh') ) {
      my $refresh = $response->header('Refresh');
      die "Expected number\n" unless $refresh =~ m/^\d+$/;
      print "Refresh: $refresh\n";
      sleep $refresh;
      $response = log_get( $ua, $response->request->uri, $response->request->uri );
    } else {
      last;
    }
  }
  die "Non-success after possible refreshes\n" unless $response->is_success;
  my $results_link = find_link( $response, "Results of your tasks" ) ||
    die "Did not find 'Results' link\n";
  my $filelink = find_link( $response, "nc" ) ||
    die "Did not find 'nc' link\n";
  # Then I need to request the actual file
  my $retrresp = log_newlink( $ua, $response, $filelink, "$rundir/output.nc" );  
  die "File retrieval was not success\n" unless $retrresp->is_success;
  if ( -f $resultfile ) {
    -f "$resultfile.bak" && unlink("$resultfile.bak");
    if (-f $resultfile) {
      print "Previous version of $resultfile exists: attempting rename\n";
      die "Unable to rename old $resultfile\n" unless rename( $resultfile, "$resultfile.bak");
      die "rename failed unexpectedly\n" if -f $resultfile;
    }
  }
  print "Moving result to $resultfile\n";
  rename("$rundir/output.nc", $resultfile) || die "Move reported failure\n";
  die "Move apparently failed\n" unless -f $resultfile;

  $response = log_newlink( $ua, $response, $results_link );
  die "Results link not success\n" unless $response->is_success;
# Then I need to delete the products off the web site
  my $dellink = find_link( $response, '<img title="erase" width="20" alt="erase" height="20" border="0" src="/contrib/images/icons/erase.gif" />' );
  if ( $dellink ) {
    $response = log_newlink( $ua, $response, $dellink );
    print "Delete request reported ", $response->is_success ? "success" : "failure", "\n";
  } else {
    print "Failed to find delete link\n";
  }
 # system("cygstart $rundir/index.html");
  exit(0);
  
