#! /bin/sh
# Flight instrument script
#__USAGE
#%C
export PATH=:/bin32:/bin:/usr/bin:/usr/local/bin
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

umask g+w
. `pick_file -C`
pick_file -q
if [ "$1" = "wait" ]; then
  namewait -N -n0 pick_file
  pick_file -q
else
  exec namewait -N -n0 pick_file
fi
