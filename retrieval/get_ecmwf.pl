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
my $restart = 0;
my $request_year;

# Index currently includes:
# ReqNum
# Time
# Method (with link to reqest and response headers)
# Duration
# Status
# Optional link to content
#
# Should include a comment field describing each requeust
# 
sub append_to_index {
  my ( $reqnum, $comment, $time, $method, $headerlink, $duration, $status, $contentlink ) = @_;
  $| = 1;  
  # Now add a line to the main index
  if ( defined $reqnum ) {
    if ( $reqnum > 0) {
      print INDEX
	"<tr><td>$reqnum</td>",
	"<td>$time</td>",
	"<td>$comment</td>\n",
	"<td><a href=\"$headerlink\">$method</a></td>",
	"<td>$duration</td>",
	"<td>$status</td>",
	"<td>", $contentlink ? "<a href=\"$contentlink\">content</a>" : '',
	"</td></tr>\n";
    } else {
      print INDEX
	"<tr><td></td><td colspan=\"6\">$comment</td></tr>\n";
    }
  }
  my $logpos = tell INDEX;
  print INDEX <<EOF;
</table>
</body>
</html>
EOF
  seek INDEX, $logpos, 0;
} 

sub append_to_index_and_die {
  my $msg = join '', @_;
  append_to_index( 0, $msg );
  append_to_index( 0, "Final run status: failure" );
  system("retrieval/build_index.pl $request_year");
  die "$msg\n";
}


sub initialize_logs {
  my ( $datestr, $paramstr ) = @_;
  $datestr =~ m|^(\d+)/(\d+)$| || die "Bad pattern in initialize_logs()\n";
  my $month = $1;
  my $year = $2;
  -d $year || mkdir $year || die "Unable to create result directory '$year'\n";
  -d "Logs" || mkdir "Logs" || die "Unable to create Logs directory\n";
  -d "Logs/$year" || mkdir "Logs/$year" || die "Unable to create log directory 'Logs/$year'\n";
  $resultfile = sprintf( "$year/%s.4xdaily.$year-%02d.nc", lc($paramstr), $month );
  $resultfile =~ s/ //g;
  $request_year = $year;
  my $runidx = 0;
  do {
    ++$runidx;
    $rundir = sprintf("Logs/$year/Run_%02d", $runidx);
  } while -e $rundir;
  print "Run number $runidx\n";
  mkdir $rundir || die "Unable to create run directory '$rundir'\n";
  open(INDEX, ">$rundir/index.html") || die "Unable to write $rundir/index.html\n";
  print INDEX <<EOF;
<!DOCTYPE html PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">
<html>
<head>
<link href="../../4xdaily.css" rel="stylesheet" type="text/css">
EOF
  my $title = "Run $runidx: ecmwf retrieval: " . escapeHTML("$datestr, $paramstr");
  $title .= " Restart" if $restart;
  print INDEX
    "<title>$title</title>\n",
    "</head>\n",
    "<body>\n",
    "<h1>$title</h1>\n";
  $| = 1;
  print INDEX <<EOF;
<table>
<tr class="top"><th>#</th><th>Time</th><th>Comment</th><th>Method</th><th>Duration</th><th>Status</th><th>Result</th></tr>
EOF
  append_to_index(); # close out the HTML
  system("retrieval/build_index.pl $year");
  $reqnum = 1;
}

