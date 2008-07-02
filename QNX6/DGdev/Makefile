CC=g++
# CFLAGS=-I/usr/local/include -Wall -lang-c++ -Y _gpp -g
CFLAGS=-I/usr/local/include -Wall -g
LDFLAGS=-L/usr/local/lib -lang-c++ -Y _cpp-ne -M
LIBS=-ltm -lnort
SRCS=test_timer.c DG_cmd.c DG_Resmgr.c
OBJS=$(SRCS:.c=.o)

.PHONY : clean all
all : test_timer
test_timer : $(OBJS)
	cc $(LDFLAGS) -o test_timer $(OBJS) $(LIBS)
clean :
	rm -f $(OBJS) test_timer

test_timer.o : test_timer.c DG_cmd.h DG_tmr.h DG_Resmgr.h
DG_cmd.o : DG_cmd.c DG_cmd.h DG_Resmgr.h
DG_Resmgr.o : DG_Resmgr.c DG_Resmgr.h
