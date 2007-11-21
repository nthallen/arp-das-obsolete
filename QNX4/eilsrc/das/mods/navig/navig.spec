tmcbase = navig.tmc swstat.tmc
cmdbase = //2/usr/local/lib/src/root.cmd navig.cmd navigfile.cmd swstat.cmd navigutil.c
NOSUBBUS
SRC = navigutil.c navvars.h navframe.h
DISTRIB = Experiment.config navig.cfg navattr.tmc
SCRIPT = interact navout
NONRCS = 

navigcol : navigutil.c
navigclt : navigutil.c
navigsrvr : navigutil.c
navigdisp : navigutil.c navig.tbl
navigdoit : navig.doit
navigalgo : navig.tma
