@rem = '--*-Perl-*--
@echo off
perl -x -S %0 %1 %2 %3 %4 %5 %6 %7 %8 %9
goto endofperl
@rem ';
#!perl
#line 8

# Environment Variables:
#   LOCAL_DOC_ROOT
#   HOME  (default doc_root is $HOME/htdocs)
#   COMSPEC     (used under Win32 to launch editor)
#   HTML_EDITOR_CMD Full command to launch editor
#   HTML_EDITOR  program to launch to edit HTML
#   EDITOR_CMD   Full command to launch editor
#   EDITOR       use if HTML_EDITOR is not defined

my $upload_url = 'http://www.arp.harvard.edu/cgi-dev/upload.cgi';

$| = 1;
my $PS = '/'; # Path separator

eval 'use Win32::Registry;';
my $WinReg = $@ ? 0 : 1;
$PS = ':' if $MacPERL::Version;

print "text/editfile client: " , join( " ", @ARGV ), "\n";

if ( $ENV{LOCAL_DOC_ROOT} ) {
  $doc_root = $ENV{LOCAL_DOC_ROOT};
  print "doc_root set to $doc_root using \$LOCAL_DOC_ROOT\n";
} else {
  print "No \$LOCAL_DOC_ROOT\n";
  if ( $ENV{HOME} ) {
	$doc_root = "$ENV{HOME}${PS}htdocs";
	print "doc_root set to $doc_root using \$HOME\n";
  } else {
	print "No \$HOME\n";
	$doc_root = ( $WinReg ? "c:/htdocs" : "${PS}htdocs" );
	print "doc_root set to default: $doc_root\n";
  }
}

-d $doc_root || mkdir $doc_root, 0775 ||
  die "Cannot create doc_root directory '$doc_root'\n";
-d $doc_root || die "I thought I'd created '$doc_root'!\n";
open( LOG, ">>$doc_root${PS}edit.log" )
  || die "Cannot open log file\n";

eval 'END { print LOG "\n"; close LOG; }';

$SIG{__WARN__} = sub {
  print LOG $_[0];
  warn $_[0];
};

$SIG{__DIE__} = sub {
  my $msg = $_[0];
  chomp $msg;
  print LOG "$msg\n";
  print STDERR "$msg\nHit Enter to continue:";
  my $wait = <STDIN>;
  print STDERR "\n";
  exit(1);
};

print LOG "Running editfile.bat:\n";
print LOG "Args: ", join( " ", @ARGV ), "\n";

foreach my $arg ( @ARGV ) {
  if ( open( IFILE, "<$arg" ) ) {
    print LOG "File $arg:\n";
    my $command = <IFILE>; chomp $command;
    my $URL = <IFILE>; chomp $URL;
    print LOG "Command: $command\n";
    print LOG "URL: $URL\n";
	print "$command $URL\n";
    my $URLpath = $URL;
    $URLpath =~ s|^//|| ||
      die "No leading slashes in URL: '$URL'\n";
    if ( $command eq "Download" ) {
      { my ( @path ) = split( m|/|, $URLpath );
        pop @path;
        my $ppath = $doc_root;
        foreach my $dir ( @path ) {
          $ppath .= "${PS}$dir";
          die "mkdir $ppath failed\n"
            unless ( -d $ppath || mkdir $ppath, 0775 );
        }
      }
      my $dest = "$doc_root$PS$URLpath";
      if ( open( OFILE, ">$dest" ) ) {
        while (<IFILE>) {
          print OFILE;
        }
        close(OFILE);
        print LOG "Ready to edit $dest\n";
        launch_editor( $dest );
      } else {
        die "Unable to open $dest\n";
      }
    } elsif ( $command eq "Upload" ) {
      my $auth = <IFILE>; chomp $auth;
	  $URLpath =~ s|/|$PS|g if $PS ne '/';
      my $src = "$doc_root$PS$URLpath";
      post_update( $URL, $src, $auth );
    } else {
      while (<IFILE>) {
        print LOG "  $_";
      }
    }
    close(IFILE);
    unlink($arg);
  } else {
    die "  Unable to open input arg $arg\n";
  }
}
exit(0);

use HTTP::Request::Common qw(POST);
use LWP::UserAgent;

sub post_update {
  my ( $URL, $src, $auth ) = @_;

  my $ua = new LWP::UserAgent;
  my $req =
    POST $upload_url,
      Content_Type => 'form-data',
      Content =>
        [ 'URL'    => $URL,
          'Auth'   => $auth,
          'Req'    => "Upload2",
          'upload' => [$src]
        ];
  print LOG "Contacting Server: $upload_url\n";
  my $response = $ua->request($req);

  my $content;
  if ( $response->is_success ) {
    $content = $response->content;
  } else {
    die "An error occurred:\n" .
               $response->status_line . "\n";
  }

  print LOG "Result was:\n$content\n";
}

# Looks for
#  HKEY_CLASSES_ROOT/.html for $htmltype, then in
#  HKEY_CLASSES_ROOT/$htmltype/shell/Homesite 3.0/command
#   or
#  HKEY_CLASSES_ROOT/$htmltype/shell/Edit/command
sub get_editor_command_from_registry {
  my ( $file ) = @_;
  my $htmlkey;
  my %htmlvals;

  unless ( $HKEY_CLASSES_ROOT->Open( ".html", $htmlkey )
    && $htmlkey->GetValues(\%htmlvals)
    && defined( $htmlvals{''} ) ) {
    die "Open(.html) failed\n";
  }

  my $htmltype = $htmlvals{''}->[2] || die "html type not defined\n";
  $htmlkey->Close;

  my @keys;
  sub getsubkey {
    my ( $key, $subkey ) = @_;
    my $newkey;
    ${$key}->Open( $subkey, $newkey ) || return 0;
    push( @keys, $newkey );
    $$key = $newkey;
    return 1;
  }

  my $key = $HKEY_CLASSES_ROOT;
  die "Unable to open $htmltype//shell\n"
    unless ( getsubkey( \$key, $htmltype ) &&
             getsubkey( \$key, "shell" ) );

  my $action = '';
  die "Unable to find a suitable editing command for $htmltype\n"
    unless ( getsubkey( \$key, $action = "Homesite 3.0" ) ||
             getsubkey( \$key, $action = "Edit" ) );

  die "No command associated with $htmltype//shell//$action\n"
    unless getsubkey( \$key, "command" );

  $key->GetValues( \%htmlvals ) || die "GetValues failed\n";
  my $command = $htmlvals{''}->[2];

  $command =~ s|^([\w_:\\/\.]+Program Files[\w_\\/\.]+)\s|"$1" |;

  while ( $key = pop(@keys) ) {
    $key->Close;
  }

  $file =~ s|/|\\|g;
  $command =~ s/\%1/$file/;
  print LOG "'$command'\n";
  return "$ENV{COMSPEC} /c start $command";
}

sub launch_editor {
  my ( $file ) = @_;
  my ( $command, $program );
  $command = $ENV{HTML_EDITOR_CMD};
  $program = $ENV{HTML_EDITOR} unless $command;
  $command = get_editor_command_from_registry( $file )
	if ( $WinReg && !( $command || $program ) );
  $command = $ENV{EDITOR_CMD} unless ( $command || $program );
  $program = $ENV{EDITOR} unless ( $command || $program );
  $program = "vi" unless ( $command || $program );
  if ( $command ) {
	$command =~ s/\%s/$file/;
  } else {
	$command = "$program $file";
  }
  system( $command ) &&
	die "Error launching editor '$command'\n";
}
__END__
:endofperl