# $response = log_request($comment, $ua, $request );
# Strategy:
#   $rundir/index.html lists each request, start time and duration and
#     provides links to display the request, response headers and response content
sub log_request {
  my ( $comment, $ua, $req, @opts ) = @_;
  my $index = "$rundir/index.html";
  
  my $reqlog = sprintf("req_%02d.html", $reqnum);
  open REQ, ">$rundir/$reqlog" || die "Cannot write request log '$rundir/$reqlog'\n";
  print REQ <<EOF;
<!DOCTYPE html PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">
<html>
<head>
<link href="../../4xdaily.css" rel="stylesheet" type="text/css">
EOF
  $| = 1;
  print REQ "<title>Request $reqnum: $datestr $paramstr</title>\n",
    "</head>\n",
    "<body>\n",
    "<h1>Request $reqnum: $datestr $paramstr</h1>\n",
    "<h2>Request:</h2>\n",
    "<pre>\n",
    $req->method, " ",  $req->uri->as_string, "\n",
    $req->headers->as_string,
    "</pre>\n";
  my $reqcont = $req->content;
  if ( $reqcont ) {
    print REQ "<table><tr><th>Param</th><th>Value</th></tr>\n";
    @pairs = split /&/, $reqcont;
    for my $pair (@pairs) {
      if ( $pair =~ m/^([^=]*)=([^=]*)$/ ) {
	print REQ "<tr><td>$1</td><td>$2</td></tr>\n";
      } else {
	print REQ "<tr><td colspan=\"2\">$pair</td></tr>\n";
      }
    }
    print REQ "</table>\n";
  }
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

  append_to_index( $reqnum, $comment, scalar(localtime($starttime)), $req->method,
	$reqlog, $elapsed, $response->code, $content );
  ++$reqnum;
  return $response;
}

# $response = log_get( $comment, $ua, $url, $referer );
# $response = log_get( $comment, $ua, $url, $referer, $content_filename );
sub log_get {
  my ( $comment, $ua, $url, $referer, @opts ) = @_;
  my @ref;
  push @ref, referer => $referer if $referer;
  my $request = HTTP::Request::Common::GET( $url, @ref );
  return log_request( $comment, $ua, $request, @opts );
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
    #print "Following redirect to $newurl\n";
    $response = log_get( "Follow redirect", $ua, $newurl, $oldurl );
}

