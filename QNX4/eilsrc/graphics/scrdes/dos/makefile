LIBS = -l curses_utils -l curses -l lat
OBJS = scrdes.o draw_lin.o draw.o help.o
CFLAGS = -g


scrdes : $(OBJS)
   	$(CC) $(CFLAGS) $(LIBS) -o scrdes -N8k $(OBJS)
scrdes.o : scrdes.c globals.h etc.h pattr.h help.h version.h draw_lin.h

help.o : help.c help.h mnuglobs.h
draw_lin.o : draw_lin.c draw_lin.h globals.h
draw.o : draw.c globals.h etc.h fldglobs.h bufglobs.h txtglobs.h
