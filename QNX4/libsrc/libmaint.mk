# libmaint.mk is specific to making libraries
# Should be referenced from /usr/local/bin/Makelib:
#   make -f /usr/local/include/libmaint.mk $*
# Usage:
#   Makelib [every]
#     Makes every model specified in MODELS
#   Makelib clean|tidy
#     Invokes maint.mk2 for each model.
# $Log$
# Revision 1.3  1994/04/18  17:47:46  nort
# Removed obsolete references to WCC32.
# Elminated directory creation, since that is handled in library.mk
# Elminated all reference to directory names since those are
# specified in library.mk and would be pathalogical.
#
# Revision 1.2  1994/02/16  02:07:25  nort
# *** empty log message ***
#
# Revision 1.1  1993/02/09  14:58:12  eil
# Initial revision
#
every :
	@if [ -z "$(MODELS)" ]; then\
	  echo MODELS not defined >&2;\
	  exit 1;\
	fi; :
	@for i in $(MODELS); do\
	  echo Making MODEL $${i};\
	  if [ $$i = 3r ]; then $(MAKE) MODEL=$$i MODELARGS="-ms -3";\
	  else $(MAKE) MODEL=$$i;\
	  fi;\
	done; :
clean tidy :
	@if test -z "$(MODELS)"; then echo MODELS not defined; exit 1; fi; :
	@for i in $(MODELS); do\
	  if [ $$i = 3r ]; then Make MODEL=$$i MODELARGS="-ms -3" $@;\
	  else Make MODEL=$$i $@;\
	  fi;\
	done; :
include Makefile
