CFLAGS = -g
MODEL = s

OBJS = menu.o attr.o scr.o fld.o cfg.o popup.o tabs.o

curses_utils$(MODEL).lib : $(OBJS)
	cp curses_utils$(MODEL).lib /usr/local/lib

.c.o:
	$(COMPILE.c) $<
	wlib curses_utils$(MODEL).lib +-$@

menu.o : menu.c menu.h
attr.o : attr.c attr.h
scr.o  : scr.c scr.h hdr.h
fld.o  : fld.c fld.h hdr.h
cfg.o  : cfg.c cfg.h
popup.o : popup.c popup.h
tabs.o : tabs.c tabs.h
