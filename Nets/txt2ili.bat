@rem = '--*-Perl-*--
@echo off
perl -x -S %0 %1 %2 %3 %4 %5 %6 %7 %8 %9
goto endofperl
@rem ';
#! /usr/local/bin/perl
#line 8

#USAGE:
#  txt2ili [ project_dir [ Component ... ] ]
#    Component - Limits data extraction to the named components
#                Components listed on the command line supplement
#                those listed in nets.ini. If none are specified
#                in either location, all components are processed.
#    project_dir Defines where nets.ini is found and where the
#                log file and the net subdirectory will be created.
#                If not specified, will look in the Registry or
#                use the current directory if nets.ini is present.

#Reads:
#  xls/cmdtm.txt
#  xls/index.txt
#  library parts (???)
#  xls/master.txt
#  xls/J\d+*.txt

use strict;
use FindBin;
use lib "$FindBin::Bin";
use SIGNAL;
use NETSLIB qw(open_nets find_nets mkdirp);


$SIGNAL::context = "TXT2ILI";

# Eliminate net/ from the search path for the duration.
# shift @NETSLIB::NETSLIB;

#---------------------------------------------------------------
# Resolve directory
#   First look for options on command line:
#---------------------------------------------------------------
my $project_dir = shift @ARGV;
my $from_registry = 0;

unless ( $project_dir ) {
  use Win32::Registry;

  my @keys;
  sub getsubkey {
    my ( $key, $subkey ) = @_;
    my $newkey;
    ${$key}->Open( $subkey, $newkey ) || return 0;
    push( @keys, $newkey );
    $$key = $newkey;
    return 1;
  }
  my $key = $main::HKEY_CURRENT_USER;
  foreach my $subkey ( qw( Software HUARP Nets BaseDir ) ) {
    goto KEY_NOT_FOUND unless getsubkey( \$key, $subkey );
  }
  my %vals;
  $key->GetValues( \%vals ) || die "GetValues failed\n";
  $project_dir = $vals{''}->[2];
  $from_registry = 1 if $project_dir;
  
  KEY_NOT_FOUND:
  while ( $key = pop(@keys) ) {
    $key->Close;
  }
}

use Cwd;
$project_dir =~ s|[/\\]nets.ini$||i;
$project_dir = getcwd if ( ! $project_dir ) && -r "nets.ini";

die "Unable to locate project directory '$project_dir'\n"
  unless $project_dir && -d $project_dir;

unless ( $from_registry ) {
  print "TXT2ILI: Changing to Project Dir $project_dir\n";
  my @keys;
  sub createsubkey {
    my ( $key, $subkey ) = @_;
    my $newkey;
    ${$key}->Create( $subkey, $newkey ) || return 0;
    push( @keys, $newkey );
    $$key = $newkey;
    return 1;
  }
  my $key = $main::HKEY_CURRENT_USER;
  foreach my $subkey ( qw( Software HUARP Nets BaseDir ) ) {
    die "Unable to create registry subkey $subkey\n"
      unless createsubkey( \$key, $subkey );
  }
  $key->SetValue( '', REG_SZ, $project_dir ) ||
    die "SetValue failed\n";
  while ( $key = pop(@keys) ) {
    $key->Close;
  }
}

chdir( $project_dir ) || die "Unable to chdir $project_dir\n";

my $logfile = "txt2ilist";

open( LOGFILE, ">$logfile.err" ) ||
  die "Unable to open log file\n";

$SIG{__WARN__} = sub {
  print LOGFILE @_;
  warn @_;
};

sub LogMsg {
  print LOGFILE @_;
  print STDERR @_;
}

$SIG{__DIE__} = sub {
  warn @_;
  print STDERR "\nHit Enter to continue:";
  my $wait = <STDIN>;
  print STDERR "\n";
  exit(1);
};

END {
  if ( defined $SIG{__WARN__} ) {
	delete $SIG{__WARN__};
	delete $SIG{__DIE__};
	close LOGFILE;
	unlink( "$logfile.bak" );
	rename( "$logfile.err", "$logfile.bak" );
	open( IFILE, "<$logfile.bak" ) ||
	  die "Unable to read $logfile.bak";
	open( OFILE, ">$logfile.err" ) ||
	  die "Unable to rewrite $logfile.err";
	print OFILE
	  map $_->[0],
		sort { $a->[1] cmp $b->[1] || $a->[0] cmp $b->[0] }
		  map { $_ =~ m/:\s+(.*)$/ ? [ $_, $1 ] : [ $_, '' ] } <IFILE>;
	close OFILE;
	close IFILE;
  }
}

LogMsg "TXT2ILI ", join( " ", @ARGV ), "\n";

$| = 1;

# SIGNAL::load_signals();
# My reasoning here is that txt2ili is supposed to build the
# database from scratch, so we don't want to load it.

#---------------------------------------------------------------
# Now we're in the project directory.
#  Read nets.ini if it exists.
#---------------------------------------------------------------
my %drawcomps;
if ( open( IFILE, "<nets.ini" ) ) {
  while (<IFILE>) {
    next if m/^\s*#/;
    if ( m/^(\w+)=(.*)$/ ) {
      $SIGNAL::global{$1} = $2;
    }
  }
  close(IFILE) || warn "Error closing nets.ini\n";
  if ( $SIGNAL::global{Components} ) {
    my ( @comps ) = split( /[,\s]+/, $SIGNAL::global{Components} );
    push( @ARGV, @comps );
    print LOGFILE "TXT2ILI ", join( " ", @ARGV ), "\n";
  }
  if ( $SIGNAL::global{Draw_Components} ) {
    my ( @comps ) = split( /[,\s]+/, $SIGNAL::global{Draw_Components} );
    foreach my $comp ( @comps ) { $drawcomps{$comp} = 1; }
  }
}

