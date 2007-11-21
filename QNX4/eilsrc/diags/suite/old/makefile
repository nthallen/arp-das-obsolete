Model = s
CFLAGS = -g

all: syscon card sccfg

OBJS = suite.o attribut.o diagutil.o
card : card.o $(OBJS)
	$(CC) $(CFLAGS) -l curses -l subbus -l lat -o card card.o $(OBJS)

syscon : syscon.o $(OBJS)
	$(CC) $(CFLAGS) -l curses -l subbus -l lat -o syscon syscon.o $(OBJS)

card.o : syscon.c syscon.h attribut.h scdiag.h define.h diagmenu.h
	$(CC) $(CFLAGS) -DCARD -Wc,-fo=card.o -c syscon.c

syscon.o : syscon.c syscon.h attribut.h scdiag.h define.h diagmenu.h
	$(CC) $(CFLAGS) -DSYSCON -c syscon.c

sccfg : sccfg.o attribut.o
	$(CC) $(CFLAGS) -o sccfg -l curses -l lat sccfg.o attribut.o

scdiag.o : scdiag.asm
suite.o : suite.c syscon.h attribut.h scdiag.h 
diagutil.o: diagutil.c define.h
attribut.o : attribut.c attribut.h
sccfg.o : sccfg.c attribut.h

sbdb : sbdb.o
    
sbdb.o : sbdb.c reslib.h subbus.h adu.h

digio : digio.o adu.lib reslib.lib

digio.o : digio.c reslib.h subbus.h
io.o : io.c
io : io.o testport.o
mem.o : mem.c
mem : mem.o testmem.o
testmem.o : testmem.asm
testport.o : testport.asm

dtoatest : dtoatest.o reslib.lib adu.lib

dtoatest.o : dtoatest.c reslib.h subbus.h adu.h

cttest : cttest.o reslib.lib adu.lib
cttest.o : cttest.c reslib.h subbus.h adu.h
