# converts .slp format files to .cmd format
# The .slp format looks essentially like this:
#
#  ; This is a comment
#  Command_Set = 'A'
#  proxy etalon {
#    O: Drive Etalon Online
#    _: Drive Etalon Offline
#  }
#
BEGIN {
  command_set="A"
  print "%{"
  print "  #define SERVER_INIT"
  print "  void cis_init(void) {"
}
/^[ \t]*;/ { # comment
  sub("^[ \t]*;", "\t/*")
  print $0 " */"
  next
}
/^[ \t]*[Cc]ommand_[Ss]et/ {
  if (match($0, "'[A-J]'")) command_set=substr($0, RSTART+1, 1)
  next
}
/^[ \t]*[Pp]roxy/ {next}
/^[ \t]*.:/ {
  nprox++
  proxy[nprox]="SOLDRV_PROXY_" command_set ", " nprox;
  sub("^[ \t]*.: *", "\tsol_proxy_init(" proxy[nprox] ", \"")
  sub("[ \t]*$", "");
  print $0 "\\n\");"
  next
}
/^[ \t]*\}/ {next}
{ system("echo >&2 slp2cmd: Syntax Error in file " FILENAME " at line " FNR )
  system("echo >&2 \"--> " $0 "\"")
  syntaxerr=1
  exit 1
}
END {
  if (syntaxerr!=1) {
	print "  }"
	print "  void cis_terminate(void) {"
	for (i = 1; i <= nprox; i++)
	  print "\tSoldrv_reset_proxy(" proxy[i] ");"
	print "  }\n%}"
  }
}