#----------------------------------------------------------------
# Itemize selected components:
#  $comp is selected if ( $scomps || $scomps{$comp} )
#  (The default is to select all)
#----------------------------------------------------------------
my %scomps = map { ( $_, 1 ); } @ARGV;
my $scomps = ( @ARGV == 0 );

my $xls_file = $SIGNAL::global{xls_file} ||
  die "xls_file undefined\n";
$xls_file = "$project_dir\\$xls_file"
  unless $xls_file =~ m|^([a-zA-Z]:)?[/\\]|;
-r $xls_file || die "Unable to read xls_file '$xls_file'\n";

use Win32::OLE;

# Note:  {Value} fails if there are more than 10 columns in
#   the specified range.

#----------------------------------------------------------------
# Open the Excel Spreadsheet and find the main index sheets
#----------------------------------------------------------------
my $ex = Win32::OLE->new('Excel.Application', sub {$_[0]->Quit;})
  || die "Unable to start Excel\n";
$ex->Workbooks->Open( $xls_file )
  || die "Unable to open $xls_file\n";
my $nsheets = $ex->Worksheets->{Count};

#----------------------------------------------------------------
# Search for the main sheet names
#----------------------------------------------------------------
my %sheet;
{ my @sheets = qw(cmdtm master component schematic);
  
  map { $sheet{$_} = [] } @sheets;
  SHEET:
  foreach my $sheet ( 1 .. $nsheets ) {
    my $name = $ex->Worksheets($sheet)->{Name};
	my $special = '';
    foreach my $sname ( @sheets ) {
      if ( $name =~ m/$sname/i ) {
		if ( $special && ( $special ne $sname ) ) {
		  die "Sheet $sheet:$name recognized as ",
			  "both $sname and $special\n";
		}
		push( @{$sheet{$sname}}, $sheet );
		$special = $sname;
	  }
    }
    $sheet{$name} = $sheet unless $special;
  }
}

