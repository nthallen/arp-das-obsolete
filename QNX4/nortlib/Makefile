# Makefile for nortlib
# Compiling options
MODEL=l
LINC=/usr/local/include
CFLAGS=-m$(MODEL) -w4 -g -I $(LINC)

.c.o:
	$(COMPILE.c) -Wc,-fo$@ $<

#  Maintenance Information
SRC=skeleton.c nl_error.c
SRC+=colsptr.c colrptr.c colsprx.c colrprx.c find_dg.c send_dg.c
SRC+=nlresp.c find_cc.c send_cc.c senddasc.c
TOOL=Makefile
OBJ=$(MODEL)/skeleton.o $(MODEL)/nl_error.o
OBJ+=$(MODEL)/colsptr.o $(MODEL)/find_dg.o $(MODEL)/colrptr.o
OBJ+=$(MODEL)/send_dg.o $(MODEL)/colsprx.o $(MODEL)/colrprx.o
OBJ+=$(MODEL)/nlresp.o $(MODEL)/find_cc.o $(MODEL)/send_cc.o
OBJ+=$(MODEL)/senddasc.o
TARGET=nortlib$(MODEL).lib
TEMP=*.err *.bak *.lib
SAVE=$(SRC) $(TOOL)
PURGE=$(OBJ) $(TEMP)
MNC=nrtl

$(TARGET) : $(OBJ)
	@echo $(OBJ) | xargs -i echo +{} | xargs -t wlib -n $(TARGET)
$(MODEL)/skeleton.o : skeleton.c
$(MODEL)/nl_error.o : nl_error.c
$(MODEL)/colsptr.o : colsptr.c
$(MODEL)/find_dg.o : find_dg.c
$(MODEL)/colrptr.o : colrptr.c
$(MODEL)/send_dg.o : send_dg.c
$(MODEL)/colsprx.o : colsprx.c
$(MODEL)/colrprx.o : colrprx.c
$(MODEL)/nlresp.o : nlresp.c
$(MODEL)/find_cc.o : find_cc.c
$(MODEL)/send_cc.o : send_cc.c
$(MODEL)/senddasc.o : senddasc.c

include /usr/local/include/maint.mk
