# library.mk
# Standard Library Stuff.
#
# Env. Var. WCC32 may be defined to distinguish between different
# versions of the compiler. Object modules will go in a separate
# directory, although the target library at this point will be
# the same.
#
# OPTARGS=Oatx will now be the default. OPTARGS will not be defined
# in this file but will either be specified as a command-line arg
# or as an environment variable. OPTARGS should be a string of
# option letters without the leading hyphen.
# OPTARGS=g would be used to get debugging info.
#
OBJMDL=obj.$(MODEL)$(WCC32)$(OPTARGS)
LINC=$(LINCNODE)/usr/local/include
LIB=$(LIBNODE)/usr/local/lib$(OPTARGS)
MODELARGS=-m$(MODEL) -2
CFLAGS=$(MODELARGS) -fo$@ -w4 -$${OPTARGS:-Oatx}
BUILDLIB=echo $(OBJ) | xargs -i echo +{} | xargs -t wlib -n $(TGT)
TGT=$(LIB)/$(LIBNAME)$(MODEL).lib
SOURCE=$(SRC) $(TOOL) $(DOC)
OBJECT=$(OBJ) $(TEMP)
TARGET=
.INIT :
	@if [ $(MODEL) = 3r -a "$(MODELARGS)" != "-ms -3" ]; then\
	  echo Must invoke Model 3r via Makelib >&2;\
	  exit 1;\
	fi; :
	@if [ ! -d $(OBJMDL) ]; then mkdir $(OBJMDL); fi; :
	@if [ ! -d $(LIB) ]; then\
	  echo Target directory $(LIB) does not exist >&2;\
	  exit 1;\
	fi; :

# This is the target
$(TGT) : $(OBJ)
	@$(BUILDLIB)
