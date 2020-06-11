# Makefile for AMI ractrends board
include $(TOPDIR)/config.mk

O_TARGET= r2c.o

OBJS	=

COBJS	= phases.o session.o term.o 
COBJS   += sess_misc.o sess_mem.o sess_flash.o
COBJS   += cmd_r2c.o

OBJS	= $(AOBJS) $(COBJS)

CFLAGS += -I.. -I$(TOPDIR)/vendors/ami/fmh

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
