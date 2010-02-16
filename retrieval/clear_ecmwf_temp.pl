#! /usr/bin/perl -w
use LWP::UserAgent;
use URI::URL;
use HTTP::Cookies;
use HTTP::Request::Common;
use CGI qw(escapeHTML);

my $rundir;
my $reqnum;
my $request_year = 'admin';

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
  my $year = $request_year;
  -d "Logs" || mkdir "Logs" || die "Unable to create Logs directory\n";
  -d "Logs/$year" || mkdir "Logs/$year" || die "Unable to create log directory 'Logs/$year'\n";
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
  my $title = "Run $runidx: ecmwf clear temp: " . escapeHTML("$year");
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
  print REQ "<title>Request $reqnum: Clear Temp</title>\n",
    "</head>\n",
    "<body>\n",
    "<h1>Request $reqnum: Clear Temp</h1>\n",
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

print "Requested Clear Temp\n";

initialize_logs( );

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

  $response = log_get( "Personal Results Page", $ua, "$host/data/d/inspect/personal/temporary/" );
  while (1) {
    $response = check_timeout($ua, $response);
    append_to_index_and_die("Failure from temporary results page") unless $response->is_success;
    if ( $response->decoded_content =~
	m|<a href="(/data/d/erase/personal/temporary/[^"]+)">| ) {
      print "$1\n";
      $response = log_newlink("Erase Job", $ua, $response, $1);
    } else {
      append_to_index(0, "Apparently no more temp jobs");
      last;
    }
  }
  append_to_index( 0, "Final run status: success" );
  system("retrieval/build_index.pl admin");
  # system("retrieval/build_root.pl");
  exit(0);
  
