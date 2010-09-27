#! /usr/bin/perl -w
use LWP::UserAgent;
use URI::URL;
use HTTP::Cookies;
use HTTP::Request::Common;
use CGI qw(escapeHTML);


sub build_index {
  my $title = shift @_ || die "No year specified\n";
  -d "Logs" || die "unable to locate log dir Logs\n";
  my @dirs = sort <Logs/Run_*>;
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

  open(IDX, ">Logs/index.html") || die "Unable to write Logs/index.html\n";
  print IDX
    "<!DOCTYPE html PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\">\n",
    "<html>\n",
    "<head>\n",
    "<link href=\"../4xdaily.css\" rel=\"stylesheet\" type=\"text/css\">\n",
    "<title>Run Log $title</title>\n",
    "</head>\n",
    "<body>\n",
    "<h1>Run Log $title</h1>\n",
    "<table>\n",
    "<tr class=\"top\"><th>Run</th><th>Descriptions</th><th>Status</th></tr>\n",
    @runs,
    "</table>\n",
    "</body>\n",
    "</html>\n";
  close IDX || warn "Error closing Logs/index.html\n";
}

# Create a logging directory
my $rundir;
my $reqnum;

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
    } elsif (defined $comment) {
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
  build_index( "log_http" );
  die "$msg\n";
}


sub initialize_logs {
  my ( $url ) = @_;
  -d "Logs" || mkdir "Logs" || die "Unable to create Logs directory\n";
  my $runidx = 0;
  do {
    ++$runidx;
    $rundir = sprintf("Logs/Run_%02d", $runidx);
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
  my $title = "Run $runidx: log http: " . escapeHTML("$url");
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
  build_index( "log_http");
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
  print REQ "<title>Request $reqnum:</title>\n",
    "</head>\n",
    "<body>\n",
    "<h1>Request $reqnum:</h1>\n",
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
  $newurl =~ s/^URL=//;
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
  # while ( $response->is_success &&
          # $response->decoded_content =~ m/Application\s+timeout\s+alarm\s+received/ ) {
    # append_to_index( 0, "Application Timeout" );
    # sleep 60;
    # $response = log_get( "Retry", $ua, $response->request->uri );
  # }
  return $response;
}

for my $url ( @ARGV ) {
  initialize_logs( $url );

  my $ua = LWP::UserAgent->new;
  $ua->cookie_jar({ file => "cookies.ecmwf", autosave => 1 });
  my $request;

  my $response = log_get( "Initial request", $ua, "$url" );
  append_to_index_and_die( "Initial request failed: ", $response->status_line )
    unless $response->is_success;
  $response = check_timeout( $ua, $response );
  
  while ( $response->is_success ) {
    # Then I get a confirmation page, which I need to parse and follow
    if ( $response->is_redirect ) {
      $response = log_redirect( $ua, $response );
      $response = check_timeout( $ua, $response );
      unless ( $response->is_success ) {
        append_to_index(0, "Failure after redirect");
        last;
      }
    } elsif ( $response->content =~
            m|<meta\s+http-equiv="Refresh"\s+content="(\d+);\s*([^"]+)"\s*>|i ) {
      #print "Refresh: $1; $2\n";
      sleep $1*3;
      $response = log_newlink( "meta Refresh", $ua, $response, $2 );
      $ok_to_timeout = 1;
    } elsif ( $response->header('Refresh') ) {
      my $refresh = $response->header('Refresh');
      append_to_index_and_die( "Expected number in refresh" )
        unless $refresh =~ m/^\d+$/;
      sleep $refresh*3;
      $response = log_get( "Refresh $refresh", $ua,
        $response->request->uri, $response->request->uri );
      $ok_to_timeout = 1;
    } else {
      last;
    }
  }
  unless ( $response->is_success ) {
    append_to_index( 0, "Non-success after possible refreshes" );
    next;
  }
  append_to_index( 0, "Final run status: success" );
  build_index( "log_http" );
}
exit(0);
  
