# makefile for das/dbr/utils
# $Id$
# $Log$
# Revision 1.4  1992/06/09  15:08:48  nort
# Modified .DEFAULT rule; Removed "all" target
#
# Revision 1.3  1992/05/20  19:53:01  nort
# Added .DEFAULT rule for automatic RCS check-out
#
# Revision 1.2  1992/05/20  17:27:39  nort
# Added maintenance targets.
# Added dgdacin.
#
# Revision 1.1  1992/05/19  14:20:47  nort
# Initial revision
#
MODEL=s
CFLAGS = -g -m$(MODEL) -I /usr/local/include

SRC=dacin.c dacoff.c db.c dbr_info.c dc.c dg.c dr.c dgdacin.c
TOOL=makefile
DOC=dc.txt dg.txt report todo
OBJ=dacin.o dacoff.o db.o dbr_info.o dc.o dg.o dr.o dgdacin.o
INTERMED=dbr_utils$(MODEL).lib
TEMPORARY=dbr_utils$(MODEL).bak
INCLUDED=dac.h dbr.h dbr_utils.h globmsg.h das_utils.h msg.h sounds.h
INCLUDED+=usage.h symname.h cmdctrl.h

.DEFAULT:
	if test ! -f $@; then co -u $@; fi; :

dbr_utils$(MODEL).lib : $(OBJ)
	ln dbr_utils$(MODEL).lib /usr/local/lib

check :
	checkup $(SRC) $(TOOL) $(DOC)

.c.o:
	$(COMPILE.c) $<
	wlib dbr_utils$(MODEL).lib +-$@

dg.o : dg.c

dc.o : dc.c

db.o : db.c

dr.o : dr.c

dacin.o : dacin.c /usr/local/include/dac.h

dacoff.o : dacoff.c /usr/local/include/dac.h

dbr_info.o : dbr_info.c

dgdacin.o : dgdacin.c

#-----------------------------------------------------------------
# Maintainance targets:
#	make clean
#	make exchange
#	make backup
#	make archive
#	make backup archive
#	make update
#	make archlist
#-----------------------------------------------------------------
# BACKUPDIR is where duplicate copies of source files are kept
MNC=dbru
BACKUPDIR=/tmp/$(MNC)
SAVE=$(SRC) $(TOOL) $(DOC)
PURGE=$(OBJ) $(INTERMED) $(TEMPORARY)
# make clean reduces to source code only
# Add commands to recurse as necessary:
#	cd regress; $(MAKE) $(MAKEFLAGS) clean
clean :
	rm $(PURGE)
	rm -r $(BACKUPDIR)

# make exchange exchanges newer versions of source with a backup directory.
# make backup copies the RCS files also. make backup archive should therefore
# backup the current source AND the RCS files.
# add commands to recurse through subdirectories as necessary:
#	cd regress; $(MAKE) $(MAKEFLAGS) exchange
exchange :
	@cp -civn $(SAVE) $(BACKUPDIR); :
	@cd /usr/local/include; cp -civn $(INCLUDED) $(BACKUPDIR)/included; :
	@cd $(BACKUPDIR); find . -level 1 -a -type f | xargs -i cp -civn {} $(PWD); :
backup : exchange
	if test -d $(BACKUPDIR)/RCS; then : ; else mkdir $(BACKUPDIR)/RCS; fi; :
	cp RCS/* $(BACKUPDIR)/RCS

# make archive will copy all the source stuff to a floppy archive.
archive : unlisted exchange
	cd $(BACKUPDIR); pax -wv . | freeze | vol -w /dev/fd0
	if test -d $(BACKUPDIR)/RCS; then rm -rf $(BACKUPDIR)/RCS; fi; :

# make update should read an archive off of floppy into the BACKUPDIR
# make update exchange should then exchange that info with the current
# info. It does not copy the RCS stuff if it was a "backup archive"
update :
	if test -d $(BACKUPDIR); then : ; else mkdir $(BACKUPDIR); fi; :
	cd $(BACKUPDIR); vol -r /dev/fd0 | melt | pax -rov
archlist :
	vol -r /dev/fd0 | melt | pax -v

# make unlisted determines whether the macros for files to be saved
# or purged define all the files in the directory.
# add commands to recurse through subdirectories as necessary:
#	@cd regress; $(MAKE) $(MAKEFLAGS) unlisted
unlisted :
	@echo Checking for unlisted files in ${PWD}/makefile:
	@find . -level 1 -a -type f -a ! -name \*.TMP | sort >$(BACKUPDIR)files
	@(for i in $(SAVE) $(PURGE); do echo ./$$i; done) | sort >$(BACKUPDIR)list
	@diff $(BACKUPDIR)files $(BACKUPDIR)list | grep "^<" > $(BACKUPDIR)unlisted; :
	@rm $(BACKUPDIR)files $(BACKUPDIR)list
	@if test -s $(BACKUPDIR)unlisted; then cat $(BACKUPDIR)unlisted; rm $(BACKUPDIR)unlisted; false; else rm $(BACKUPDIR)unlisted; fi
