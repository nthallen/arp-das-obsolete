# Makefile for /usr/local/src/libsrc
# EXEC files are linked into /usr/local/bin
# PRIV files are promoted and linked into /usr/local/bin
# SRC files will be distributed to /usr/local/lib/src on request
#  (Not all SRC files are also in osupdate.1. Some are only required
#   on the server)
# SRCX files are under development or otherwise not to be distributed
#  Suffix of X means not distributed
#  Suffix of O means obsolete
EXEC=Backup4
EXEC+=CDimage
EXEC+=Copyin4
EXEC+=DR2tmc
EXEC+=Make
EXEC+=Makelib
EXEC+=Makercs
EXEC+=RCScheck
EXEC+=appgen
EXEC+=checkts
EXEC+=cls
EXEC+=data_attr
EXEC+=dircksum
EXEC+=dircompare
EXEC+=extract
EXEC+=ext2rtg
EXEC+=flight.sh
EXEC+=getarc
EXEC+=help
EXEC+=idx64cfg
EXEC+=install_app
EXEC+=libdoc
EXEC+=mkdoit2
EXEC+=mkpatch
EXEC+=mlf_find
EXEC+=noderpt
EXEC+=prepCD
EXEC+=promote
EXEC+=rcs2html
EXEC+=saverun
EXEC+=tmg2tmc
EXEC+=tmpath
EXEC+=winsetsize
EXEC+=reduce
EXEC+=vt100
EXEC+=ar
EXEC+=cpp
EXEC+=nm
EXEC+=ranlib
EXEC+=tbl2txt
EXEC+=mailpage
EXECXO=Backup3
EXECXO+=Copyin3
EXECXO+=appclone
EXECXO+=archive
EXECXO+=arc
EXECXO+=dupdir
EXECXO+=fld_transfer
EXECXO+=flinit
EXECXO+=hangup
EXECXO+=lprint
EXECXO+=mkdoit
EXECXO+=odinit
EXECXO+=unarc
###################################
# PRIVs are EXECs that get setuid #
###################################
PRIV=demote
PRIV+=distribute
PRIV+=fixdisk
PRIV+=flttime
PRIV+=osupdate
PRIV+=password
PRIV+=pcin pcout
PRIV+=reboot
PRIV+=settime
PRIV+=stopclock
PRIVXO=mount_od
PRIVXO+=umount_od

SRC=libmaint.mk
SRC+=library.mk
SRC+=maint.mk3
SRC+=appgen.mk
SRC+=boot.floppy
SRC+=dccc.por
SRC+=edf2ext.awk
SRC+=edf2oui.awk
SRC+=fld2disp.awk
SRC+=flttime.tmc
SRC+=getopt.c
SRC+=hpfrun.cmd
SRC+=idx64.cmd
SRC+=idx64cfg.awk
SRC+=Makefile.pm
SRC+=mkdoit2.awk
SRC+=mkdoit2.sh
# distribute to /usr/local/sbin
SRC+=noderpt.awk
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
SRCX=deluser
SRCX+=oldfiles
SRCX+=nm.awk
SRCX+=netinstall
SRCX+=netinstall5
SRCX+=newboot
SRCX+=noderpt.alive
SRCX+=toc2html
SRCX+=osupdate.test
SRCX+=maildb.pl maildb.src
# actual maildb source lives in //1/usr/local/sendmail
SRCX+=tabletest rcsupdate
SRCX+=resynch
SRCX+=SetupBackup
SRCX+=VerifyBackup
SRCX+=tapepipe
SRCX+=parsetc
SRCX+=getarcsum
SRCX+=BackTrack

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
SRCXO+=copyin.mk
SRCXO+=cycle.awk
SRCXO+=prt2cmd.awk
SRCXO+=prt2dccc.awk
SRCXO+=prt2edf.awk
SRCXO+=prt2tmc.awk
SRCXO+=fixser

# moved to web/orch
SRCXO+=makepage
SRCXO+=makepages
SRCXO+=editfile.bat


TOOL=Makefile
TOOL+=rcs2html.dat
TOOL+=todo
TOOL+=todo.getarc

HOMEDIR=/usr/local/src/libsrc
MNC=libsrc
SOURCE=$(EXEC) $(PRIV) $(SRC) $(SRCCFG) $(SRCX) $(SRCXO) $(TOOL)
OBJECT=maildb.text source.html sourcel.html source.txt
TARGET=$(SRCO)
TGTNODE=//1
LIBSRCDIR=$(TGTNODE)/usr/local/lib/src
BINDIR=$(TGTNODE)/usr/local/bin
DISTRIB=$(SRC)

SU=sudo sh

all : $(EXEC) $(PRIV) $(SRCCFG)
	@find . -name promote -perm -4000 -perm 771 \
	  \( ! -perm -2000 \) -user root -group root >/dev/null ||\
	  { echo Make promote setuid: && \
	    $(SU) -c "chmod u=rwx,g=rwx,o=x promote; promote promote"; \
	  }
	promote $(PRIV)
	promote netmaptrap
	@for i in $(SRCCFG); do\
	  if cmp -s $$i //1/etc/config/$$i; then\
		echo $$i is current;\
	  else echo WARNING: $$i differs from //1/etc/config/$$i ;\
	  fi;\
	done; :

# @echo "Update executables in /usr/local/bin: \\c"; \
#  $(SU) -c "ln -i $(EXEC) $(PRIV) /usr/local/bin"
# @find . -name doc -group man \
#   -perm -2755 ! -perm -4000 >/dev/null ||\
#   $(SU) -c "chgrp man doc; chmod u=rwx,g=rxs,o=rx doc"
# chmod g+s archive

distribution : $(DISTRIB) all
	@if [ ! -d $(LIBSRCDIR) ]; then mkdir -p $(LIBSRCDIR); fi; :
	$(SU) -c "cp -vcpni $(DISTRIB) $(LIBSRCDIR)"; :
	$(SU) -c "cp -vcpni $(EXEC) $(PRIV) $(BINDIR)"; :

# Disable the default .sh rule
mkdoit2 :
	: ;
source.html : $(SOURCE) RCS/*
	rcs2html /usr/local/src/libsrc
