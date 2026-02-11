#
# (C) Copyright 2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# See file CREDITS for list of people who contributed to this
# project.
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of
# the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston,
# MA 02111-1307 USA
#
include $(TOPDIR)/config.mk

LIB	= $(obj)lib$(VENDOR).o

VC_DIR := $(TOPDIR)/board/broadcom/common/vceb
CFLAGS_board/broadcom/common +=	-I$(VC_DIR)/common		\
                        -I$(VC_DIR)/host/uboot		\
                        -I$(VC_DIR)/host		\
                        -I$(VC_DIR)/host/common		\
                        -DVCEB_AP_MODE \
                        -Wno-declaration-after-statement

#			-I$(VC_DIR)/videocore \

COBJS-y	+= 
#VCOBJS-$(CONFIG_MTD_NAND_BCM_UMI) += nand_correct_data512.o 
#VCOBJS-y += cmd_dlmode.o
#VCOBJS-y += cmd_defaultenv.o
#VCOBJS-y += cmd_fixparts.o

VCOBJS-$(CONFIG_VCEB) += rle_decoder.o
VCOBJS-$(CONFIG_VCEB) += vceb/host/uboot/vceb_uboot_os.o
VCOBJS-$(CONFIG_VCEB) += vceb/host/uboot/vceb_app_code.o
VCOBJS-$(CONFIG_VCEB) += vceb/host/uboot/vceb_2nd_stage_code.o
VCOBJS-$(CONFIG_VCEB) += vceb/host/vceb.o
VCOBJS-$(CONFIG_VCEB) += vceb/common/vceb_common.o
VCOBJS-$(CONFIG_VCEB) += vceb/host/common/vceb_interface.o

VCOBJS-$(CONFIG_VCEB) += vceb/host/common/vceb_host_interface.o
VCOBJS-$(CONFIG_VCEB) += vceb/host/uboot/2835/vceb_platform.o
VCOBJS-$(CONFIG_VCEB) += vceb/host/uboot/2835/images.o


#VCOBJS-$(CONFIG_VCEB) += vceb/host/uboot/2ndstage_host_b0.bin.o
#VCOBJS-$(CONFIG_VCEB) += vceb/host/uboot/vesper_ep4/vceb.bin.o
VCOBJS-$(CONFIG_VCEB) += boot.rle.o

SRCS	:= $(SOBJS:.o=.S) $(COBJS-y:.o=.c)
OBJS	:= $(addprefix $(obj),$(COBJS-y))
SOBJS	:= $(addprefix $(obj),$(SOBJS))
OBJS	+= $(VCOBJS-y)

$(LIB):	$(obj).depend $(OBJS)
	$(call cmd_link_o_target, $(OBJS))

clean:
	rm -f $(SOBJS) $(OBJS)

distclean:	clean
	rm -f $(LIB) core *.bak .depend

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
