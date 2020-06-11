# Makefile for AMI ractrends board
include $(TOPDIR)/config.mk

LIB = ../libvendor.a

SUBDIRS := IPMI fwupdate r2c fmh ncsi
OBJS = IPMI/ipmi.o fwupdate/fwupd.o r2c/r2c.o fmh/fmh.o ncsi/ncsilib.o


all : $(LIB)
	
$(LIB):	$(OBJS)
	$(AR) crv $@ $(OBJS)

$(OBJS) : subdirs

subdirs:
	@for dir in $(SUBDIRS) ; do $(MAKE) -C $$dir || exit 1 ; done

.depend:
	@for dir in $(SUBDIRS) ; do $(MAKE) -C $$dir .depend ; done

distclean:
	@for dir in $(SUBDIRS) ; do $(MAKE) -C $$dir distclean ; done
	rm -f $(LIB)

clean:
	@for dir in $(SUBDIRS) ; do $(MAKE) -C $$dir clean ; done
	rm -f $(LIB)

