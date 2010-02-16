#! /usr/bin/perl -w

# For each variable, create a table
# rows are for years, columns months

# include links to yearly run logs

# Start by collecting information about which files exist
# Search for year directories

my @years = sort grep m/^\d{4}$/, <*>;
my %vars;
for my $year ( @years ) {
  my @files = <$year/*.nc>;
  for my $file ( @files ) {
    if ( $file =~ m|^\d{4}/(\w+)\.4xdaily\.(\d{4})-(\d{2})\.nc$| ) {
      my $var = $1;
      my $month = $3+0;
      if ( $year == $2 ) {
	$vars{$var}->{$year}->{$month} = 1;
      } else {
	warn "File in wrong year: '$file'\n";
      }
    } else {
      warn "File does not match pattern: '$file'\n";
    }
  }
}

my @vars = sort keys %vars;
open(INDEX, ">Logs/index.html") || die "Unable to write Logs/index.html\n";
print INDEX <<EOF;
<!DOCTYPE html PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">
<html>
<head>
<link href="4xdaily.css" rel="stylesheet" type="text/css">
<title>4xdaily Retrieval Record</title>
</head>
<body>
<h1>4xdaily Retrieval Record</h1>
EOF

for my $var (@vars) {
  print INDEX "<table class=\"vartab\">\n<tr class=\"top\"><th rowspan=\"2\">Year</th><th colspan=\"12\">$var</th></tr>\n",
    "<tr class=\"top\">",
    map( "<th>$_</th>", qw(J F M A M J J A S O N D) ),
    "</tr>\n";
  for my $year ( @years ) {
    my $yeartext = $year;
    $yeartext = "<a href=\"$year/index.html\">$year</a>" if -f "Logs/$year/index.html";
    print INDEX "<tr><th>$yeartext</th>";
    my $months = $vars{$var}->{$year} || {};
    for ( my $i = 1; $i <= 12; ++$i ) {
      my $def = defined $months->{$i} ? "X" : "";
      my $defclass = $def ? "X" : "O";
      print INDEX "<td class=\"$defclass\">$def</td>";
    }
    print INDEX "</tr>\n";
  }
  print INDEX "</table>\n";
}
print INDEX <<EOF;
</body>
</html>
EOF

close INDEX || warn "Error closing Logs/index.html\n";

