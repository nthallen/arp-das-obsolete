# makefile for /usr/local/include
# $Log$
SRC=das.h dbr_utils.h dac.h dbr.h das_utils.h globmsg.h file.h mod_utils.h
SRC+=msg.h sights.h sounds.h symname.h timing.h usage.h curses.h company.h
SRC+=lat.h cmdctrl.h
TOOL=makefile
DOC=
OBJ=
INTERMED=
TEMPORARY=

all : archive

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
MNC=linc
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
	@echo Checking for unlisted files in tmc/makefile:
	@find . -level 1 -a -type f -a ! -name \*.TMP | sort >$(BACKUPDIR)files
	@(for i in $(SAVE) $(PURGE); do echo ./$$i; done) | sort >$(BACKUPDIR)list
	@diff $(BACKUPDIR)files $(BACKUPDIR)list | grep "^<" > $(BACKUPDIR)unlisted; :
	@rm $(BACKUPDIR)files $(BACKUPDIR)list
	@if test -s $(BACKUPDIR)unlisted; then cat $(BACKUPDIR)unlisted; rm $(BACKUPDIR)unlisted; false; else rm $(BACKUPDIR)unlisted; fi
