tmcbase = hsim.tmc
cmdbase = /usr/local/lib/src/root.cmd hsim.cmd

SRC = hsim.pl box.pl fld.pl htrsim.h hsim.tmas* hsim.rtg
OBJ = hsim.log
NONRCS = hsim.cfg fields.cfg
SCRIPT = interact playback Experiment.config
NOSUBBUS

hsimcol :
hsimdisp : hsim.fld hsim.tmg
hsimdoit : hsim.doit
hsimext : hsim.edf
hsimalgo : hsim.tma scan.tmg
playdoit : play.doit
%%
hsim.fld : hsim.pl box.pl fld.pl
	hsim.pl >hsim.fld
hsimdoit : hsim.fld
