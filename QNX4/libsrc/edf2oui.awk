# edf2oui.awk Converts .edf files to .oui for extra usage
# $Log$
#
# spreadsheet deleteme 6
#  1 O3Ref %6.0lf Ct24_Double
#
/^ *spreadsheet/ {
  if (printed == 0) {
	print "<package> edfoui"
	printed = 1
  }
  print "<unsort>\n<BLANK>"
  print "Spreadsheet " $2 ", " $3 " columns:"
  sps = $2
}
/^[ \t]*[0-9]/ {
  printf "  %-13s[%2d] = %s\n", sps, $1, $2
}
