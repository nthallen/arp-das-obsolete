#-----------------------------------------------------------------
# $Id$
#
# Maintainance targets:
#	make clean
#	make exchange
#	make backup
#	make archive
#	make backup archive
#	make update
#	make archlist
#   make rcscheck
# This set of commands should be entirely generic given the
# definition of the following macros:
#   MNC the name of the backup directory. This should be
#       unique system-wide, since it will probably go into
#       the same directory with all the other backup directories
#   SUBDIRS list of subdirectories that should be grouped together
#       with this one for maintenance purposes.
#   INCLUDED list of files from /usr/local/include which are
#       used in this directory. This will be copied together
#       with other source files into an "included" subdirectory
#       when the backup target is invoked.
#	SAVE list of files which are to be saved during backup
#	PURGE list of files which are to be deleted during cleanup
#	SAVE and PURGE should list all files in the directory.
#		make unlisted (or exchange) will alert you to any
#		files not listed in one or the other category.
#-----------------------------------------------------------------
# BACKUPDIR is where duplicate copies of source files are kept
BACKUPROOT=$(TMPDIR)
BACKUPDIR=$(BACKUPROOT)/$(MNC)
ULRT=$(TMPDIR)/$(MNC)
RECURSE=if test -n "$(SUBDIRS)"; then\
	  for i in $(SUBDIRS); do\
	    if (cd $$i; $(MAKE) $(MAKEFLAGS) BACKUPROOT=$(BACKUPDIR) $@);\
		then :;\
		else exit 1;\
		fi;\
	  done;\
	else :;\
	fi;

# make clean reduces to source code only
clean :
	@$(RECURSE)
	rm $(PURGE)
	rm -rf $(BACKUPDIR)

# make exchange exchanges newer versions of source with a backup directory.
# make backup copies the RCS files also. make backup archive should therefore
# backup the current source AND the RCS files.
copyout :
	@if test ! -d $(BACKUPDIR); then mkdir -p $(BACKUPDIR); fi; :
	@if test ! -d $(BACKUPDIR); then echo Unable to create $(BACKUPDIR); FALSE; else : ; fi
	@cp -cfvn $(SAVE) $(BACKUPDIR); :
	@$(RECURSE)
copyin :
	@if test ! -d $(BACKUPDIR);\
	  then echo Directory $(BACKUPDIR) does not exist; FALSE; else : ; fi
	@cd $(BACKUPDIR); find . -level 1 -a -type f | xargs -i cp -civn {} $(PWD); :
	@$(RECURSE)
backup : copyout
	@if test -n "$(INCLUDED)"; then\
	  cd /usr/local/include; cp -cvn $(INCLUDED) $(BACKUPDIR)/included;\
	fi; :
	@if test -d RCS; then\
	  cp -cvn RCS/* $(BACKUPDIR)/RCS;\
	fi; :
	@$(RECURSE)

# make archive will copy all the source stuff to a floppy archive.
archive : unlisted copyout
	cd $(BACKUPDIR); pax -wv . | freeze | vol -w /dev/fd0
	if test -d $(BACKUPDIR)/RCS; then rm -rf $(BACKUPDIR)/RCS; fi; :
	if test -d $(BACKUPDIR)/included; then rm -rf $(BACKUPDIR)/included; fi; :

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
unlisted :
	@echo Checking for unlisted files in $(PWD)/Makefile:
	@find . -level 1 -a -type f -a ! -name \*.TMP | sort >$(ULRT)files
	@(for i in $(SAVE) $(PURGE); do echo ./$$i; done) | sort >$(ULRT)list
	@diff $(ULRT)files $(ULRT)list | grep "^<" > $(ULRT)unlisted; :
	@rm $(ULRT)files $(ULRT)list
	@if test -s $(ULRT)unlisted; then cat $(ULRT)unlisted; rm $(ULRT)unlisted; false; else rm $(ULRT)unlisted; fi
	@$(RECURSE)

# make rcscheck
#  checks out any modules not present
#  compares all $(SAVE) modules against RCS files
# todo:
# if test -f $(PWD)/$i -a $i -nt $(PWD)/$i -a ! -w $(PWD)/$i
# if dest file exists and is older than source and it is unwritable {
# 	Lock/Skip/Copy
# }
rcscheck :
	@for i in $(SAVE); do \
	  if test -f RCS/$${i},v; then \
		if test -f $${i}; then \
		  echo Checking $$i ; \
		  rcsdiff -q $$i >/dev/null 2>&1 ; \
		  case $$? in \
			0) ;; \
			1) ismine=`rlog -L -R -l${LOGNAME} $$i`; \
			   case $$ismine in \
				 ?*) echo Do you wan''t to check it in? ; \
				    read ismine ; \
					case $$ismine in \
					  [yY]*) ci -l $$i ;; \
					esac;; \
				 *) echo Do you wan''t to check it out? ; \
				    read ismine ; \
					case $$ismine in \
					  [yY]*) co -r$$revlev $$i ;; \
					esac;; \
			   esac;; \
			*) rcsdiff -q $$i >/dev/null ;; \
		  esac; \
		else co -u $$i; \
		fi; \
	  else echo Not archived with RCS: $$i ; \
	  fi; \
	done; :
