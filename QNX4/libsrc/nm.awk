/^  / {
  for (i = 1; i <= NF; i++) {
    sub("_$", "", $i)
	print "000000  T " $i
  }
}
