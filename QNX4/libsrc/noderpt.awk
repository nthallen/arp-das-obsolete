BEGIN {
  FS=":"
}
/^=+/ {
  nfields=NF
  for ( i = 1; i <= NF; i++ ) width[i] = length($i)
  next
}
/^#/ { next }
{ for (i = 1; i <= NF; i++) if (i <= nfields) {
	format=" %-" width[i] "." width[i] "s"
	printf format, $i
  }
  printf "\n"
}
