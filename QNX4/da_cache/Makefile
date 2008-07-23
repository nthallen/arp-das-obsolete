HOMEDIR=/usr/local/src/das/cache
MNC=cache
TOOL=Makefile todo
SRC=da_cache.c da_cache_int.h da_cache.oui
OBJ=da_cache.o da_cacheoui.o
TEMP=da_cacheoui.c
TGT=da_cache
MODEL=s
CPU=-2

SOURCE=$(SRC) $(TOOL)
OBJECT=$(OBJ) $(TEMP)
TARGET=$(TGT)

CFLAGS=-w4 -m$(MODEL) $(CPU) -g
LDFLAGS=-g
INC=/usr/local/include
LIBFLAGS=-l eillib -l das $${BETALIB:--b}
LINK.priv=/bin/rm -f $@; $(LINK.c) $(LIBFLAGS) -T 1 -o $@ $(LDFLAGS)

da_cache : $(OBJ)
	$(LINK.priv) $(OBJ)
	usemsg da_cache da_cacheoui.c
	promote da_cache

da_cache.o : da_cache.c da_cache_int.h $(INC)/da_cache.h $(INC)/nortlib.h
da_cacheoui.o : da_cacheoui.c da_cache_int.h $(INC)/da_cache.h $(INC)/nortlib.h
da_cacheoui.c : da_cache.oui
	oui -o da_cacheoui.c da_cache.oui