#----------------------------------------------------------------
# First check cmdtm
#----------------------------------------------------------------
my %renamed;
if ( @{$sheet{cmdtm}} ) {
  foreach my $sheet ( @{$sheet{cmdtm}} ) {
	my $wsheet = $ex->Worksheets($sheet) || die;
	local $SIGNAL::context = "xls/$wsheet->{Name}";
	my $nrows = $wsheet->{UsedRange}->Rows->{Count} || die;
	foreach my $rownum ( 1 .. $nrows ) {
	  my $range = $wsheet->Range("A$rownum:O$rownum") || die;
	  my $data = $range->{Value} || die;
	  my $line = $data->[0];
	  my $renamed;
	  map { $_ =~ s/"//g; } @$line;
	  my ( $lineno, $type, $oldname, $desc, @cfg ) = @$line;
	  if ( $lineno =~ /\d+/ && $oldname && ! $oldname =~ /^\s*$/ ) {
		my $name = SIGNAL::signal_from_txt( $oldname );
		$name = SIGNAL::define_sigcase($name);
		if ( $name ne $oldname ) {
		  $range->Cells(1,2)->{Value} = $name; ### Change Excel
		}
		if ( $type =~ m/^\s*(C|DO)\s*$/ ) {
		  my $cmdno = $cfg[0] || '';
		  $desc = "$desc($1:$cmdno)";
		  my $comment = $cfg[2] || '';
		  $renamed = $1 if $comment =~ m/\b(\w+)\s+renamed/;
		} elsif ( $type =~ m/^\s*AI\s*$/ ) {
		  my $config = [ 'AI', @cfg ];
		  $SIGNAL::sigcfg{$name} = $config;
		  my $comment = $cfg[10] || '';
		  $renamed = $1 if $comment =~ m/\b(\w+)\s+renamed/;
		} elsif ( $type =~ m/^\s*DI\s*$/ ) {
		  my $comment = $cfg[2] || '';
		  $renamed = $1 if $comment =~ m/\b(\w+)\s+renamed/;
		} elsif ( $type =~ m/^\s*AO\s*$/ ) {
		  my $comment = $cfg[10] || '';
		  $renamed = $1 if $comment =~ m/\b(\w+)\s+renamed/;
		}
		SIGNAL::define_sigdesc( $name, $desc );
		if ( $renamed ) {
		  if ( $renamed{"\U$renamed"} ) {
			warn "$SIGNAL::context: Rename conflict on '$renamed' between new ",
				  "'$name' and '", $renamed{"\U$renamed"}, "'\n";
		  } else {
			$renamed{"\U$renamed"} = $name;
		  }
		}
	  }
	}
  }
} else {
  warn "Found no cmdtm sheet\n";
}

#----------------------------------------------------------------
# Next read component types
#----------------------------------------------------------------
if ( @{$sheet{component}} ) {
  foreach my $sheet ( @{$sheet{component}} ) {
	my $wsheet = $ex->Worksheets($sheet) || die;
	local $SIGNAL::context = "xls/$wsheet->{Name}";
	my $nrows = $wsheet->{UsedRange}->Rows->{Count} || die;
	my $data = $wsheet->Range("A1:D$nrows")->{Value} || die;
	foreach my $line ( @$data ) {
	  map { s/"//g; } @$line;
	  my ( $comp, $comptype, $desc, $base ) = @$line;
	  next if $comp =~ /(^\s*#)|(COMP\s+NAME)/i;
	  $comp =~ s/^\s*((\S(.*\S)?)?)\s*$/$1/;
	  next if $comp =~ m/^$/;
	  $comptype =~ s/^\s*(.*)\s*$/$1/;
	  $comptype = $comp unless $comptype;
	  if ( $scomps || $scomps{$comp} ) {
		SIGNAL::define_comptype( $comptype, "" );
		SIGNAL::define_comp( $comp, $comptype );
		SIGNAL::define_compdesc( $comp, $desc );
		$SIGNAL::comp{$comp}->{base} = $base
		  if defined($base) && $base ne '';
	  }
	}
  }
} else {
  warn "Warning: Unable to locate component type sheet\n";
}

#----------------------------------------------------------------
# Handle any library parts:
#  Library parts have:
#    sym/<comptype>/SIGNALS defining signal:desc
#    sym/<comptype>/PARTS in the usual format
#    sym/<comptype>/NETLIST (etc.)
#  Specifically, I'll say a part is a library part if SIGNALS is
#  present.
#----------------------------------------------------------------
my %is_lib_type;
foreach my $comptype ( keys %SIGNAL::comptype ) {
  local $SIGNAL::context = "sym/$comptype/SIGNALS";
  if ( open_nets( *IFILE{FILEHANDLE}, "$SIGNAL::context" ) ) {
    $is_lib_type{$comptype} = 1;
    while ( <IFILE> ) {
      if ( m/^([^:]+)(:([^:]*))?$/ ) {
        my $signal = $1;
        my $desc = $3 || "";
        SIGNAL::define_sigcase( $signal );
        if ( $desc && ! $SIGNAL::sigdesc{$signal} ) {
          SIGNAL::define_sigdesc( $signal, $desc );
        }
      }
    }
    close IFILE || warn "$SIGNAL::context: Error closing\n";
  }
  $SIGNAL::context = "sym/$comptype/PARTS";
  if ( open_nets( *IFILE{FILEHANDLE}, "$SIGNAL::context" ) ) {
    $is_lib_type{$comptype} = 1;
    while ( <IFILE> ) {
      if ( m/^TITLE:(.*)$/ ) {
        SIGNAL::define_comptype( $comptype, $1 ) if $1;
      } elsif ( m/^([^:]+):([^:]+):(.*)$/ ) {
        my ( $conn, $pkg, $desc ) = ( $1, $2, $3 || "" );
        SIGNAL::define_ctconn( $comptype, $conn, $pkg, $desc );
      }
    }
    close IFILE || warn "$SIGNAL::context: Error closing\n";
  }
}

my %CableDefs;
my %CableSheets;
if ( @{$sheet{master}} ) {
  foreach my $sheet ( @{$sheet{master}} ) {
	my $wsheet = $ex->Worksheets($sheet) || die;
	local $SIGNAL::context = "xls/$wsheet->{Name}";
	my $nrows = $wsheet->{UsedRange}->Rows->{Count} || die;
	my $data = $wsheet->Range("A1:E$nrows")->{Value} || die;
	foreach my $line ( @$data ) {
	  map { s/"//g; } @$line;
	  my ( $conncomp, $desc, $conntype, $locname, $comment ) = @$line;
	  next if $conncomp =~ m/^conn/i;
	  next unless $conncomp =~ m/^(\w+)(-C\w*)?$/;
	  local $SIGNAL::context = "$SIGNAL::context:$conncomp";
	  my $cablesheet = $2 ? $conncomp : '';
	  $conncomp = $1;

	  #----------------------------------------------------------------
	  # Process "alias" column
	  # Where segment name(s) are specified, add segment name to
	  # the global name, and hence to the cable name...
	  # WAIT!
	  my $alias;
	  my $feedthrough;
	  if ( $locname ) {
		if ( $locname =~ m/^([(]([A-Z]+)(=([A-Z]+))?[)])?(\w+)?(=(\w+))?$/ ) {
		  my ( $seg1, $seg2, $ln, $ft ) = ( $2, $4, $5, $7 );
		  my $gname = $conncomp;
		  if ( $ln ) {
			$alias = $conncomp;
			$conncomp = $ln;
		  }
		  $feedthrough = $ft if $ft;
		  if ( $seg1 ) {
			my ( $aconn, $acomp ) = SIGNAL::split_conncomp( $gname );
			next unless $aconn;
			$alias = SIGNAL::make_conncomp( "${aconn}_$seg1", $acomp );
			if ( $seg2 ) {
			  $feedthrough =
				SIGNAL::make_conncomp( "${aconn}_$seg2", $acomp );
			}
		  }
		  $conncomp = $SIGNAL::conlocname{$alias} if
			$SIGNAL::conlocname{$alias};
		} else {
		  warn "$SIGNAL::context:$conncomp: Could not parse alias: ",
			  "'$locname'\n";
		}
	  }
	  # If it's a cabledef, save it.
	  if ( $cablesheet ) {
		my $gname = $alias || $conncomp;
		my $cable = SIGNAL::get_cable_name( $gname );
		$CableDefs{$cable} = [] unless $CableDefs{$cable};
		push( @{$CableDefs{$cable}}, $cablesheet );
		$CableSheets{$gname} = $cablesheet;
	  }

	  my ( $conn, $comp ) = SIGNAL::split_conncomp( $conncomp );
	  if ( $conn ) {
		next unless ( $scomps || $scomps{$comp} ); # kluge for speed
		$conncomp = SIGNAL::make_conncomp( $conn, $comp ); # make it canonical

		$conntype =~ s/\s+to\s.*$// if $cablesheet;
		$conntype = '' unless $conntype;
		$conntype =~ s/^\s*//;
		$conntype =~ s/\s*$//;
		unless ( $conntype ) {
		  warn "$SIGNAL::context: No connector type for $conncomp\n";
		  next;
		}

		if ( $SIGNAL::comp{$comp} && $SIGNAL::comp{$comp}->{type}) {
		  my $comptype = $SIGNAL::comp{$comp}->{type};
		  if ( $is_lib_type{$comptype} ) {
			my $compdef = $SIGNAL::comptype{$comptype};
			unless ( defined $compdef->{conn}->{$conn} ) {
			  warn "$SIGNAL::context: Undefined Connector $conncomp for ",
				"library comptype $comptype\n";
			}
		  }
		}
		$alias = $conncomp unless $alias;
		SIGNAL::define_conncomp( $conncomp, $conntype, $desc, $alias );

		if ( $feedthrough ) {
		  my ( $ftconn, $ftcomp ) =
			SIGNAL::split_conncomp( $feedthrough );
		  $feedthrough = SIGNAL::make_conncomp( $ftconn, $ftcomp );
		  my $lfdthru = $SIGNAL::conlocname{$feedthrough} ||
						$feedthrough;
		  ( $ftconn, $ftcomp ) = SIGNAL::split_conncomp( $lfdthru );
		  if ( $ftconn ) {
			if ( $ftcomp eq $comp ) {
			  SIGNAL::define_conncomp( $lfdthru, $conntype,
						$desc, $feedthrough );
			  my $comptype = $SIGNAL::comp{$comp}->{type} || die;
			  my $ct = $SIGNAL::comptype{$comptype} || die;
			  my $group;
			  if ( $ct->{conn}->{$ftconn}->{fdthr} ) {
				$group = $ct->{conn}->{$ftconn}->{fdthr};
			  } else {
				$group = $conn;
				$ct->{conn}->{$ftconn}->{fdthr} = $group;
			  }
			  $ct->{conn}->{$conn}->{fdthr} = $group;
			  $ct->{fdthr}->{$group} = {}
				unless $ct->{fdthr}->{$group};
			  $ct->{fdthr}->{$group}->{$conn} = 1;
			  $ct->{fdthr}->{$group}->{$ftconn} = 1;
			} else {
			  warn "$SIGNAL::context:$conncomp: ",
				"Feedthrough on different comp! '$feedthrough'\n";
			}
		  } else {
			warn "$SIGNAL::context:$conncomp: Feedthrough corrupted: ",
				  "'$feedthrough'\n";
		  }
		}
	  } else {
		if ( $conncomp =~ m/^J\d+$/ ) {
		  warn "$SIGNAL::context: ",
			"Missing Board Designation for Connector $conncomp\n";
		} else {
		  warn "$SIGNAL::context: Connector name out of spec: \"$conncomp\"\n";
		}
	  }
	}
  }
} else {
  warn "$SIGNAL::context: Unable to locate 'master' sheet(s).\n";
}

#----------------------------------------------------------------
# Prepare to handle renaming:
#----------------------------------------------------------------
foreach my $name ( keys %renamed ) {
  if ( $renamed{"\U$renamed{$name}"} ) {
	warn "$SIGNAL::context: Potential renaming chain: $name -> $renamed{$name} ->",
		$renamed{"\U$renamed{$name}"}, "\n";
  }
}
my $rename_pattern = '^(' . join( '|', keys %renamed ) . ')(_\w+)?$';

#----------------------------------------------------------------
# How do we handle library parts?
# What are the options:
#  A. component/type is specified entirely by conn listings
#      Build NETLIST from listings
#      Write out NETLIST,Symbols,Descriptions(PARTS)
#  B. Conn desc. defined in listings but NETLIST exists
#      Load NETLIST, check against listings
#----------------------------------------------------------------
foreach my $comptype ( keys %SIGNAL::comptype ) {
  local $SIGNAL::context = "comptype/$comptype";
  my ( %coCT, %sgCT );
  my %links;
  my %pinlist;
  my $sawonecomp = 0;
  foreach my $comp ( @{$SIGNAL::comptype{$comptype}->{'comps'}} ) {
	my ( %coC, %sgC );
	foreach my $conn ( @{$SIGNAL::comptype{$comptype}->{'conns'}} ) {
	  $pinlist{$conn} = [] unless $pinlist{$conn};
	  my $galias = SIGNAL::get_global_alias(
		SIGNAL::make_conncomp( $conn, $comp ) );
	  my $sheet = $galias;
	  if ( $sheet =~ m/_.+_/ ) {
		my ( $sconn, $scomp ) = SIGNAL::split_conncomp( $sheet );
		$sconn =~ s/_.*$//;
		$sheet = SIGNAL::make_conncomp( $sconn, $scomp );
	  }
      local $SIGNAL::context = "xls/$sheet";
	  if ( $SIGNAL::comptype{$comptype}->{conn}->{$conn}->{fdthr} ) {
		if ( $sheet{$sheet} ) {
		  warn "$SIGNAL::context: Listing ignored for feedthrough\n";
		}
		next;
	  }
      if ( $sheet{$sheet} ) {
		my %lnk;
		load_xls_jfile( $ex, $sheet, $comp, $conn, \%coC, \%sgC,
				\%lnk, $pinlist{$conn} );
		foreach my $pin ( keys %lnk ) {
		  local $SIGNAL::context = "$SIGNAL::context:$pin";
		  my $link = make_local_link( $comp, $lnk{$pin} || '' );
		  if ( defined $links{"$conn.$pin"} ) {
			my $olink = $links{"$conn.$pin"};
            warn "$SIGNAL::context: ",
                 "Link changed from \"$olink\" to \"$link\"\n"
              if $olink ne $link;
		  }
		  $links{"$conn.$pin"} = $link;
		}
      } elsif ( $CableSheets{$galias} ) {
		my %lnk;
		$sheet = $CableSheets{$galias};
		if ( $sheet{$sheet} ) {
		  local $SIGNAL::context = "xls/$sheet";
		  load_xls_jfile( $ex, $sheet, $comp, $conn,
				  \%coC, \%sgC, \%lnk, $pinlist{$conn} );
		}
	  }
    }
	if ( $sawonecomp ) {
	  my @renames =
		compare_netlists( $comp, \%coC, \%sgC, "earlier defs", \%coCT, \%sgCT, 1 );
	  write_transfile( "net/comp/$comp", "NETLIST.NDC", @renames );
	} else {
	  %coCT = %coC;
	  %sgCT = %sgC;
	  $sawonecomp = 1;
	}
  }
  
  # Save pin defs
  foreach my $conn ( @{$SIGNAL::comptype{$comptype}->{'conns'}} ) {
    local $SIGNAL::context = "$comptype:$conn";
    my $conntype =
      $SIGNAL::comptype{$comptype}->{conn}->{$conn}->{type};
	SIGNAL::define_pins( $conntype, $pinlist{$conn} );
  }
  
  #----------------------------------------------------------------
  # now handle feedthroughs
  #----------------------------------------------------------------
  { my $ct = $SIGNAL::comptype{$comptype};
	my $fdthr = $ct->{fdthr} || {};
	foreach my $group ( keys %{$fdthr} ) {
	  my @conns = keys %{$fdthr->{$group}};
	  my $type;
	  foreach my $conn ( @conns ) {
		if ( $ct->{conn}->{$conn}->{type} ) {
		  $type = $ct->{conn}->{$conn}->{type};
		} else {
		  warn "$SIGNAL::context: Fdthr conn undefined '$conn'\n";
		}
	  }
	  if ( $type ) {
		foreach my $conn ( @conns ) {
		  my $ctype = $ct->{conn}->{$conn}->{type} || $type;
		  if ( $ctype ne $type ) {
			warn "$SIGNAL::context:$group: Fdthr type clash: ",
			  "'$type', '$ctype'\n";
		  }
		}
		my @pins;
		SIGNAL::define_pins( $type, \@pins );
		my $c0 = $conns[0];
		foreach my $pin ( @pins ) {
		  foreach my $conn ( @conns ) {
			SIGNAL::define_pinsig( $conn, $pin,
				"\$FT_${c0}_$pin", \%coCT, \%sgCT );
		  }
		}
	  }
	}
  }
  
  #----------------------------------------------------------------
  # Load NETLIST if present
  #----------------------------------------------------------------
  my $has_netlist;
  { my ( %coNL, %sgNL );
	$has_netlist =
        SIGNAL::load_netlist( $comptype, '', \%coNL, \%sgNL );
	if ( $has_netlist ) {
	  my @renames =
		compare_netlists( "conn listings", \%coCT, \%sgCT,
						  "NETLIST", \%coNL, \%sgNL, 0 );
	  write_transfile( "net/sym/$comptype", "NETLIST.NDC2", @renames );
	  %coCT = %coNL;
	  %sgCT = %sgNL;
	}
  }
  foreach my $conn ( @{$SIGNAL::comptype{$comptype}->{conns}} ) {
	local $SIGNAL::context = "xls/$conn:$comptype";
    my %pins;
	my @pins;
	my $pins;
    my $conntype =
      $SIGNAL::comptype{$comptype}->{conn}->{$conn}->{type};
	SIGNAL::define_pins( $conntype, \@pins );
	foreach my $pin ( @pins ) { $pins{$pin} = 1; }
	if ( $coCT{$conn} ) {
	  $pins = $coCT{$conn};
	  foreach my $pin ( keys %$pins ) {
		warn "$SIGNAL::context: Listed pin not found ",
			 "in pkgtype $conntype pin \"$pin\"\n"
		  unless $pins{$pin};
	  }
	} else {
	  warn "$SIGNAL::context: No listing found for (local) $conn$comptype\n";
	}
	# generate .list file
	$SIGNAL::context = "net/sym/$comptype/$conn.list";
	mkdirp( "net/sym/$comptype" );
	open( OFILE, ">$SIGNAL::context" ) ||
	  die "Unable to write to $SIGNAL::context\n";
	foreach my $pin ( @pins ) {
	  my $signal = ( defined $pins && defined $pins->{$pin} ) ?
					$pins->{$pin} : "";
	  my $link = $links{"$conn.$pin"} || "";
	  print OFILE "$pin:$signal:$link\n";
	}
	close OFILE || warn "$SIGNAL::context: Error on close\n";
  }

  # generate NETLIST (but only if one doesn't already exist).
  unless ( $has_netlist ) {
    local $SIGNAL::context = "net/sym/$comptype/NETLIST";
    mkdirp( "net/sym/$comptype" );
    open( OFILE, ">$SIGNAL::context" ) ||
      die "Unable to write to $SIGNAL::context\n";
    print OFILE "*PADS-PCB*\n\n*CLUSTER* ITEM\n\n\n*PART*\n";
    my $ct = $SIGNAL::comptype{$comptype};
    my @conns = @{$ct->{'conns'}};
    foreach my $conn ( @conns ) {
      my $pkgtype = $ct->{'conn'}->{$conn}->{'type'};
      print OFILE "$conn $comptype$conn\@$pkgtype\n";
    }
    print OFILE "\n*NET*\n";
    foreach my $signal ( sort keys %sgCT ) {
      print OFILE "*SIGNAL* $signal 12 0 0 0 -2\n";
      my $pins = $sgCT{$signal};
      my $col = 0;
      foreach my $pin ( sort keys %$pins ) {
        my $len = length($pin);
        if ( $col > 0 ) {
          if ( $col + $len + 1 > 70 ) {
            print OFILE "\n";
            $col = 0;
          } else {
            print OFILE " ";
            $col++;
          }
        }
        print OFILE $pin;
        $col += $len;
      }
      print OFILE "\n";
    }
    print OFILE "\n*END*\n";
    close OFILE || warn "$SIGNAL::context: Error closing\n";
  }
}

#----------------------------------------------------------------
# Process Cable Definitions
#----------------------------------------------------------------
foreach my $cable ( keys %CableDefs ) {
  local $SIGNAL::context = "CableDef:$cable";
  unless ( defined $SIGNAL::cable{$cable} ) {
	warn "$SIGNAL::context: (internal) No cable defined\n";
	next;
  }
  if ( find_nets( "cable/$cable/NETLIST" ) ) {
	warn "$SIGNAL::context: Found NETLIST - ",
	  "Ignoring cable definition(s)\n";
	next;
  }
  mkdirp( "net/cable/$cable" );
  $SIGNAL::context = "net/cable/$cable/NETLIST";
  open( OFILE, ">$SIGNAL::context" ) ||
	die "Unable to write to $SIGNAL::context\n";
  print OFILE "*PADS-PCB*\n\n*CLUSTER* ITEM\n\n\n*PART*\n";
  # identify the connectors involved and output their type
  my %defined;
  foreach my $conncomp ( @{$SIGNAL::cable{$cable}} ) {
	my ( $conn, $comp ) = SIGNAL::split_conncomp($conncomp);
	my $comptype = $SIGNAL::comp{$comp}->{type} || die;
	my $alias = $SIGNAL::comp{$comp}->{alias}->{$conn} || "$conn$comp";
	$defined{$alias} = 1;
	my $pkgtype = $SIGNAL::comptype{$comptype}->{conn}->{$conn}->{type};
	print OFILE "$alias $comptype$conn\@$pkgtype\n";
  }
  print OFILE "\n*NET*\n";
  
  my %sig;
  my %pin;
  my $cablesegment = ( $cable =~ m/(_\w+)$/ ) ? $1 : '';
  foreach my $sheet ( @{$CableDefs{$cable}} ) {
	# my %pins;
	my ( %sgL, %coL, %lnk );
	my @pins;
  
	local $SIGNAL::context = "xls/$sheet";
	unless ( $sheet{$sheet} ) {
	  warn "$SIGNAL::context: No definition found for $sheet\n";
	  next;
	}
	$sheet =~ m/^(\w+)-C\w*$/ || die;
	my $conncomp = $1;
	if ( $cablesegment ) {
	  my ( $conn, $comp ) = SIGNAL::split_conncomp( $conncomp );
	  if ( $conn =~ /_/ ) {
		warn "$SIGNAL::context: Unexpected segment in sheet name\n";
		next;
	  }
	  $conncomp = SIGNAL::make_conncomp(
		  "$conn$cablesegment", $comp );
	}

	# load_xls_jfile( $ex, $sheet, '', \%pins, \@pins );
	load_xls_jfile( $ex, $sheet, '', '',
		  \%coL, \%sgL, \%lnk, \@pins );
	# can discard the %coL, %sgL stuff. All we want is the %lnk
	foreach my $pin ( @pins ) {
	  # my $links = $pins{$pin}->{link} || '';
	  my $links = $lnk{$pin} || '';
	  my @links;
	  foreach my $link ( split /[,\s]\s*/, $links ) {
		if ( $link =~ m/^(\w+)[-.](\w+)$/ ) {
		  my ( $lconncomp, $lpin ) = ( $1, $2 );
		  if ( $cablesegment ) {
			my ( $lconn, $lcomp ) =
			  SIGNAL::split_conncomp( $lconncomp );
			unless ( $lconn ) {
			  warn "$SIGNAL::context:$pin: ",
				"Unrecognized link '$link'\n";
			  next;
			}
			$lconncomp =
			  SIGNAL::make_conncomp( "$lconn$cablesegment", $lcomp );
		  }
		  if ( $defined{$lconncomp} ) {
			push( @links, "$lconncomp.$lpin" );
		  } else {
			warn "$SIGNAL::context:$pin: Undefined conn in Link $link\n"
		  }
		} elsif ( $link !~ m/^BLANK|N.?C.?|PAD$/ ) {
		  warn "$SIGNAL::context:$pin: unrecognized link: '$link'\n"
		}
	  }
	  if ( @links > 0 ) {
		unshift( @links, "$conncomp.$pin" );
		my $newsig = "\$${conncomp}_$pin";
		$sig{$newsig} = [];
		foreach my $pin ( @links ) {
		  if ( defined $pin{$pin} ) {
			if ( $pin{$pin} ne $newsig ) {
			  my $othersig = $pin{$pin};
			  my $otherlist = $sig{$othersig};
			  foreach my $p ( @{$sig{$newsig}} ) {
				$pin{$p} = $othersig;
				push( @$otherlist, $p );
			  }
			  delete $sig{$newsig};
			  $newsig = $othersig;
			} # else redundant
		  } else {
			push( @{$sig{$newsig}}, $pin );
			$pin{$pin} = $newsig;
		  }
		}
	  }
	}
  }
  $SIGNAL::context = "net/cable/$cable/NETLIST";
  foreach my $sig ( keys %sig ) {
	print OFILE "*SIGNAL* $sig 12 0 0 0 -2\n";
	my $col = 0;
	foreach my $link ( @{$sig{$sig}} ) {
	  my $len = length($link);
	  if ( $col > 0 ) {
		if ( $col + $len + 1 > 70 ) {
		  print OFILE "\n";
		  $col = 0;
		} else {
		  print OFILE " ";
		  $col++;
		}
	  }
	  print OFILE $link;
	  $col += $len;
	}
	print OFILE "\n";
  }
  print OFILE "\n*END*\n";
  close OFILE || warn "$SIGNAL::context: Error closing\n";
}

# Load xls/schematic to get schematic designations
if ( @{$sheet{schematic}} ) {
  foreach my $sheet ( @{$sheet{schematic}} ) {
	my $wsheet = $ex->Worksheets($sheet) || die;
	local $SIGNAL::context = "xls/$wsheet->{Name}";
	my $nrows = $wsheet->{UsedRange}->Rows->{Count} || die;
	my $data = $wsheet->Range("A1:C$nrows")->{Value} || die;
	foreach my $line ( @$data ) {
	  my ( $lineno, $sch, $conncomp ) = @$line;
	  if ( $lineno =~ /^\d+$/ ) {
		if ( $conncomp =~ m/^(\w+):(\w+)$/ ||
			$conncomp =~ m/^(J\d+)(\D.*)$/ ) {
		  my ( $conn, $comp ) = ( $1, $2 );
		  next unless ( $scomps || $scomps{$comp} );

		  unless( $SIGNAL::comp{$comp} &&
				  $SIGNAL::comp{$comp}->{type} ) {
			warn "$SIGNAL::context: Component undefined: $comp\n";
		  }
		  my $comptype = $SIGNAL::comp{$comp}->{type};
		  my ( $ct, $conndef );
		  if ( ( $ct = $SIGNAL::comptype{$comptype} ) &&
			   $ct->{conn} &&
			   ( $conndef = $ct->{conn}->{$conn} ) ) {
			if ( $conndef->{schematic} &&
				 $conndef->{schematic} ne $sch ) {
			  warn "$SIGNAL::context: Schematic redefined for $conncomp\n";
			}
			$conndef->{schematic} = $sch;
		  } else {
			warn "$SIGNAL::context: Comptype/conn undefined: $comptype/$conn\n";
		  }
		} elsif ( $conncomp =~
				m/^\s*(\w+)\s+Generated\s+Buf(fer)?s:\s*(.*)$/ ) {
		  my ( $comp, $conns ) = ( $1, $3 );
		  if ( $SIGNAL::comp{$comp} &&
				  $SIGNAL::comp{$comp}->{type} ) {
			my $comptype = $SIGNAL::comp{$comp}->{type};
			if ( $SIGNAL::comptype{$comptype} ) {
			  $SIGNAL::comptype{$comptype}->{bufsch} = $sch;
			  $SIGNAL::comptype{$comptype}->{bufconns} = $conns;
			} else {
			  warn "$SIGNAL::context: Comptype for GenBufs undefined: '$comptype'\n";
			}
		  } else {
			warn "$SIGNAL::context: Component for GenBufs undefined: $comp\n";
		  }
		}
	  }
	}
  }
} else {
  warn "$SIGNAL::context: unable to locate schematic sheet(s)\n";
}

SIGNAL::save_signals;

# load_xls_jfile() reads into standard netlist
# $ex: excel spreadsheet object
# $sheet: sheet name for connector listing (ends in -C for cable)
# $comp, $conn: as usual
# $co, $sg: Standard netlist hash refs
# $lnk: hash ref => { <pin> => <link> }
# $order: array ref set to pin names in the order read
sub load_xls_jfile {
  my ( $ex, $sheet, $comp, $conn, $co, $sg, $lnk, $order ) = @_;
  my $wsheet = $ex->Worksheets($sheet{$sheet}) ||
	die "opening worksheet '$sheet'";
  my $nrows = $wsheet->{UsedRange}->Rows->{Count} || die;
  my @pins;
  foreach my $rownum ( 1 .. $nrows ) {
	my $range = $wsheet->Range("A$rownum:D$rownum") || die;
	my $line = $range->{Value}->[0];
    map { s/"//g; } @$line; # get rid of embedded quotes
    my ( $pin, $tsignal, $link, $comment ) = @$line;
    $pin =~ s/^\s*//;
	$pin =~ s/\s*$//; # leading or trailing whitespace
    $pin =~ s/\.$//; # trailing '.' in pin name
    next if $pin eq "PIN" || $pin eq "";
    local $SIGNAL::context = "$SIGNAL::context:$pin";

	my $signal = $tsignal;
	if ( $signal =~ s/(\(.+\))// ) {
	  warn "$SIGNAL::context: Comment removed from signal: '$1'\n";
	  $comment = $comment ? "$comment $1" : $1;
	  $range->Cells(1,4)->{Value} = $comment;
	}
    $signal = SIGNAL::signal_from_txt($signal);
    $signal = SIGNAL::define_sigcase($signal) if $signal;
	if ( $signal =~ s/$rename_pattern/$renamed{"\U$1"}.$2/ieo ) {
	  warn "$SIGNAL::context: Renamed $1$2 to $signal\n";
	}
	if ( $signal ne $tsignal ) {
	  $range->Cells(1,2)->{Value} = $signal;
	}
    if ( $signal eq "" && $sheet !~ m/-C/ ) {
      $link = "PAD" unless $link;
      warn "$SIGNAL::context: Bogus link-to without signal\n"
        unless ! $drawcomps{$comp} ||
          $link =~ /\bPAD\b|\bE\d+\b|N\.?C\.?/;
    }
    if ( defined $lnk->{$pin} ) {
      warn "$SIGNAL::context: Pin listed more than once\n";
    } else {
	  push( @pins, $pin ) if defined $order;
    }
	SIGNAL::define_pinsig( $conn, $pin, $signal, $co, $sg );
	$lnk->{$pin} = $link;
  }
  if ( @$order > 0 ) {
	my ( $onpins, $npins ) = ( scalar(@$order), scalar(@pins) );
	if ( $onpins != $npins ) {
	  warn "$SIGNAL::context: n_pins differs: ",
			"was $onpins, now $npins\n";
	}
	my $mpins = $onpins < $npins ? $onpins : $npins;
	foreach my $i ( 0 .. $mpins-1 ) {
	  if ( $order->[$i] ne $pins[$i] ) {
		warn "$SIGNAL::context: pin name differs: ",
			 "'$order->[$i]' != '$pins[$i]'\n";
	  }
	}
  }
  @$order = @pins;
}

# Translates a list of global links to local links
# Supports \w+[-.]\w+ (translates to '.' notation)
# Also E\d+, PAD and NC. Comma or space-separated list
# Since these are for on-board link-tos, can skip the
# componenet suffix, but should check to make sure the
# global name maps to this component.
sub make_local_link {
  my ( $comp, $link ) = @_;
  my @links = split( /\s*[,\s]\s*/, $link );
  map {
	unless ( m/^(PAD|N\.?C\.?|E\d+|FANOUT)$/ ) {
	  if ( m/^(\w+)[-.](\w+)$/ ) {
		my ( $galias, $pin ) = ( $1, $2 );
		$galias = "$galias$comp" if $galias =~ m/^J\d+$/;
		my $conncomp = $SIGNAL::conlocname{$galias} || $galias;
		my ( $lconn, $lcomp ) = SIGNAL::split_conncomp( $conncomp );
		if ( $lconn && $lcomp ) {
		  warn "$SIGNAL::context: ",
			"Illegal off-board link to '$galias.$pin'\n"
			if $lcomp ne $comp;
		  $_ = "$lconn.$pin";
		} else {
		  # warn "$SIGNAL::context: Unrecognized link element: '$_'\n";
		}
	  } else {
		# warn "$SIGNAL::context: Unrecognized link element: '$_'\n";
	  }
	}
	$_;
  } @links;
  join ",", @links;
}

# @renames is an array of array refs, each containing
# [ <from>, <to> ]. These get transposed in the NDC
# output format.
sub write_transfile {
  my ( $dir, $file, @renames ) = @_;
  if ( @renames > 0 ) {
	local $SIGNAL::context = "$dir/$file";
	mkdirp($dir);
	open( OFILE, ">$SIGNAL::context" ) ||
	  die "$SIGNAL::context: Unable to write file\n";
	map { print OFILE "$_->[1] $_->[0]\n"; }
	  sort { $a->[0] cmp $b->[0] } @renames;
	close OFILE || warn "$SIGNAL::context: Error closing file\n";
	warn "$SIGNAL::context: Wrote component translation file\n";
  }
}

#----------------------------------------------------------------
# compare_netlists()
#  Takes two netlists and compares them.
#  B is considered the more complete NETLIST.
#  If $useBnames is specified, nets in B are renamed as necessary
#  to match those in A. Otherwise renames that would be required
#  in A are simply recorded. In either case, an array of array refs
#  containing [ <from>, <to> ] pairs is returned.
#  $useBnames determines whether to use A's names or B's names.
#  It is an error if a net in one netlist maps to more than one
#  net in the other netlist.
#  Warning generated if a pin/sig in A is not present in B
#  (it's expected the other way)
#----------------------------------------------------------------
sub compare_netlists {
  my ( $Aname, $coA, $sgA, $Bname, $coB, $sgB, $useBnames ) = @_;
  my @renames = cmp_nets( $Aname, $coA, $sgA, $Bname, $coB, $sgB, 1, $useBnames );
  cmp_nets( $Bname, $coB, $sgB, $Aname, $coA, $sgA, 0 );
  unless ( $useBnames ) {
	foreach ( @renames ) {
	  my ( $from, $to ) = @$_;
	  $sgB->{$to} = $sgB->{$from};
	  delete $sgB->{$from};
	  foreach my $connpin ( keys %{$sgB->{$to}} ) {
		my ( $conn, $pin ) = split( /[.]/, $connpin );
		$coB->{$conn}->{$pin} = $to;
	  }
	}
  }
  @renames;
}

sub cmp_nets {
  my ( $Aname, $coA, $sgA, $Bname, $coB, $sgB, $Bcomplete, $useBnames ) = @_;
  my @renames;
  my %warn;
  foreach my $Asig ( keys %$sgA ) {
	my %Bsig;
	foreach my $connpin ( keys %{$sgA->{$Asig}} ) {
	  my ( $conn, $pin ) = split( /[.]/, $connpin );
	  if ( $coB->{$conn} ) {
		if ( $coB->{$conn}->{$pin} ) {
		  $Bsig{$coB->{$conn}->{$pin}} = $connpin;
		} elsif ( $Bcomplete ) {
		  warn "$SIGNAL::context:$conn.$pin: Pin missing in $Bname\n";
		}
	  } elsif ( $Bcomplete ) {
		unless ( $warn{$conn} ) {
		  warn "$SIGNAL::context:$conn: Connector absent in $Bname\n";
		  $warn{$conn} =1;
		}
	  }
	}
	if ( scalar(keys %Bsig) > 1 ) {
	  warn "$SIGNAL::context:$Asig: ",
		"Error: Signal maps to multiple nets in $Bname: ",
		# join( ", ", map( "$_:$Bsig{$_}", keys %Bsig )), "\n";
		join( ", ",
		  map {
			my $pins = join(' ', keys %{$sgB->{$_}});
			"$_:$pins";
		  } keys %Bsig
		), "\n";
	} elsif ( scalar(keys %Bsig) == 1 ) {
	  my ( $Bsig ) = keys %Bsig;
	  if ( $Bsig ne $Asig ) {
		if ( $sgB->{$Asig} ) {
		  warn "$SIGNAL::context:$Bsig: ",
			"Cannot rename $Bname signal to '$Asig'\n";
		} elsif ( $Bcomplete ) {
		  push( @renames, [ $Bsig, $Asig ] );
		  warn "$SIGNAL::context:$Bsig: ",
			"Signal on $Bname renamed to '$Asig'\n"
			  unless $useBnames;
		} else {
		  #warn "$SIGNAL::context:$Bsig: ",
		  #  "Internal: unexpected rename with '$Asig'\n";
		}
	  }
	}
  }
  @renames;
}

__END__
:endofperl
