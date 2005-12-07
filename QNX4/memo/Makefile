LDFLAGS += -N5k
SRCS = memo.c memooui.c
OBJS = $(SRCS:.c=.o)

memo : $(OBJS)
	$(LINK.c) -o memo $(OBJS)
	usemsg memo memooui.c
memooui.c : memo.oui
	oui -o memooui.c memo.oui
