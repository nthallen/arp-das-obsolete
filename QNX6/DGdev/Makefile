# CC=g++
# CFLAGS=-I/usr/local/include -Wall -lang-c++ -Y _gpp -g
#VER=2.95.3
VER=3.3.5
TGT=-V$(VER),gcc_ntox86_cpp -lang-c++
CFLAGS=$(TGT) -I/usr/local/include -Wall -g
LDFLAGS=-L/usr/local/lib -M -Wl,-rpath -Wl,/usr/local/lib
LIBS=-ltm -lnort
SRCS=test_timer.c DG_cmd.c DG_Resmgr.c
OBJS=$(SRCS:.c=.o)

.PHONY : clean all
all : test_timer
test_timer : $(OBJS)
	cc $(CFLAGS) $(LDFLAGS) -o test_timer $(OBJS) $(LIBS)
clean :
	rm -f $(OBJS) test_timer

test_timer.o : test_timer.c DG_cmd.h DG_tmr.h DG_Resmgr.h
DG_cmd.o : DG_cmd.c DG_cmd.h DG_Resmgr.h
DG_Resmgr.o : DG_Resmgr.c DG_Resmgr.h
