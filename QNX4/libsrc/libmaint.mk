# libmaint.mk is specific to making libraries
# Should be referenced from /usr/local/bin/Makelib:
#   make -f /usr/local/include/libmaint.mk $*
# Usage:
#   Makelib [every]
#     Makes every model specified in MODELS
#   Makelib clean|tidy
#     Invokes maint.mk2 for each model.
# $Log$
every :
	@if test -z "$(MODELS)"; then echo MODELS not defined; exit 1; fi; :
	@for i in $(MODELS); do\
	  if test ! -d $$i; then\
		echo Making directory for model $$i;\
		mkdir -m g+w $$i;\
	  fi;\
	  echo Making MODEL $$i;\
	  $(MAKE) MODEL=$$i;\
	done; :
clean tidy :
	@if test -z "$(MODELS)"; then echo MODELS not defined; exit 1; fi; :
	@for i in $(MODELS); do\
	  if test ! -d $$i; then\
		echo Directory for model $$i does not exist;\
	  else Make MODEL=$$i $@;\
	  fi;\
	done; :
include Makefile
