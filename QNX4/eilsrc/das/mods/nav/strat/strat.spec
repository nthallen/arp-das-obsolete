# strat.spec specification for the O3/H2O STRAT configuration
# The o3tmc.prt comes from //14/home/ozone/src/ozone.ag
# The h2otmc.prt comes from //11/home/wvc/src/h2otmc.prt
# There is apparently no h2o.ag anymore...  :-(
tmcbase = types.tmc strattmc.prt
tmcbase = o3types.tmc o3tmc.prt o3grp.tmc
tmcbase = h2otmc.prt h2ogrp.tmc
cmdbase = root.cmd all.cmd
cmdbase = o3cmd.prt swstat.cmd o3sw.cmd
cmdbase = h2ocmd.prt ix.cmd stratcmd.prt
SCRIPT = interact runfile.dflt o3doit h2odoit hskdoit Experiment.config
SRC = h2o.cyc
TGTDIR = $(TGTNODE)/home/strat
stratcol : stratcol.tmc
strat.dccc : stratdig.prt o3dig.prt h2odig.prt
h2odisp : h2octr.tmc h2o.fld h2o.cfg
o3disp : o3rat.tmc o3.cyc o3.fld o3.cfg
hskdisp : hsk.fld
o3.sft : o3.sol
h2o.sft : h2o.sol
o3ext : o3.edf
h2oext : h2o.edf
rtgext : o3rat.tmc o3.cyc o3rtg.tmc o3rtg.oui
hskext : hskext.prt
testalgo : test.tma
o3balgo : o3b.tma
