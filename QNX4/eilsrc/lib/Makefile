include $(SRCDIR)/src.mk
BASE = eillib
HOMEDIR = $(SRCDIR)/lib

# file list macros
TXTS =  msg.txt serial.txt
HDRS += msg.h serial.h symname.h usage.h vector.h company.h sounds.h \
reply.h get_priv.h reboot.h sound_esc.h sigs.h qnx_ipc.h mx.h fd.h
SRCS =  msg.c msg_options.c serial.c serial_options.c symname.c usage.c \
vector.c get_priv.c sound_esc.c sigs.c \
fd_write.c fd_read.c fd_rdmx.c fd_wrmx.c fd_blk.c \
qnx_ipc_init.c qnx_rec.c qnx_snd.c qnx_rep.c qnx_recmx.c qnx_sndmx.c \
qnx_repmx.c qnx_name_a.c qnx_name_d.c qnx_name_l.c qnx_pflags.c qnx_trig.c \
qnx_ipc.c qnx_ipc_tmp.c qnx_ipc_end.c qnx_proxy_a.c qnx_proxy_d.c \
get_name_entry.c put_name_entry.c
OBJS = $(MODEL)/msg.o $(MODEL)/msg_options.o $(MODEL)/serial.o \
$(MODEL)/serial_options.o $(MODEL)/symname.o $(MODEL)/usage.o \
$(MODEL)/vector.o $(MODEL)/get_priv.o $(MODEL)/sound_esc.o $(MODEL)/sigs.o \
$(MODEL)/fd_read.o $(MODEL)/fd_rdmx.o $(MODEL)/fd_wrmx.o \
$(MODEL)/fd_write.o $(MODEL)/fd_blk.o \
$(MODEL)/qnx_ipc_init.o $(MODEL)/qnx_rec.o $(MODEL)/qnx_snd.o \
$(MODEL)/qnx_rep.o $(MODEL)/qnx_recmx.o $(MODEL)/qnx_sndmx.o \
$(MODEL)/qnx_repmx.o $(MODEL)/qnx_name_a.o $(MODEL)/qnx_name_d.o \
$(MODEL)/qnx_name_l.o $(MODEL)/qnx_pflags.o $(MODEL)/qnx_trig.o \
$(MODEL)/qnx_ipc.o $(MODEL)/qnx_ipc_tmp.o $(MODEL)/qnx_relay.o \
$(MODEL)/qnx_proxy_a.o $(MODEL)/qnx_proxy_d.o $(MODEL)/qnx_ipc_end.o \
$(MODEL)/get_name_entry.o $(MODEL)/put_name_entry.o
USES =
DOCS = sound.h

include $(SRCDIR)/lib.mk

$(MODEL)/serial.o : serial.c serial.h msg.h
$(MODEL)/serial_options.o : serial_options.c serial.h msg.h vector.h
$(MODEL)/msg.o : msg.c msg.h sounds.h symname.h $(INCDIR)/globmsg.h
$(MODEL)/msg_options.o : msg_options.c msg.h
$(MODEL)/symname.o : symname.c symname.h 
$(MODEL)/usage.o : usage.c usage.h
$(MODEL)/vector.o : vector.c vector.h
$(MODEL)/get_priv.o : get_priv.c get_priv.h
$(MODEL)/sound_esc.o : sound_esc.c sound_esc.h
$(MODEL)/sigs.o : sigs.c sigs.h
$(MODEL)/fd_read.o: fd_read.c fd.h
$(MODEL)/fd_rdmx.o: fd_rdmx.c fd.h mx.h
$(MODEL)/fd_wrmx.o: fd_wrmx.c fd.h mx.h
$(MODEL)/fd_write.o: fd_write.c fd.h mx.h
$(MODEL)/fd_blk.o: fd_blk.c fd.h
$(MODEL)/qnx_ipc.o: qnx_ipc.c qnx_ipc.h
$(MODEL)/qnx_ipc_init.o: qnx_ipc_init.c qnx_ipc.h
$(MODEL)/qnx_ipc_tmp.o: qnx_ipc_tmp.c qnx_ipc.h
$(MODEL)/qnx_rec.o: qnx_rec.c qnx_ipc.h
$(MODEL)/qnx_snd.o: qnx_snd.c qnx_ipc.h
$(MODEL)/qnx_rep.o: qnx_rep.c qnx_ipc.h
$(MODEL)/qnx_recmx.o: qnx_recmx.c mx.h qnx_ipc.h
$(MODEL)/qnx_sndmx.o: qnx_sndmx.c mx.h qnx_ipc.h
$(MODEL)/qnx_repmx.o: qnx_repmx.c mx.h qnx_ipc.h
$(MODEL)/qnx_name_a.o: qnx_name_a.c qnx_ipc.h
$(MODEL)/qnx_name_d.o:qnx_name_d.c  qnx_ipc.h
$(MODEL)/qnx_name_l.o:qnx_name_l.c  qnx_ipc.h
$(MODEL)/qnx_pflags.o: qnx_pflags.c  qnx_ipc.h
$(MODEL)/qnx_relay.o: qnx_relay.c  qnx_ipc.h
$(MODEL)/qnx_trig.o: qnx_trig.c  qnx_ipc.h
$(MODEL)/qnx_proxy_a.o: qnx_proxy_a.c  qnx_ipc.h
$(MODEL)/qnx_proxy_d.o: qnx_proxy_d.c  qnx_ipc.h
$(MODEL)/qnx_ipc_end.o: qnx_ipc_end.c  qnx_ipc.h
$(MODEL)/get_name_entry.o: get_name_entry.c
$(MODEL)/put_name_entry.o: put_name_entry.c


