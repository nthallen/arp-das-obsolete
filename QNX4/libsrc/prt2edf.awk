function end_sps() {
  if (sps != "" && maxcol > 0) {
	gsub("\372", "", spstext)
	sub("\.sps", "", sps)
	if (maxcol >= width) width = maxcol+1
	print "spreadsheet " sps " " width
	print spstext
	spstext = ""
	sps=""
	maxcol = 0
	width = 0
  }
}

/^\372/ { sub("^\372", "") }
/\.sps$/ {
  end_sps()
  sps = $NF
}
/ > / {
  if ($1 > maxcol) maxcol = $1
  spstext = spstext "\t" $1 " " $2 " " $3 " " $4 "\n"
}
/^ *[Ww]idth[ \t]+[0-9]+$/ { width = $NF; }
END {
  end_sps()
}
