@rem = '--*-Perl-*--
@echo off
perl -x -S %0 %1 %2 %3 %4 %5 %6 %7 %8 %9
goto endofperl
@rem ';
#! /usr/local/bin/perl
#line 8

#USAGE:
#  cmpxls file1 file2

open(STDOUT, ">test.txt" ) || die "Unable to redirect STDOUT\n";

use Cwd;
my $dir = getcwd;
my ( $file1, $file2 ) = @ARGV;
my $afile1 = "$dir\\$file1";
my $afile2 = "$dir\\$file2";
-r $afile1 || die "Cannot locate '$file1'\n";
-r $afile2 || die "Cannot locate '$file2'\n";

use Win32::OLE;

# Note:  {Value} fails if there are more than 10 columns in
#   the specified range.

#----------------------------------------------------------------
# Open the Excel Spreadsheet and find the main index sheets
#----------------------------------------------------------------
my $ex = Win32::OLE->new('Excel.Application', sub {$_[0]->Quit;})
  || die "Unable to start Excel\n";
$ex->Workbooks->Open( $afile1 )
  || die "Unable to open $afile1\n";
my $nsheets = $ex->Worksheets->{Count};
my @sheets1;
foreach my $sheet ( 1 .. $nsheets ) {
  push( @sheets1, $ex->Worksheets($sheet)->{Name} );
}
$ex->Workbooks->Open( $afile2 )
  || die "Unable to open $afile2\n";
$nsheets = $ex->Worksheets->{Count};
my @sheets2;
foreach my $sheet ( 1 .. $nsheets ) {
  push( @sheets2, $ex->Worksheets($sheet)->{Name} );
}
if ( @sheets1 != @sheets2 ) {
  my $ns1 = scalar(@sheets1);
  my $ns2 = scalar(@sheets2);
  print "$file1 contains $ns1 sheets\n",
        "$file2 contains $ns2 sheets\n";
}
{
my $sheet = 0;
my $errors = 0;
while ( defined($sheets1[$sheet]) || defined($sheets2[$sheet]) ) {
  my $s1 = $sheets1[$sheet] || '';
  my $s2 = $sheets2[$sheet] || '';
  if ( $s1 ne $s2 ) {
    print "sheet $sheet: '$s1'  '$s2'\n" ;
    $errors = 1;
  }
  $sheet++;
}
die "Stopping due to errors\n" if $errors;
}

# Now we can assume all the sheets are named the same, so we
# can go through them one by one and compare the contents.

my @col = qw(A B C D E F G H I);
foreach my $sheet ( @sheets1 ) {
  my $printed = 0;
  $ex->Workbooks(1)->Activate ||
    die "Trouble activating $file1 for sheet $sheet\n";
  my $wsheet = $ex->Worksheets($sheet) || die;
  my $nrows1 = $wsheet->{UsedRange}->Rows->{Count} || die;
  my $data1 = $wsheet->Range("A1:I$nrows1")->{Value} || die;
  $ex->Workbooks(2)->Activate ||
    die "Trouble activating $file2 for sheet $sheet\n";
  $wsheet = $ex->Worksheets($sheet) || die;
  my $nrows2 = $wsheet->{UsedRange}->Rows->{Count} || die;
  my $data2 = $wsheet->Range("A1:E$nrows1")->{Value} || die;
  print "  nrows: $nrows1 $nrows2\n" if ( $nrows1 != $nrows2 );
  my $row = 1;
  while ( @$data1 > 0 && @$data2 > 0 ) {
    my $row1 = shift(@$data1) || [ '', '', '', '', '', '', '', '', '' ];
    my $row2 = shift(@$data2) || [ '', '', '', '', '', '', '', '', '' ];
    foreach my $i ( 0 .. 4 ) {
      unless ( $row1->[$i] eq $row2->[$i] ) {
        print "Sheet '$sheet'\n" unless $printed;
        $printed = 1;
        print "  $col[$i]$row: '$row1->[$i]'  '$row2->[$i]'\n";
      }
    }
    $row++;
  }
}
exit(0);
__END__
:endofperl
