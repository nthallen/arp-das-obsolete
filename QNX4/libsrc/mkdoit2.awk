# mkdoit2.awk Generates a doit script
#	batchfile <batch file name>
#	memo [ <log file name> ]
#		specifies that "more -e <log file>" should run on a spare console
#	display <program> <screen file> [ <screen file> ... ]
#		specifies a display program to run
#	extraction <program>
#		specifies an extraction to run, including rtg extractions
#	algorithm <program>
#		specifies an algorithm to run
#	client <program>
#		specifies a keyboard client to run
#	monoconfig <filename>
#	colorconfig <filename>
#		specifies screen configuration files to use
#	rtg <config file>
#		start up rtg if it isn't already running
#----------------------------------------------------------------
#  Use of row: Start at the bottom (row 24). Two lines reserved
#  for keyboard client, if any. Next line reserved
#  for global status ($_out) then two lines for each algorithm
#  Decrement row by the number or rows you need *before* you
#  use it.
#----------------------------------------------------------------

BEGIN {
  log_file_name = "$Experiment.log"
  batch_file_name = "interact"
  n_displays = 0
  n_screens = 0
  n_exts = 0
  n_algos = 0
}
/^ *#/ { next }
/^ *memo/ {
  if ( NF > 1 ) log_file_name = $2
  memo = "yes"
  n_screens++
  next
}
/^ *monoconfig/ { monoconfig = $2; next }
/^ *colorconfig/ { colorconfig = $2; next }
/^ *rtg/ { rtg = $2; next }
/^ *batchfile/ { batch_file_name = $2; next }
/^ *display/ {
  if ( NF < 3 ) {
	system( "echo " FILENAME ":" NR " Error: display requires a screen name >&2" )
	exit(1)
  }
  if ( n_displays > 0 ) n_screens++
  n_screens += NF-3
  n_displays++
  displays[n_displays] = $2
  disp_screens[n_displays] = NF-2
  for ( i = 3; i <= NF; i++ ) {
	screens[n_displays,i-2] = $i
  }
  next
}
/^ *extraction/ {
  n_exts++
  exts[n_exts] = $2
  next
}
/^ *algorithm/ {
  n_algos++
  algos[n_algos] = $2
  next
}
/^ *client/ {
  if ( client != "" ) {
	system( "echo " FILENAME ":" NR " Error: Only one keyboard client allowed per script >&2" )
	exit(1)
  }
  client = $2
  next
}
{
  system( "echo " FILENAME ":" NR " Syntax Error >&2" )
  exit(1)
}
END {
  if (n_displays == 0 && n_exts == 0 && n_algos == 0 && client == "" ) {
	system( "echo " FILENAME ": Error - No programs defined >&2" )
	exit(1)
  }
  if ( memo == "yes" && client == "" ) {
	system( "echo " FILENAME ": Error - memo requires a client >&2" )
	exit(1)
  }
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
  print "\n_scr0=`tty`"
  if ( n_screens > 0 ) {
	printf "typeset gcpid"
	for (i = 1; i <= n_screens; i++) printf " _scr" i
	printf "\n%s", "eval `getcon ${_scr0%[0-9]}"
	for (i = 1; i <= n_screens; i++) printf " _scr" i
	print "`"
	print "[ -z \"$_scr1\" ] && exit 1"
  }

  print "\necho Waiting for pick_file"
  print "FlightNode=`pick_file -n " batch_file_name "`"
  print "[ -n \"$FlightNode\" ] || exit 1"
  
  if ( n_displays > 0 ) {
	print "\ntypeset _cfgfile"
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
  
  # reserve space for client
  row = 25
  if ( client != "" ) row -= 2
  
  printf "\n%s\n", "_out=`tty`," --row ",0,80,$_attrs"
  
  print "\ntypeset _scropts _bufopts _algopts _cltopts"
  print "_scropts=\"-v -o $_out -c$FlightNode\""
  print "_bufopts=\"$_scropts -b$FlightNode\""
  print "_algopts=\"$_bufopts -C$FlightNode -i1 -l\""
  print "_cltopts=\"$_scropts -C$FlightNode\""
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
	printf "%s\n", "on -t $_scr" n_screens " less +F //$FlightNode$HomeDir/" log_file_name
  }

  used_screens = 0
  if ( n_displays > 0 ) {
	printf "\n"
	print "#----------------------------------------------------------------"
	print "# Display Programs:"
	print "#----------------------------------------------------------------"
  }
  #----------------------------------------------------------------
  # display each screen
  #----------------------------------------------------------------
  for ( i = 1; i <= n_displays; i++ ) {
	n_scrs = disp_screens[i]
	scr_opts = ""
	for ( j = 1; j <= n_scrs; j++ ) {
	  if ( j == 1 && used_screens == 0 ) {
		scr = ""
	  } else {
		scr = " > $_scr" used_screens " < $_scr" used_screens
		scr = scr "; stty +opost < $_scr" used_screens
		if ( j == 1 ) scr_opts = scr_opts " -A $_scr" used_screens
		else scr_opts = scr_opts " -a $_scr" used_screens
	  }
	  used_screens++
	  print "scrpaint $_scropts " screens[i,j] " $_cfgfile" scr
	}
	printf "%s", displays[ i ] " $_bufopts -i1"
	print scr_opts " &"
	if ( bg_ids == 1 ) print "_bg_pids=\"$_bg_pids $!\""
  }
  #----------------------------------------------------------------
  # Release getcon if we started it
  #----------------------------------------------------------------
  if ( n_screens > 0 ) {
	print "getcon -q $gcpid"
  }

  if ( n_exts > 0 ) {
	printf "\n"
	print "#----------------------------------------------------------------"
	print "# Extraction Programs:"
	print "#----------------------------------------------------------------"
  }
  for ( i = 1; i <= n_exts; i++ ) {
	print exts[ i ] " $_bufopts &"
	if ( bg_ids == 1 ) print "_bg_pids=\"$_bg_pids $!\""
  }
  
  if ( n_algos > 0 ) {
	printf "\n"
	print "#----------------------------------------------------------------"
	print "# Algorithms:"
	print "#----------------------------------------------------------------"
  }
  for ( i = 1; i <= n_algos; i++ ) {
	row -= 2
	print algos[ i ] " $_algopts -r " row " &"
	if ( bg_ids == 1 ) print "_bg_pids=\"$_bg_pids $!\""
  }
  
  if ( client != "" ) {
	printf "\n"
	print "#----------------------------------------------------------------"
	print "# Keyboard Client:"
	print "#----------------------------------------------------------------"
	printf "%s",  client " $_cltopts"
	#----------------------------------------------------------------
	# n_screens is the number of "additional" consoles required, not
	# counting the current console.
	# If a memo log was requested, that accounts for one console.
	# All the other consoles are displays and will get a client
	# command line.
	#----------------------------------------------------------------
	j = n_screens
	if ( memo == "yes" ) j--
	for ( i = 1; i <= j; i++ ) printf " -a $_scr%d", i
	if ( bg_ids == 1 ) printf "%s", " && _bg_pids=\"\""
	printf "\n"
	if ( memo == "yes" ) {
	  print "slay -t /${_scr" n_screens "#//*/} less"
	  # print "echo \"\\\\f\\\\c\" > $_scr" n_screens
	}
  }

  if ( bg_procs == 1 ) {
	printf "%s", "exec parent -qvn"
	if ( bg_ids == 1 ) printf "t3%s", " \"$_bg_pids\""
	for ( i = 0; i <= n_screens; i++ ) {
	  printf " $_scr" i
	}
	printf "\n"
  }
}
