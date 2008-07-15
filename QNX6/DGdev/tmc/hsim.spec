tmcbase = hsim.tmc
cmdbase = /usr/local/lib/src/root.cmd hsim.cmd

SRC = hsim.pl box.pl fld.pl htrsim.h hsim.tmas* hsim.rtg
OBJ = hsim.log hsim.gdt
NONRCS = hsim.cfg fields.cfg
SCRIPT = interact Experiment.config Inetdoit
NOSUBBUS

hsimcol :
hsimdisp : /usr/local/lib/src/flttime.tmc hsim.tbl hsim.tmg
hsimdoit : hsim.doit
hsimext : hsim.edf
hsimalgo : hsim.tma scan.tmg
playdoit : play.doit
%%
hsimdoit : hsim.fld
TMAREV=tmcalgoV2R1 -D $(<:tma=gdt)
