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
EXEC+=Backup4
EXEC+=Copyin4
EXEC+=DR2tmc
EXEC+=Make
EXEC+=Makelib
EXEC+=Makercs
EXEC+=RCScheck
EXEC+=appclone
EXEC+=appgen
EXEC+=archive
EXEC+=arc
EXEC+=checkts
EXEC+=cls
EXEC+=data_attr
EXEC+=dircksum
EXEC+=dircompare
EXEC+=doc
EXEC+=doctex
EXEC+=dupdir
EXEC+=extract
EXEC+=ext2rtg
EXEC+=fld_transfer
EXEC+=flight.sh
EXEC+=flinit
EXEC+=getarc
EXEC+=hangup
EXEC+=help
EXEC+=idx64cfg
EXEC+=install_app
EXEC+=libdoc
EXEC+=lprint
EXEC+=mkdoit
EXEC+=mkdoit2
EXEC+=mkpatch
EXEC+=mlf_find
EXEC+=noderpt
EXEC+=odinit
EXEC+=prepCD
EXEC+=promote
EXEC+=rcs2html
EXEC+=saverun
EXEC+=tmg2tmc
EXEC+=tmpath
EXEC+=unarc
EXEC+=winsetsize
EXEC+=reduce
EXEC+=vt100
EXEC+=ar
EXEC+=cpp
EXEC+=nm
EXEC+=ranlib
EXEC+=tbl2txt
EXEC+=mailpage
EXEC+=makepage
EXEC+=makepages
###################################
# PRIVs are EXECs that get setuid #
###################################
PRIV=demote
PRIV+=distribute
PRIV+=fixdisk
PRIV+=flttime
PRIV+=mount_od
PRIV+=umount_od
PRIV+=osupdate
PRIV+=password
PRIV+=pcin pcout
PRIV+=reboot
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
SRC+=editfile.bat
SRC+=fld2disp.awk
SRC+=flttime.tmc
SRC+=getopt.c
SRC+=hpfrun.cmd
SRC+=idx64.cmd
SRC+=idx64cfg.awk
SRC+=Makefile.pm
SRC+=mkdoit2.awk
SRC+=mkdoit2.sh
SRC+=netinstall
SRC+=noderpt.awk
SRC+=prt2cmd.awk
SRC+=prt2dccc.awk
SRC+=prt2edf.awk
SRC+=prt2tmc.awk
SRC+=rcsid.c
SRC+=rcsmaint.mk
SRC+=root.cmd
SRC+=slp2cmd.awk
SRC+=slp2sol.awk
SRC+=Table.pm
SRC+=time_check
SRC+=tmg2tmc.awk
#############################################################
# Config files: These are passively compared to copies
# in //1/etc/config in an attempt to keep local copies current
#############################################################
SRCCFG=sysinit.all netmaptrap
SRCCFG+=OSU.1 OSU.423.1
SRCCFG+=update_net update_passwd

#############################################################
# SRCX are my local sources which are not (yet) distributed #
#############################################################
SRCX=fixser
SRCX+=deluser
SRCX+=doctex.mk
SRCX+=doctex.tex
SRCX+=doc2tex.awk
SRCX+=nm.awk
SRCX+=newboot
SRCX+=noderpt.alive
SRCX+=toc2html
SRCX+=oldfiles
SRCX+=osupdate.test
SRCX+=maildb.pl maildb.src
SRCX+=tabletest rcsupdate
SRCX+=resynch
SRCX+=SetupBackup
SRCX+=VerifyBackup
SRCX+=tapepipe
SRCX+=parsetc
SRCX+=getarcsum
# These last few are now obsolete
SRCXO=env2pcl
SRCXO+=maint.mk
SRCXO+=maint.mk2
SRCXO+=cmdmain.c
SRCXO+=cltmain.c
SRCXO+=srvmain.c
SRCXO+=cmdalgo.tmc
SRCXO+=osupdate.sh osupdate.1 osu.win.1 os.422.1
SRCXO+=osupdate.awk osupdate.2 osupdate.abigail
SRCXO+=update_host update_node

TOOL=Makefile
TOOL+=rcs2html.dat
TOOL+=todo
TOOL+=todo.getarc

# These come from application directories
SRCO=colmain.skel extmain.skel cmdgen.skel

HOMEDIR=/usr/local/lib/src
MNC=libsrc
SOURCE=$(EXEC) $(PRIV) $(SRC) $(SRCCFG) $(SRCX) $(SRCXO) $(TOOL)
OBJECT=doc2roff.awk maildb.text source.html sourcel.html source.txt
TARGET=$(SRCO)
TGTNODE=//1
TGTDIR=$(TGTNODE)/usr/local/lib/src
DISTRIB=$(SRC) $(SRCO)

all : $(EXEC) $(PRIV) $(SRCCFG)
	@find . -name promote -perm -4000 -perm 771 \
	  \( ! -perm -2000 \) -user root -group root >/dev/null ||\
	  su -c "chmod u=rwx,g=rwx,o=x promote; promote promote"
	@find . -name doc -group man \
	  -perm -2755 ! -perm -4000 >/dev/null ||\
	  su -c "chgrp man doc; chmod u=rwx,g=rxs,o=rx doc"
	promote $(PRIV)
	chmod g+s archive
	@echo "Update executables in /usr/local/bin: \\c"; \
	 su -c "ln -i $(EXEC) $(PRIV) /usr/local/bin"
	promote netmaptrap
	@for i in $(SRCCFG); do\
	  if cmp -s $$i //1/etc/config/$$i; then\
		echo $$i is current;\
	  else echo WARNING: $$i differs from //1/etc/config/$$i ;\
	  fi;\
	done; :

distribution : $(DISTRIB)
	@if [ $(TGTDIR) -ef . ]; then echo Must define TGTNODE; exit 1; fi; :
	@if [ ! -d $(TGTDIR) ]; then mkdir -p $(TGTDIR); fi; :
	su -c "cp -vcpni $(DISTRIB) $(TGTDIR)"; :


# Disable the defualt .sh rule
mkdoit2 :
	: ;
source.html : $(SOURCE) RCS/*
	rcs2html /usr/local/lib/src
