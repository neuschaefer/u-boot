#
# See file CREDITS for list of people who contributed to this
# project.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#

include $(TOPDIR)/config.mk

LIB	= $(obj)lib$(SOC).o

COBJS += chal_memc_ddr3.o chal_memc.o

ifeq ($(CONFIG_BCM11140_FPGA),y)
COBJS	+= clock_capri_fpga.o
else
COBJS	+= clock_capri.o
endif

ifeq ($(CONFIG_BOOT_VC),y)
COBJS	+= chal_ipc.o
COBJS   += cmd_ipcbootvc.o
endif

ifeq ($(CONFIG_CMD_A9FREQ),y)
COBJS += cmd_a9freq.o
endif

ifeq ($(CONFIG_MEMC),y)
COBJS	+= cmd_memc.o
SOBJS += write_qos_entry.o
endif

ifeq ($(CONFIG_CMD_NAND),y)
COBJS	+= cmd_nandcrc.o
endif

ifeq ($(CONFIG_PINMUX),y)
COBJS += pinmux_table.o pinmux.o
COBJS-$(CONFIG_BCM11130_RAY) += pinmux_ray.o
COBJS-$(CONFIG_BCM11130_RAY_JFFS2) += pinmux_ray_nand.o
COBJS-$(CONFIG_BCM11130_ROKU_AUSTIN) += pinmux_roku_austin.o
COBJS-$(CONFIG_BCM11140_RAY) += pinmux_ray.o
COBJS-$(CONFIG_BCM11351_RAY) += pinmux_ray.o
COBJS-$(CONFIG_BCM11351_LI_RAY) += pinmux_ray.o
COBJS-$(CONFIG_BCM11351_TABLET) += pinmux_tablet.o
COBJS-$(CONFIG_BCM11351_LI_TABLET) += pinmux_tablet.o
COBJS-$(CONFIG_BCM11140_TABLET) += pinmux_tablet.o
COBJS-$(CONFIG_BCM11140_LI_TABLET) += pinmux_tablet.o
COBJS-$(CONFIG_BCM28145_CAPRISTONE) += pinmux_capristone.o
COBJS-$(CONFIG_BCM28155_CAPRISTONE) += pinmux_capristone.o
COBJS-$(CONFIG_BCM28145_RAY) += pinmux_ray.o
COBJS-$(CONFIG_BCM28155_RAY) += pinmux_ray.o
endif

SOBJS	+=

SRCS	:= $(START:.o=.S) $(SOBJS:.o=.S) $(COBJS:.o=.c)

OBJS	:= $(addprefix $(obj),$(COBJS) $(COBJS-y) $(SOBJS))
START	:= $(addprefix $(obj),$(START))

all:	$(obj).depend $(LIB)

$(LIB):	$(OBJS)
	$(call cmd_link_o_target, $(OBJS))

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