sub log_newlink {
  my ( $comment, $ua, $resp, $newurl, @opts ) = @_;
  my $oldurl = $resp->request->uri;
  $newurl = new_url( $oldurl, $newurl );
  # print "Following link to $newurl\n";
  return log_get( $comment, $ua, $newurl, $oldurl, @opts );
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

sub check_timeout {
  my ($ua, $response) = @_;
  while ( $response->is_success &&
	  $response->decoded_content =~ m/Application\s+timeout\s+alarm\s+received/ ) {
    append_to_index( 0, "Application Timeout" );
    sleep 60;
    $response = log_get( "Retry", $ua, $response->request->uri );
  }
  return $response;
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
  } elsif ( $arg =~ m/^restart$/i ) {
    $restart = 1;
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
print "Requested $datestr $paramstr", $restart ? " Restart" : '', "\n";

initialize_logs( $datestr, $paramstr );

  my $ua = LWP::UserAgent->new;
  $ua->cookie_jar({ file => "cookies.ecmwf", autosave => 1 });
  my $request;

  # Request pressure-level page:
  # http://data-portal.ecmwf.int/data/d/interim_daily/levtype=pl/
  # using what cookies we have.
  my $host = "http://data-portal.ecmwf.int";
  my $response = log_get( "Initial request", $ua, "$host/data/d/interim_daily/levtype=pl" );
  append_to_index_and_die( "Initial request failed: ", $response->status_line )
    unless $response->is_success;
  $response = check_timeout( $ua, $response );

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
    $response = log_request( "Agree to Terms", $ua, $request );
    if ( $response->is_redirect ) {
      $response = log_redirect( $ua, $response );
    } elsif ( $response->is_success ) {
      append_to_index_and_die("Expected redirect after license acceptance: Got success");
    } else {
      append_to_index_and_die("License acceptance failed: ". $response->status_line );
    }
    if ( $response->content =~
	  m/In order to retrieve data from this server, you first have to accept the/ ) {
      append_to_index_and_die(
	"License acceptance request succeeded, but still see verbiage." );
    }
    append_to_index( 0, "License granted" );
  } else {
    append_to_index( 0, "We apparently already have our license cookie installed." );
  }

  my $ok_to_timeout = 0;
  if ( $restart ) {
    $response = log_get( "Personal Results Page", $ua, "$host/data/d/inspect/personal/results/" );
    $response = check_timeout($ua, $response);
    append_to_index_and_die("Failure from presonal results page") unless $response->is_success;
    append_to_index_and_die("Could not find active request")
      unless $response->decoded_content =~
	m|<a href="(/data/d/inspect/personal/results/[^"]+)">|;
    $response = log_newlink("Inspect Job", $ua, $response, $1);
    $ok_to_timeout = 1;
  } else {
    # Now format data request for one month
    $request = HTTP::Request::Common::POST( "$host/data/d/interim_daily/levtype=pl/", \@params );
    $response = log_request( "Submit Data Request", $ua, $request );

    # Then I get a confirmation page, which I need to parse and follow
    append_to_index_and_die("Expected redirect after submission") unless $response->is_redirect;
    $response = log_redirect( $ua, $response );
    $response = check_timeout( $ua, $response );
    append_to_index_and_die("Failure after post and redirect") unless $response->is_success;

    append_to_index_and_die("Did not find the content I was looking for") unless  
      $response->content =~ m|The\snetcdf\swill\sbe\sdone\susing\sthe\sfollowing\sattributes:
      .* <a\shref="([^"]*)">Now</a>|xs;
      
    $response = log_newlink("Confirm Request", $ua, $response, $1 );
    ### Do not retry on application timeout, or the job will be submitted twice
  }

  # Then I will need to handle refreshes until the request is completed
  while ( $response->is_success ) {
    $response = check_timeout( $ua, $response ) if $ok_to_timeout;
    if ( $response->content =~
            m|<meta\s+http-equiv="Refresh"\s+content="(\d+);\s*([^"]+)"\s*>| ) {
      #print "Refresh: $1; $2\n";
      sleep $1*3;
      $response = log_newlink( "meta Refresh", $ua, $response, $2 );
      $ok_to_timeout = 1;
    } elsif ( $response->header('Refresh') ) {
      my $refresh = $response->header('Refresh');
      append_to_index_and_die( "Expected number in refresh" ) unless $refresh =~ m/^\d+$/;
      sleep $refresh*3;
      $response = log_get( "Refresh $refresh", $ua, $response->request->uri, $response->request->uri );
      $ok_to_timeout = 1;
    } else {
      last;
    }
  }
  append_to_index_and_die("Non-success after possible refreshes") unless $response->is_success;
  my $results_link = find_link( $response, "Results of your tasks" ) ||
    append_to_index_and_die("Did not find 'Results' link");
  my $filelink = find_link( $response, "nc" ) ||
    append_to_index_and_die("Did not find 'nc' link");
  # Then I need to request the actual file
  my $retrresp = log_newlink( "Download nc", $ua, $response, $filelink, "$rundir/output.nc" );  
  append_to_index_and_die("File retrieval was not success") unless $retrresp->is_success;
  if ( -f $resultfile ) {
    -f "$resultfile.bak" && unlink("$resultfile.bak");
    if (-f $resultfile) {
      append_to_index( 0, "Previous version of $resultfile exists: attempting rename" );
      append_to_index_and_die("Unable to rename old $resultfile")
	unless rename( $resultfile, "$resultfile.bak");
      append_to_index_and_die("rename failed unexpectedly") if -f $resultfile;
    }
  }
  append_to_index(0, "Moving result to $resultfile" );
  rename("$rundir/output.nc", $resultfile) ||
    append_to_index_and_die( "Move reported failure" );
  append_to_index_and_die( "Move apparently failed" ) unless -f $resultfile;

  $response = log_newlink( "See all results", $ua, $response, $results_link );
  append_to_index_and_die("Results link not success") unless $response->is_success;
# Then I need to delete the products off the web site
  my $dellink = find_link( $response, '<img title="erase" width="20" alt="erase" height="20" border="0" src="/contrib/images/icons/erase.gif" />' );
  if ( $dellink ) {
    $response = log_newlink( "Delete File", $ua, $response, $dellink );
    append_to_index( 0, "Delete request reported " . $response->is_success ? "success" : "failure");
  } else {
    append_to_index( 0, "Failed to find delete link");
  }
  append_to_index( 0, "Final run status: success" );
  system("retrieval/build_index.pl $request_year");
 # system("cygstart $rundir/index.html");
  exit(0);
  
