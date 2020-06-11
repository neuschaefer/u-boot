# Makefile for AMI ractrends board
include $(TOPDIR)/config.mk

O_TARGET= ipmi.o

OBJS	=

COBJS	= cmdhandler.o

OBJS	= $(AOBJS) $(COBJS)

CPPFLAGS += -I..

CFLAGS += -I.. -I$(TOPDIR)/../linux-2.4.18/include -I$(TOPDIR)/vendors/ami/IPMI  -I$(TOPDIR)/vendors/ami/fmh -I$(TOPDIR)/include/$(SOC) -I$(TOPDIR)/..
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
