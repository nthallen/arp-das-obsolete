# library.mk
# Standard Library Stuff.
#
# OPTARGS=Oatx will now be the default. OPTARGS will not be defined
# in this file but will either be specified as a command-line arg
# or as an environment variable. OPTARGS should be a string of
# option letters without the leading hyphen.
# OPTARGS=g would be used to get debugging info.
#
LINC=$(LINCNODE)/usr/local/include
OBJMDL=OBJ.$(MODEL)$(OPTARGS)
LIB=$(LIBNODE)/usr/local/lib$(OPTARGS)
MODELARGS=-m$(MODEL) -2
CFLAGS=$(MODELARGS) -fo$@ -w4 -$${OPTARGS:-Oatx}
BUILDLIB=wlib -n $(TGT) $(OBJ)
TGT=$(LIB)/$(LIBNAME)$(MODEL).lib
SOURCE=$(SRC) $(TOOL) $(DOC)
OBJECT=$(OBJ) $(TEMP)
TARGET=
.INIT :
	@if [ $(MODEL) = 3r -a "$(MODELARGS)" != "-ms -3" ]; then\
	  echo Must invoke Model 3r via Makelib >&2;\
	  exit 1;\
	fi; :
	@if [ -z "$(LIBMAINT)" -a ! -d $(OBJMDL) ]; then mkdir $(OBJMDL); fi; :
	@if [ ! -d $(LIB) ]; then\
	  echo Target directory $(LIB) does not exist >&2;\
	  exit 1;\
	fi; :

# This is the target
$(TGT) : $(OBJ)
	@$(BUILDLIB)
