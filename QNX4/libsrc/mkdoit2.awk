# mkdoit2.awk Generates a doit script
#	batchfile <batch file name>
#	memo [ <log file name> ]
#		specifies that "more -e <log file>" should run on a spare console
#	display <program> <screen file> [ <screen file> ... ]
#		specifies a display program to run
#	extraction <program> [options]
#		specifies an extraction to run, including rtg extractions
#	algorithm <program> [options]
#		specifies an algorithm to run
#	client <program> [options]
#		specifies a keyboard client to run
#	monoconfig <filename>
#	colorconfig <filename>
#		specifies screen configuration files to use
#	rtg <config file>
#		start up rtg if it isn't already running
#
#  options may only be specified once for a particular program.
#----------------------------------------------------------------
# New version begun 9/12/95
#  No longer using "row". Will instead place screen items
#  based on the .fld files. Fields with text beginning with
#  % will designate instructions to mkdoit:
#   %STATUS:<app>   Indicates a field to locate the status
#                   for the specified application
#   %STATUS         Indicates a general status field for any
#                   applications without a specific status field
#   %CLIENT         Indicates that a keyboard client should run
#                   on this screen.
#   %TMA:<app>[:#]  Defines a field for the specified TMCALGO
#                   application's partition status. The
#                   partition number is optional to support
#                   status for multiple partitions. The
#                   partitions are numbered starting with 1.
# /* fields: number, line, col, width, length, attribute code, string */
# #FIELD# 152  4 72 5 1 4 "%TMA:hoxalgo:3"
#----------------------------------------------------------------
# app[program] = "$_genopts"
# TM[program] = "$_dcopts"
# CMD[program] = "$_cmdopts"
# DISP[program] = "-A ..."
# CltScreens = " -A ..."
# opts[program] = "-l"
# Statline[program] = "-o ..."
# Statline["general"] = "-o ..."
# tma_con[ program, number ] = "$_scr0"
# tma_row[ program, number ] = 5
# tma_nparts[ program ] = largest partition number referenced
#
#  Every application goes on the app[] list and then is
#  added to the appropriate other lists. TM[] is for TM clients
#  CMD is for command clients, DISP is for display programs.
#  The actual value in that list is a string containing the
#  arguments appropriate to that particular program.
#
#  displays[n_displays] stores the names of the display programs
#  exts[n_exts] stores the names of the extractions
#  algos[n_algos] stores the names of the algorithms
#    All of these are 1-based.
#
#  disp_screens[n_displays] = number of screens/consoles for this display prog
#  disp_con[n_displays,i] = console number for this prog's i'th screen
#  disp_fld[n_displays,i] = fld file for this prog's i'th screen
#    All indicies here are 1-based.
#----------------------------------------------------------------
function nl_error( level, text ) {
  system( "echo " FILENAME ":" NR lvlmsg[level] ": " text " >&2" )
  if ( level >= 2 ) exit(1)
}

function output_header( text ) {
  printf "\n"
  print "#----------------------------------------------------------------"
  print "# " text
  print "#----------------------------------------------------------------"
}

#----------------------------------------------------------------
# program is the name of the program
# background should be " &" if the program is supposed to run in
# the background, "" otherwise
#----------------------------------------------------------------
function output_app( program, background ) {
  if ( app[ program ] == "" )
	nl_error( 4, "Application not initialized" )
  printf "%s", program " $_msgopts"
  if ( TM[program] != "" ) printf " $_dcopts"
  if ( CMD[program] != "" ) printf " $_cmdopts"
  if ( DISP[program] != "" ) printf "%s", DISP[program]
  if ( Statline[program] != "" )
	printf " %s", Statline[program]
  else if ( Statline["general"] != "" )
	printf " %s", Statline["general"]
  if ( opts[program] != "" ) printf "%s", opts[program]
  print background
  if ( background != "" )
	if ( bg_ids == 1 ) print "_bg_pids=\"$_bg_pids $!\""
}

