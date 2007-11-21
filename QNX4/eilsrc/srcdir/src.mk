# location variables
LIBDIR = /usr/local/lib
INCDIR = /usr/local/include
USEDIR = /usr/local/include/use
BINDIR = .

# flag variables
MODEL = s
# MODEL = 3r
# MODEL = l
# MODELS = s l c
# CFLAGS = -I $(INCDIR) -3 -ms -g
#CFLAGS = -I $(INCDIR) -3
#CFLAGS = -I $(INCDIR) -3 -mf
CFLAGS = -m$(MODEL) -I $(INCDIR) -2
# CFLAGS = -g -I $(INCDIR)
# LDFLAGS = -T0 -L $(LIBDIR) -b
LDFLAGS = -L $(LIBDIR) -b

SRCS = $(BASE).c
OBJS = $(SRCS:.c=.o)
HDRS = $(BASE).h
TXTS = $(BASE).txt
USES = $(BASE).use
DOCS =

# interface to norts maintenance targets in maint.mk2
SOURCE = $(SRCS) $(HDRS) $(TXTS) $(DOCS) makefile Makefile
OBJECT = $(OBJS) $(USES)
TARGET = $(BINDIR)/$(BASE)
MNC = $(BASE)

# source location variables
DASDIR = $(SRCDIR)/../das

# rules
#.c:
#	$(LINK.c) -o $@ $(OBJS) $(LDFLAGS)
