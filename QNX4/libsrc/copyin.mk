# copyin.mk Support makefile for Copyin3
#
include Makefile

MAKEN=make -f /usr/local/lib/src/copyin.mk $(MAKEFLAGS)

# Define this to suppress creation of library object directories
LIBMAINT=yes

TESTHDIR=\
if test -z "$(HOMEDIR)"; then \
  echo $$PWD: No HOMEDIR set in Makefile; exit 1; \
fi; :

# MNC_Report simply outputs the MNC, which is useful for copyin
# operations
MNC_Report :
	@echo $(MNC)

# copyin     copy (or link) files from backup directory
#            copyin is run in the backup directory
# Copyin     Recursively copyin
copyin :
	@$(TESTHDIR)
	@if [ ! -d $(HOMEDIR) ]; then\
	  echo $$PWD: Creating directory $(HOMEDIR);\
	  if mkdir -p -m g+rwx,o+rx $(HOMEDIR); then :; else exit 1; fi;\
	fi; :
	@if [ $(LEVEL) ]; then\
	  TFRT=.backup_;\
	  ( cd $(HOMEDIR);\
		for i in $$TFRT*; do\
		  if [ -f $$i ]; then\
			if [ "$${i##$$TFRT}" -ge $(LEVEL) ]; then rm -v $$i; fi;\
		  fi;\
		done;\
	  ) ;\
	  i=0;\
	  while [ $$i -le $(LEVEL) ]; do\
		if [ -f $$TFRT$$i ]; then\
		  if [ -f $(HOMEDIR)/$$TFRT$$i ]; then\
			until diff $$TFRT$$i $(HOMEDIR)/$$TFRT$$i; do\
			  echo Backups at level $$i do not agree.;\
			  exit 1;\
			done;\
		  fi;\
		  ln -f $$TFRT$$i $(HOMEDIR);\
		fi;\
		let i=$$i+1;\
	  done;\
	fi; :
	@if [ $(SOURCE) ]; then\
	  for j in $(SOURCE); do\
		if [ -f $$j ]; then\
		  if [ ! -f $(HOMEDIR)/$$j ]; then i="y";\
		  elif [ $$j -nt $(HOMEDIR)/$$j ]; then\
			if [ "$(RAMBO)" = "yes" ]; then i="y";\
			else\
			  echo $$PWD:$(HOMEDIR): Overwrite $$j? \(N/y\) \\c;\
			  read i;\
			fi;\
		  else\
			if [ $$j -ot $(HOMEDIR)/$$j ]; then\
			  echo "$$PWD:$(HOMEDIR):\\n  Archived file $$j out of date";\
			else ln -f $(HOMEDIR)/$$j $$j;\
			fi;\
			i="n";\
		  fi;\
		  case $$i in \
			[yY]*) echo $$PWD:$(HOMEDIR): Linking $$j; ln -f $$j $(HOMEDIR);;\
		  esac; \
		fi;\
	  done;\
	fi; :
	@if [ -d RCS ]; then\
	  if [ ! -d $(HOMEDIR)/RCS ]; then \
		echo $$PWD: Making directory $(HOMEDIR)/RCS;\
		until mkdir -p -m g+rwx,o+rx $(HOMEDIR)/RCS; do exit 1; done;\
	  fi;\
	  for j in RCS/*; do\
		if [ -f $$j ]; then\
		  if [ \( ! -f $(HOMEDIR)/$$j \) ]; then i="y";\
		  elif [ $$j -nt $(HOMEDIR)/$$j ]; then\
			if [ "$(RAMBO)" == "yes" ]; then i="y";\
			else\
			  echo $$PWD:$(HOMEDIR): Overwrite $$j? \(N/y\) \\c;\
			  read i;\
			fi;\
		  else\
			if [ -f $(HOMEDIR)/$$j ]; then\
			  if [ $$j -ot $(HOMEDIR)/$$j ]; then\
				echo "$$PWD:$(HOMEDIR):\\n  Archived file $$j out of date";\
			  else ln -f $(HOMEDIR)/$$j $$j;\
			  fi;\
			  i="n";\
			fi;\
		  fi;\
		  case $$i in \
			[yY]*) echo $$PWD:$(HOMEDIR): Linking $$j; ln -f $$j $(HOMEDIR)/RCS;;\
		  esac; \
		fi;\
	  done;\
	fi; :
	@touch copyin
Copyin : copyin
	@if [ $(SUBDIRS) ]; then\
	  sd="";\
	  for i in $(SUBDIRS); do\
		if [ -z "$$sd" ]; then sd=$$i;\
		else\
		  if [ -d $$i ]; then\
			if [ $$MKVERBOSE ]; then\
			  echo $$PWD: making Copyin in $$i;\
			fi;\
			until (cd $$i && $(MAKEN) BROOT=$(BDIR) BMNC=$$i Copyin); do\
			  echo $$PWD: Error making subdirectory $$i; exit 1;\
			done;\
		  fi;\
		  sd="";\
		fi;\
	  done;\
	fi; :
