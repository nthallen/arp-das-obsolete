# make rcscheck
#  checks out any modules not present
#  compares all $(SOURCE) modules against RCS files
# todo:
# if test -f $(PWD)/$i -a $i -nt $(PWD)/$i -a ! -w $(PWD)/$i
# if dest file exists and is older than source and it is unwritable {
# 	Lock/Skip/Copy
# }
include Makefile

rcscheck :
	@if test -z "$(SOURCE)"; then\
	  echo No SOURCE files specified; exit 1; fi; :
	@if test ! -d RCS; then\
	  echo Making RCS directory;\
	  mkdir RCS; fi; :
	@for i in $(SOURCE); do \
	  if test -f RCS/$${i},v; then \
		if test -f $${i}; then \
		  echo Checking $$i ; \
		  rcsdiff -q $$i >/dev/null 2>&1 ; \
		  case $$? in \
			0) ;; \
			1) ismine=`rlog -L -R -l${LOGNAME} $$i`; \
			   case $$ismine in \
				 ?*) echo Do you want to check it in? ; \
				    read ismine ; \
					case $$ismine in \
					  [yY]*) ci -l $$i ;; \
					esac;; \
				 *) echo Do you want to check it out? ; \
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
	  if test -f $$i; then\
		for j in $(SOURCE); do \
		  : ; \
		  if test "RCS/$$j,v" = "$$i"; then k="yes"; break; fi; \
		done;\
		if test "$$k" != "yes"; then\
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
	@if test -z "$(REV)"; then echo No REV defined; exit 1; fi; :
	@for i in RCS/*; do\
	  if test -f $$i; then\
		j=`rlog $$i | awk "/^head:/ { print \$$NF; exit 0 }"`;\
		echo $$i $(REV) = $$j;\
		rcs -N$(REV):$$j $$i;\
	  fi;\
	done; :
