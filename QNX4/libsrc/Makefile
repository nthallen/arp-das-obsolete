# Makefile for /usr/local/lib/src
# EXEC files are linked into /usr/local/bin
# PRIV files are promoted and linked into /usr/local/bin
# SRC files will be distributed to /usr/local/lib/src on request
# SRCO files will be distributed but not backed up. They are linked
#      from other directories
# SRCX files are under development or otherwise not to be distributed
EXEC=Backup3
EXEC+=Copyin3
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
EXEC+=odinit
EXEC+=viewmet
EXEC+=hangup
EXEC+=promote
EXEC+=appclone
EXEC+=appgen
EXEC+=RCScheck
EXEC+=man
EXEC+=noderpt
EXEC+=reduce
EXEC+=rundos
EXEC+=data_attr
EXEC+=vt100
EXEC+=nm
PRIV=demote
PRIV+=distribute
PRIV+=fixdisk
PRIV+=odin
PRIV+=odout
PRIV+=osupdate
PRIV+=password
PRIV+=stopclock
SRC=cmdalgo.tmc
SRC+=libmaint.mk
SRC+=library.mk
SRC+=maint.mk
SRC+=maint.mk2
SRC+=maint.mk3
SRC+=copyin.mk
SRC+=rcsmaint.mk
SRC+=cmdmain.c
SRC+=cltmain.c
SRC+=srvmain.c
SRC+=doc2txt.awk
SRC+=edf2ext.awk
SRC+=fld2disp.awk
SRC+=prt2edf.awk
SRC+=prt2dccc.awk
SRC+=prt2tmc.awk
SRC+=prt2cmd.awk
SRC+=slp2cmd.awk
SRC+=slp2sol.awk
SRC+=getopt.c
SRC+=time_check
SRC+=appgen.mk
SRC+=dccc.por
SRC+=netinstall
SRC+=rcsid.c
SRC+=noderpt.awk
SRC+=update_node
SRC+=osupdate.awk
SRC+=env2pcl
SRCX=fixser
SRCX+=boot.floppy
SRCX+=nm.awk
SRCX+=mkalias
SRCX+=mkal.src
SRCX+=mkaliases
TOOL=Makefile
SRCO=colmain.skel extmain.skel cmdgen.skel

HOMEDIR=/usr/local/lib/src
MNC=libsrc
SOURCE=$(EXEC) $(PRIV) $(SRC) $(SRCX) $(TOOL)
TARGET=$(SRCO)
TGTDIR=$(TGTNODE)/usr/local/lib/src
DISTRIB=$(SRC) $(SRCO)

all : $(EXEC) $(PRIV)
	until find . -name promote -perm -4000 -perm 771 \
		  \( ! -perm -2000 \) -user root -group root >/dev/null; do\
	  su -c "chmod u=rwx,g=rwx,o=x promote; promote promote";\
	done
	promote $(PRIV)
	ln -f $(EXEC) $(PRIV) /usr/local/bin

distribution : $(DISTRIB)
	@if [ $(TGTDIR) -ef . ]; then echo Must define TGTNODE; exit 1; fi; :
	@if [ ! -d $(TGTDIR) ]; then mkdir -p $(TGTDIR); fi; :
	su -c "cp -vcpn $(DISTRIB) $(TGTDIR)"; :

	