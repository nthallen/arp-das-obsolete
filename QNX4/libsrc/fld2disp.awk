/^#FIELD#/ {
  if ( curfile == "" ) {
	curfile = FILENAME
	console=0
  } else if ( curfile != FILENAME ) {
	curfile = FILENAME
	console++
  }
  name = $NF;
  sub("^\"", "", name)
  sub("\"$", "", name)
  print "display(" console+0 "," $3 "," $4 "," name ");"
  if (console > max_console) max_console = console
}
END { print "%{\n  #define N_CONSOLES " max_console+1 "\n%}\n" }
