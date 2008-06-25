test_timer : test_timer.o
CC=g++
CFLAGS=-I/usr/local/include -Wall
LDFLAGS=-L/usr/local/lib -ltm -lnort
test_timer.o : test_timer.c DG_cmd.h DG_tmr.h

.PHONY : clean
clean :
	rm *.o test_timer
