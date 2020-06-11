# Makefile for AMI ractrends board
include $(TOPDIR)/config.mk

O_TARGET= fwupd.o

OBJS	=

COBJS	= fwupdate.o

OBJS	= $(AOBJS) $(COBJS)

CFLAGS += -I.. -I$(TOPDIR)/vendors/ami/fmh -I$(TOPDIR)/vendors/ami/kcs -I$(TOPDIR)/vendors/ami/IPMI -I$(TOPDIR)/..

CFLAGS += -I$(TOPDIR)/vendors/ami/r2c

CFLAGS += -I.. -I$(TOPDIR)/../linux-2.4.18/include 
CFLAGS += -DCPU_BYTE_ORDER_LITTLE_ENDIAN

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
