# Makefile for /usr/local/lib/src
EXEC=Backup
EXEC+=Copyin
EXEC+=Make
EXEC+=Makelib
EXEC+=Makercs
EXEC+=arc
EXEC+=cls
EXEC+=extract
EXEC+=flinit
EXEC+=lprint
EXEC+=reboot
EXEC+=saverun
EXEC+=unarc
EXEC+=password
EXEC+=snafu
EXEC+=odin
EXEC+=odout
EXEC+=odinit
EXEC+=viewmet
EXEC+=hangup
SRC=cmdalgo.tmc
SRC+=libmaint.mk
SRC+=maint.mk
SRC+=maint.mk2
SRC+=rcsmaint.mk
SRC+=cmdmain.c
SRC+=cltmain.c
SRC+=srvmain.c
SRC+=edf2ext.awk
SRC+=fld2disp.awk
SRC+=prt2edf.awk
SRC+=prt2dccc.awk
SRC+=prt2tmc.awk
SRC+=prt2cmd.awk
SRC+=getopt.c
SRC+=netinstall
SRC+=time_check
TOOL=Makefile

HOMEDIR=/usr/local/lib/src
MNC=libsrc
SOURCE=$(EXEC) $(SRC) $(TOOL)

all :
	ln -f $(EXEC) /usr/local/bin
	