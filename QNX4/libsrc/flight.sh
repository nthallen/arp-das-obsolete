#! /bin/sh
# Flight instrument script
#__USAGE
#%C
export PATH=:/bin32:/bin:/usr/bin:/usr/local/bin
echo "\nRunning flight.sh: \c"; date
#----------------------------------------------------------------
# Make the console readable so we can ditto it easily if necessary
#----------------------------------------------------------------
chmod a+r `tty` 2> /dev/null
#----------------------------------------------------------------
# This is where we will decide what experiment we are
#----------------------------------------------------------------
cfile=Experiment.config
if [ -f "$cfile" ]; then
  . $cfile
else
  echo flight.sh: missing $cfile >&2
fi

if [ -n "$Experiment" -a -n "$HomeDir" -a -d "$HomeDir" ]; then
  cd $HomeDir
else
  echo flight.sh: Experiment or HomeDir undefined or non-existant >&2
  Experiment=none
fi
export Experiment

typeset launch_error=""

function Launch {
  name=$1
  shift
  [ -n "$launch_error" ] && return 1
  [ -n "$VERBOSE" ] && echo "Launch: $*"
  if { $* & }; then
	if [ "$name" != "-" ]; then
	  [ -n "$VERBOSE" ] && echo "Launch: Waiting for $!:$name"
	  namewait -p $! -t 20 $name || {
		echo "Launch namewait failure: $*" >&2
		launch_error=yes
		return 1
	  }
	fi
  else
	echo "Launch Error: $*" >&2
	launch_error=yes
	return 1
  fi
  return 0
}

umask g+w
[ -n "$FlightNode" ] && namewait -n0 pick_file
script=`pick_file -C`
if [ -r "$script" ]; then
  echo flight.sh: `id`: Experiment=$Experiment script=$script
  . $script
else
  echo flight.sh: Specified script $script not found >&2
fi
[ -z "$launch_error" ] && pick_file -q

typeset qoc
[ -n "$FlightNode" ] && qoc="-q"
exec parent -sy $qoc
