LDFLAGS=-lnort -ltmph -lsocket
CFLAGS=-I/usr/local/include -g
INETSRC=Inetin.c Inet.c Inetinoui.c
INETOBJ=$(INETSRC:.c=.o) 
Inetin : $(INETOBJ)
$(INETOBJ) : dbr.h globmsg.h

CRSRC=CRphdisp.c DC.c
CROBJ=$(CRSRC:.c=.o) 
CRphdisp : $(CROBJ)
$(CROBJ) : dbr.h globmsg.h

INET2SRC=Inetin2.c Inet.c Inetinoui.c
INET2OBJ=$(INET2SRC:.c=.o) 
Inetin2 : $(INET2OBJ)
$(INET2OBJ) : dbr.h globmsg.h

tryTM : tryTM.c
