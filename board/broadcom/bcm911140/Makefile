include $(TOPDIR)/config.mk

LIB = $(obj)lib$(BOARD).o

COBJS	= $(BOARD).o
SOBJS	= boot_core1.o

SRCS	:= $(SOBJS:.o=.S) $(COBJS:.o=.c)
COBJS	:= $(addprefix $(obj),$(COBJS))
SOBJS	:= $(addprefix $(obj),$(SOBJS))

$(LIB):	$(COBJS) $(SOBJS)
	$(call cmd_link_o_target, $(COBJS) $(SOBJS))

clean:
	rm -f $(SOBJS) $(COBJS)

distclean:	clean
	rm -f $(LIB) core *.bak $(obj).depend

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk
#include /home/lab/uboot_from_denx_tot/uboot/rules.mk

sinclude $(obj).depend

#########################################################################
