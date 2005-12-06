SRCS = lgr.c lgr_dc.c lgroui.c
OBJS = $(SRCS:.c=.o)
LDFLAGS = -l dfs_mod

lgr : $(OBJS)
	$(LINK.c) -o lgr $(OBJS)
	usemsg lgr lgroui.c
lgroui.c : lgr.oui
	oui -o lgroui.c lgr.oui

clean :
	rm *.o lgroui.c lgr