BEGIN {
  lvlmsg[1] = " Warning"
  lvlmsg[2] = " Error"
  lvlmsg[3] = " Fatal"
  lvlmsg[4] = " Internal"
  log_file_name = "$Experiment.log"
  batch_file_name = "interact"
  n_displays = 0
  n_screens = 0
  n_exts = 0
  n_algos = 0
  scrno = -1
  lastcltscr = -1
}
/^#FIELD#.*"%/ { if ( scrno < 0 ) nl_error( 4, "Got a #FIELD#" ) }
/^#FIELD#.*"%STATUS:/ {
  prog = $NF
  sub( "^\"%STATUS:", "", prog )
  sub( "\".*$", "", prog )
  Statline[ prog ] = "-o $_scr" scrno "," $3 "," $4 "," $5 ",$_attrs"
  next
}
/^#FIELD#.*"%STATUS"/ {
  Statline[ "general" ] = "-o $_scr" scrno "," $3 "," $4 "," $5 ",$_attrs"
  next
}
/^#FIELD#.*"%CLIENT"/ {
  if ( lastcltscr != scrno ) {
	if ( lastcltscr == -1 ) {
	  if ( scrno != 0 ) CltScreens = " -A $_scr" scrno
	} else CltScreens = CltScreens " -a $_scr" scrno
	lastcltscr = scrno
  }
  next
}
/^#FIELD#.*"%TMA:/ {
  prog = $NF
  sub( "\"%TMA:", "", prog )
  sub( "\"$", "", prog )
  pos = index( prog, ":" )
  if ( pos > 0 ) {
	partno = substr( prog, pos+1 )
	prog = substr( prog, 1, pos-1 )
  } else partno = 1
  tma_con[ prog, partno ] = "$_scr" scrno
  tma_row[ prog, partno ] = $3
  if ( partno > tma_nparts[ prog ] )
	tma_nparts[ prog ] = partno
  next
}
scrno >= 0 { next }
{ sub( "^[ \t]*", "" ) }
/^#/ { next }
/^[ /t]*$/ { next }
/^memo/ {
  if ( NF > 1 ) log_file_name = $2
  memo = "yes"
  next
}
/^monoconfig/ { monoconfig = $2; next }
/^colorconfig/ { colorconfig = $2; next }
/^rtg/ { rtg = $2; next }
/^batchfile/ { batch_file_name = $2; next }
/^display/ {
  if ( NF < 3 ) nl_error( 3, "display requires a screen name" )
  n_displays++
  displays[n_displays] = $2
  disp_screens[n_displays] = NF-2
  for ( i = 3; i <= NF; i++ ) {
	disp_con[n_displays,i-2] = n_screens
	disp_fld[n_displays,i-2] = $i
	# Queue .fld for later processing
	ARGV[ARGC] = "scrno=" n_screens
	ARGC++
	ARGV[ARGC] = $i ".fld"
	ARGC++
	n_screens++
  }
  if ( app[ $2 ] == "" ) {
	app[ $2 ] = "yes"
	TM[ $2 ] = "yes"
  }
  next
}
/^extraction/ {
  if ( app[ $2 ] == "" ) {
	n_exts++
	exts[ n_exts ] = $2
	app[ $2 ] = "yes"
  }
  TM[ $2 ] = "yes"
  if ( opts[ $2 ] == "" && NF > 2 ) {
	$1 = $2 = ""; sub( "^ *", "" )
	opts[ $2 ] = $0
  }
  next
}
/^algorithm/ {
  if ( app[ $2 ] == "" ) {
	n_algos++
	algos[n_algos] = $2
	app[ $2 ] = "yes"
  }
  TM[ $2 ] = "yes"
  CMD[ $2 ] = "yes"
  if ( opts[ $2 ] == "" && NF > 2 ) {
	$1 = $2 = ""; sub( "^ *", "" )
	opts[ $2 ] = $0
  }
  next
}
/^client/ {
  if ( client != "" )
	nl_error( 3, "Only one keyboard client allowed per script" )
  client = $2
  app[ $2 ] = "yes"
  CMD[ $2 ] = "yes"
  if ( opts[ $2 ] == "" && NF > 2 ) {
	$1 = $2 = ""; sub( "^ *", "" )
	opts[ $2 ] = $0
  }
  next
}
{ nl_error( 3, "Syntax Error: " $0 ) }
END {
  if (n_displays == 0 && n_exts == 0 && n_algos == 0 && client == "" )
	nl_error( 3, "No programs defined" )
  if ( memo == "yes" && client == "" )
	nl_error( 3, "memo requires a client" )
  print "#! /bin/sh"
  print "#__USAGE"
  print "#%C"
  print "#	Starts Instrument operation, including"
  for (i = 1; i <= n_displays; i++) {
	print "#	  display " displays[i]
  }
  for (i = 1; i <= n_exts; i++) {
	print "#	  extraction " exts[i]
  }
  for (i = 1; i <= n_algos; i++) {
	print "#	  algorithm " algos[i]
  }
  if ( client != "" ) print "#	  client " client
  if ( memo == "yes" ) print "#	  command log"
  if ( rtg == "yes" ) print "#	  rtg"
  print "#%C	stop"
  print "#	Terminates Instrument Operations"
  print "#%C	not"
  print "#	Prevents Instrument Operation on power up by invoking"
  print "#	pick_file /dev/null"
  print "#"
  print "#The command line to use when adding this script to your"
  print "#QNX Windows workspace menu is:"
  print "#"
  print "#        /windows/bin/wterm <path>"
  print "#"
  print "#where <path> is the fully-qualified path to this script."
  printf "\n"
  print "cd `dirname $0`"
  print "cfile=Experiment.config"
  print "if [ ! -f \"$cfile\" ]; then"
  print "  echo Cannot locate $cfile >&2"
  print "  exit 1"
  print "fi"
  print "unset Experiment HomeDir FlightNode"
  print ". $cfile"
  print "if [ -z \"$Experiment\" ]; then"
  print "  echo Experiment undefined in $cfile >&2"
  print "  exit 1"
  print "fi"
  print "export Experiment"
  print "[ -n \"FlightNode\" ] && export FlightNode"

  print "\nfor i in ; do"
  print "  case $i in"
  print "    not)"
  print "      echo Deterring Startup of Experiment $Experiment"
  print "      echo Waiting for pick_file"
  print "      pick_file /dev/null"
  print "      exit 0;;"
  print "    stop)"
  print "      if [ -z \"$FlightNode\" ]; then"
  print "        FlightNode=`namewait -n0 -t0 -G parent 2>/dev/null`"
  print "        if [ -z \"$FlightNode\" ]; then"
  print "          echo Unable to locate flight node for experiment $Experiment >&2"
  print "          exit 1"
  print "        fi"
  print "      fi"
  print "      echo Shutting down Experiment $Experiment on Node $FlightNode"
  print "      on -f $FlightNode /usr/local/bin/startdbr quit"
  print "      exit 0;;"
  print "    *) : ;;"
  print "  esac"
  print "done"

  #----------------------------------------------------------------
  # Now we need to collect the required consoles
  #----------------------------------------------------------------
  if ( n_screens == 0 ) n_screens = 1
  if ( memo == "yes" ) n_screens++
  print "\n_scr0=`tty`"
  if ( n_screens > 1 ) {
	printf "typeset gcpid"
	for (i = 1; i < n_screens; i++) printf " _scr" i
	printf "\n%s", "eval `getcon ${_scr0%[0-9]}"
	for (i = 1; i < n_screens; i++) printf " _scr" i
	print "`"
	print "[ -z \"$_scr1\" ] && exit 1"
  }

  print "\necho Waiting for pick_file"
  print "FlightNode=`pick_file -n " batch_file_name "`"
  print "[ -n \"$FlightNode\" ] || exit 1"
  
  if ( n_displays > 0 ) {
	print
	if ( colorconfig != "" || monoconfig != "" )
	  print "typeset _cfgfile"
	print "if [ -z \"$MONOCHROME\" ]; then"
	print "  _attrs=02,06,04,05"
	if ( colorconfig != "" ) print "  _cfgfile=" colorconfig
	if ( monoconfig != "" ) {
	  print "else"
	  print "  _cfgfile=" monoconfig
	}
	print "fi"
  }

  if ( rtg != "" ) {
	print "\nnamewait -t0 huarp/rtg 2>/dev/null || {"
	print "  [ ! -f " rtg " ] && touch " rtg
	print "  on -t /dev/con1 /windows/apps/rtg/rtg -f " rtg
	print "  renice -2 -p $$"
	print "}"
  }
  
  if ( n_displays > 0 || n_exts > 0 || n_algos > 0 ) {
	print "\necho Waiting for Data Buffer"
	print "namewait -n$FlightNode db"
  }
  
  if ( n_algos > 0 || client != "" ) {
	print "\necho Waiting for Command Interpreter"
	print "namewait -n$FlightNode cmdinterp"
  }
  
  # printf "\n%s\n", "_out=`tty`," --row ",0,80,$_attrs"
  
  print "\ntypeset _msgopts _dcopts _cmdopts"
  print "_msgopts=\" -v -c$FlightNode\""
  print "_dcopts=\" -b$FlightNode -i1\""
  print "_cmdopts=\" -C$FlightNode\""
  #----------------------------------------------------------------
  # bg_procs is true if there are background processes
  # bg_ids is true if we need to keep track of their pids
  #----------------------------------------------------------------
  if ( n_displays > 0 || n_exts > 0 || n_algos > 0 ) {
	bg_procs = 1
  } else bg_procs = 0

  if (client != "" && bg_procs ) {
	bg_ids = 1
	print "typeset _bg_pids='-p'"
  } else bg_ids = 0

  if ( memo == "yes" ) {
	printf "\n%s", "on -t $_scr" n_screens-1
	printf "%s\n", " less +F //$FlightNode$HomeDir/" log_file_name
  }

  if ( n_displays > 0 )
	output_header( "Display Programs:" )
  #----------------------------------------------------------------
  # Determine display options for each display program
  #----------------------------------------------------------------
  for ( i = 1; i <= n_displays; i++ ) {
	prog = displays[i]
	part_no = 1 # Next partition to consider for display
	for ( j = 1; j <= disp_screens[j]; j++ ) {
	  if ( j != 1 || disp_con[ j ] != 0 ) {
		if ( j == 1 ) opt = "-A"
		else opt = "-a"
		DISP[ prog ] = DISP[ prog ] " " opt " $_scr" disp_con[ j ]
	  }
	  # Now check to see if we have partition status for this prog
	  while ( part_no <= tma_nparts[ prog ] ) {
		part_con = tma_con[ prog, part_no ]
		if ( part_con != "" ) {
		  if ( part_con < disp_con[ j ] ) {
			# report error cannot display partition part_no
			part_no++
		  } else if ( part_con == disp_con[ j ] ) {
			DISP[ prog ] = DISP[ prog ] " -r " tma_row[ prog, part_no ]
			part_no++
		  } else break
		}
	  }
	}
  }
  
  #----------------------------------------------------------------
  # display each screen
  #----------------------------------------------------------------
  for ( i = 1; i <= n_displays; i++ ) {
	n_scrs = disp_screens[i]
	for ( j = 1; j <= n_scrs; j++ ) {
	  console = "$_scr" disp_con[i,j]
	  if ( console == "$_scr0" ) scr = ""
	  else {
		scr = " > " console " < " console
		scr = scr "; stty +opost < " console
	  }
	  printf "%s", "scrpaint $_msgopts " disp_fld[i,j]
	  if ( colorconfig != "" || monoconfig != "" )
		printf " $_cfgfile"
	  print scr
	}
	output_app( displays[ i ], " &" )
  }

  #----------------------------------------------------------------
  # Release getcon if we started it
  #----------------------------------------------------------------
  if ( n_screens > 1 ) {
	print "getcon -q $gcpid"
  }

  if ( n_exts > 0 )
	output_header( "Extraction Programs:" )

  for ( i = 1; i <= n_exts; i++ )
	output_app( exts[ i ], " &" )
  
  if ( n_algos > 0 )
	output_header( "Algorithms:" )

  for ( i = 1; i <= n_algos; i++ ) {
	prog = algos[ i ]
	curscr = ""
	for ( part_no = 1; part_no <= tma_nparts[ prog ]; part_no++ ) {
	  part_con = tma_con[ prog, part_no ]
	  if ( part_con == "" ) {
		if ( curscr == "" ) curscr = "$_scr0"
		DISP[ prog ] = DISP[ prog ] " -r -1"
	  } else {
		if ( part_con != curscr ) {
		  if ( curscr == "" ) {
			if ( part_con != "$_scr0" )
			  DISP[ prog ] = DISP[ prog ] " -A " part_con
		  } else DISP[ prog ] = DISP[ prog ] " -a " part_con
		  curscr = part_con
		}
		DISP[ prog ] = DISP[ prog ] " -r " tma_row[ prog, part_no ]
	  }
	}
	output_app( prog, " &" )
  }

  if ( client != "" ) {
	output_header( "Keyboard Client:" )
	if ( lastcltscr != -1 )
	  DISP[ client ] = DISP[ client ] CltScreens
	else {
	  j = n_screens
	  if ( memo == "yes" ) j--
	  for ( i = 1; i < j; i++ )
		DISP[ client ] = DISP[ client ] " -a $_scr" i
	}
	output_app( client, "" )

	if ( memo == "yes" )
	  print "slay -t /${_scr" n_screens-1 "#//*/} less"
  }

  if ( bg_procs == 1 ) {
	printf "%s", "exec parent -qvn"
	if ( bg_ids == 1 ) printf "t3%s", " \"$_bg_pids\""
	for ( i = 0; i < n_screens; i++ ) {
	  printf " $_scr" i
	}
	printf "\n"
  }
}
