# Makefile for /usr/local/lib/src
# EXEC files are linked into /usr/local/bin
# PRIV files are promoted and linked into /usr/local/bin
# SRC files will be distributed to /usr/local/lib/src on request
#  (Not all SRC files are also in osupdate.1. Some are only required
#   on the server)
# SRCO files will be distributed but not backed up. They are linked
#      from other directories
# SRCX files are under development or otherwise not to be distributed
EXEC=Backup3
EXEC+=Copyin3
EXEC+=Make
EXEC+=Makelib
EXEC+=Makercs
EXEC+=RCScheck
EXEC+=appclone
EXEC+=appgen
EXEC+=archive
EXEC+=arc
EXEC+=cls
EXEC+=data_attr
EXEC+=doc
EXEC+=doctex
EXEC+=extract
EXEC+=ext2rtg
EXEC+=fld_transfer
EXEC+=flight.sh
EXEC+=flinit
EXEC+=hangup
EXEC+=lprint
EXEC+=mkdoit
EXEC+=mkdoit2
EXEC+=noderpt
EXEC+=odinit
EXEC+=promote
EXEC+=reboot
EXEC+=saverun
EXEC+=tmg2tmc
EXEC+=unarc
EXEC+=viewmet
EXEC+=winsetsize
EXEC+=reduce
EXEC+=rundos
EXEC+=vt100
EXEC+=ar
EXEC+=cpp
EXEC+=nm
EXEC+=ranlib
###################################
# PRIVs are EXECs that get setuid #
###################################
PRIV=demote
PRIV+=distribute
PRIV+=fixdisk
PRIV+=flttime
PRIV+=odin
PRIV+=odout
PRIV+=osupdate
PRIV+=password
PRIV+=settime
PRIV+=stopclock
SRC=libmaint.mk
SRC+=library.mk
SRC+=maint.mk3
SRC+=appgen.mk
SRC+=boot.floppy
SRC+=copyin.mk
SRC+=cycle.awk
SRC+=dccc.por
SRC+=doc2txt.awk
SRC+=edf2ext.awk
SRC+=edf2oui.awk
SRC+=fld2disp.awk
SRC+=getopt.c
SRC+=idx64.cmd
SRC+=mkdoit2.awk
SRC+=mkdoit2.sh
SRC+=netinstall
SRC+=noderpt.awk
SRC+=osupdate.awk
SRC+=prt2cmd.awk
SRC+=prt2dccc.awk
SRC+=prt2edf.awk
SRC+=prt2tmc.awk
SRC+=rcsid.c
SRC+=rcsmaint.mk
SRC+=root.cmd
SRC+=slp2cmd.awk
SRC+=slp2sol.awk
SRC+=time_check
SRC+=tmg2tmc.awk
SRC+=update_host
SRC+=update_node
#######################################################
# SRCX are my local sources which are not distributed #
#######################################################
SRCX=fixser
SRCX+=archive
SRCX+=doctex.mk
SRCX+=doctex.tex
SRCX+=doc2tex.awk
SRCX+=env2pcl
SRCX+=nm.awk
SRCX+=mkalias
SRCX+=mkal.src
SRCX+=mkaliases
SRCX+=mkalsum
SRCX+=osupdate.1 osupdate.2 osu.win.1 os.421.1 os.422.1
SRCX+=osupdate.abigail
SRCX+=noderpt.alive
# These last few are now obsolete
SRCX+=maint.mk
SRCX+=maint.mk2
SRCX+=cmdmain.c
SRCX+=cltmain.c
SRCX+=srvmain.c
SRCX+=cmdalgo.tmc

TOOL=Makefile
# These come from application directories
SRCO=colmain.skel extmain.skel cmdgen.skel

HOMEDIR=/usr/local/lib/src
MNC=libsrc
SOURCE=$(EXEC) $(PRIV) $(SRC) $(SRCX) $(TOOL)
OBJECT=doc2roff.awk
TARGET=$(SRCO)
TGTNODE=//1
TGTDIR=$(TGTNODE)/usr/local/lib/src
DISTRIB=$(SRC) $(SRCO)

all : $(EXEC) $(PRIV)
	until find . -name promote -perm -4000 -perm 771 \
		  \( ! -perm -2000 \) -user root -group root >/dev/null; do\
	  su -c "chmod u=rwx,g=rwx,o=x promote; promote promote";\
	done
	until find . -name doc -group man \
	  -perm -2755 ! -perm -4000 >/dev/null; do\
	  su -c "chgrp man doc; chmod u=rwx,g=rxs,o=rx doc";\
	done
	promote $(PRIV)
	chmod g+s archive
	ln -f $(EXEC) $(PRIV) /usr/local/bin
	@for i in osupdate.1 os.421.1 os.422.1 osu.win.1; do\
	  echo Checking $$i: ;\
	  diff $$i //1/etc/config/$$i ;\
	done | more

distribution : $(DISTRIB)
	@if [ $(TGTDIR) -ef . ]; then echo Must define TGTNODE; exit 1; fi; :
	@if [ ! -d $(TGTDIR) ]; then mkdir -p $(TGTDIR); fi; :
	su -c "cp -vcpn $(DISTRIB) $(TGTDIR)"; :


# Disable the defualt .sh rule
mkdoit2 :
	: ;