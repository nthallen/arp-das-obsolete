# library.mk
# Standard Library Stuff.
# Env. Var. WCC32 must be defined when compiling with beta software.
# This will put beta-compiled objects in special directories
OBJMDL=$(MODEL)$(WCC32)
LINC=$(LINCNODE)/usr/local/include
LIB=$(LIBNODE)/usr/local/lib$(WCC32)
MODELARGS=-m$(MODEL) -2
# OPTARGS=-Oatx is good for speed
OPTARGS=-g
CFLAGS=$(MODELARGS) -fo$@ -w4 $(OPTARGS)
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
	@if [ $(MODEL) = 3r -a -z "$(WCC32)" ]; then\
	  echo Cannot compile Model 3r without WCC32 >&2;\
	  exit 1;\
	fi; :
	@if [ ! -d $(OBJMDL) ]; then mkdir $(OBJMDL); fi; :

# This is the target
$(TGT) : $(OBJ)
	@$(BUILDLIB)
