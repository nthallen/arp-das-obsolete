# This requires:
#   dccc.por   which defines the boards/ports
#   Your .prt file
#   awk -f prt2dccc.awk h2odig.prt dccc.por h2odig.prt
# $Log$
# Revision 1.1  1993/05/28 20:06:15  nort
# Initial revision
#
# Revision 1.2  1992/10/09  18:08:35  nort
# An Update
#
# Revision 1.1  1992/10/06  19:15:43  nort
# Initial revision
#
#
BEGIN {
  bittr[0] = "0x01"
  bittr[1] = "0x02"
  bittr[2] = "0x04"
  bittr[3] = "0x08"
  bittr[4] = "0x10"
  bittr[5] = "0x20"
  bittr[6] = "0x40"
  bittr[7] = "0x80"
  ncmds = 0
  nports = 0
  initcmds = 0
  pass = 0
  cmdsreported = 0
}
/^#BOARD / { board = $2; next }  # Would use toupper here
/^#INIT / {
  if (board in boards) {
    cmnt = index($0, ";")
	if (cmnt > 0) cmnt = " " substr($0, cmnt);
	else cmnt = ""
	inittxt = inittxt "  " $2 ", " $3 cmnt "\n"
	initcmds++
	boards[board] = 2
  }
  next
}
/^#PORT / {
  if (board in boards) {
	portno[board,$2] = nports
	portaddr[nports++] = "  " $3 ", 0 ; " board " " $2
  }
  next
}
/^#ENDPORTS/ {
  for (i in boards) {
	if (boards[i] == 1) {
	  print "Undefined Board: " i
	  exit 1
	}
  }
  print initcmds
  print inittxt
  print nports
  for (i = 0; i < nports; i++) print portaddr[i]
  pass++
  next
}
{  gsub("[\372\r]","") }
/^ *$/ { next }
!/ > / {
  if (pass != 0) print "; " $0
  next
}
/ > / {
  gsub(" > ", " ")
  sub("Main O3", "Main_O3")
  if (pass == 0) {
    if (ncmds > $5) {
	  print "Command " $5 " arrived after " ncmds " commands"
	  exit 1
	}
	boards[$2] = 1
    ncmds = $5+1
  } else {
    if (!cmdsreported) {
      print ncmds
      cmdsreported = 1
	  ncmds = 0
    }
	while (ncmds < $5) {
	  print "  SPARE,  0, 0x0000 ;Command " ncmds " unspecified."
	  ncmds++
	}
    chipno = $3 - 1
    bit = bittr[$4]
    if (chipno >= 2) bit = bit "00"
    else bit = bit "  "
    cmdtype = $1 ", "
    for (i = length($1); i < 5; i++) cmdtype = cmdtype " "
    board = $2   # Would like to use toupper here
    port = $3
    sub("^[A-Z]* *","")
    if (portno[board,port] < 10) cmdtype = cmdtype " "
    gsub("  *", " ")
    print "  " cmdtype portno[board,port] ", " bit " ;" $0
	ncmds++
  }
  next
}
