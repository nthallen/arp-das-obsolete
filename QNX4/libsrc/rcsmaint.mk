# make rcscheck
#  checks out any modules not present
#  compares all $(SOURCE) modules against RCS files
# todo:
# if [ -f $(PWD)/$i -a $i -nt $(PWD)/$i -a ! -w $(PWD)/$i ]
# if dest file exists and is older than source and it is unwritable {
# 	Lock/Skip/Copy
# }
RCS=$(SOURCE)

include Makefile

rcscheck :
	@if [ -z "$(RCS)" ]; then\
	  echo No RCS files specified; exit 1; fi; :
	@if [ ! -d RCS ]; then\
	  echo Making RCS directory;\
	  mkdir RCS; fi; :
	@for i in $(RCS); do \
	  if [ -f RCS/$${i},v ]; then \
		if [ -f $${i} ]; then \
		  echo Checking $$i ; \
		  rcsdiff -q $$i >/dev/null 2>&1 ; \
		  case $$? in \
			0) ;; \
			1) ismine=`rlog -L -R -l${LOGNAME} $$i`; \
			   case $$ismine in \
				 ?*) echo Do you want to check it in? \\c; \
				    read ismine ; \
					case $$ismine in \
					  [yY]*) ci -l $$i ;; \
					esac;; \
				 *) echo Do you want to check it out? \\c; \
				    read ismine ; \
					case $$ismine in \
					  [yY]*) co -r$$revlev $$i ;; \
					esac;; \
			   esac;; \
			*) rcsdiff -q $$i >/dev/null ;; \
		  esac; \
		else co -u $$i; \
		fi; \
	  else\
		echo "\nCreating archive for $$i:" ; \
		ci -l $$i;\
	  fi; \
	done; :
	@for i in RCS/*; do\
	  k="";\
	  if [ -f $$i ]; then\
		for j in $(RCS); do \
		  : ; \
		  if [ "RCS/$$j,v" = "$$i" ]; then k="yes"; break; fi; \
		done;\
		if [ "$$k" != "yes" ]; then\
		  echo $$i is not a source file: Delete it?;\
		  read k ; \
		  case $$k in \
			[yY]*) rm -vf $$i ;; \
		  esac; \
		fi;\
	  fi;\
	done; :

# For freeze functionality:
#  would want to first check that latest versions are checked in
#  via rcscheck, then go through and set the symbolic rev to the
#  rev of the head in each case.

rcsfreeze :
	@if [ -z "$(REV)" ]; then echo No REV defined; exit 1; fi; :
	@for i in RCS/*; do\
	  if [ -f $$i ]; then\
		j=`rlog $$i | awk '/^head:/ { print $$NF; exit 0 }'`;\
		echo $$i $(REV) = $$j;\
		rcs -N$(REV):$$j $$i;\
	  fi;\
	done; :
