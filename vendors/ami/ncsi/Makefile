# Makefile for AMI ractrends board
include $(TOPDIR)/config.mk

O_TARGET= ncsilib.o

OBJS	=

COBJS	= ncsistart.o ncsi.o getmac.o

#COBJS	+= datadisp.o
#CFLAGS += -DNCSI_DEBUG

OBJS	= $(AOBJS) $(COBJS)

CFLAGS += -I.. -I$(TOPDIR)/vendors/ami/ncsi

all:	$(O_TARGET) 

$(O_TARGET): .depend $(OBJS)
	$(LD) -r -o $@ $(OBJS)

distclean clean :
	rm -f $(OBJS) $(O_TARGET)

#########################################################################

.depend:	Makefile $(AOBJS:.o=.S) $(COBJS:.o=.c)
		$(CC) -M $(CFLAGS) $(AOBJS:.o=.S) $(COBJS:.o=.c) > $@

sinclude .depend

#########################################################################
