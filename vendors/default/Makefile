# Makefile for default vendor (generic u-boot)
include $(TOPDIR)/config.mk

LIB	= ../libvendor.a

AOBJS	=

COBJS	= novendor.o 

OBJS	= $(AOBJS) $(COBJS)

CPPFLAGS += -I..

all:	$(LIB) $(OBJS) 

$(LIB):	.depend $(OBJS)
	$(AR) crv $@ $(OBJS)

distclean clean : 
	rm -f $(LIB) $(OBJS)

#########################################################################

.depend:	Makefile $(AOBJS:.o=.S) $(COBJS:.o=.c)
		$(CC) -M $(CFLAGS) $(AOBJS:.o=.S) $(COBJS:.o=.c) > $@

sinclude .depend

#########################################################################
