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

LIB	= $(obj)libkona-common.o

COBJS-$(CONFIG_FASTBOOT) += fastboot.o
COBJS-$(CONFIG_FASTBOOT_BUILD_GPT) += gpt_mmc.o
COBJS-$(CONFIG_FASTBOOT_GPT_TABLE_IN_FLASH) += gpt_mmc_table_in_flash.o
COBJS-$(CONFIG_BOOST_ARM_CLK) += cmd_boost_arm_clk.o
COBJS-$(CONFIG_CMD_CLOCK) += clock.o cmd_clock.o
COBJS-$(CONFIG_SPLASH_SCREEN) += mdec.o cmd_vcsplash.o
COBJS-$(CONFIG_BOOT_VC) += ipc.o cmd_bootvc.o vc_debug_sym.o cmd_vc_bootfs.o cmd_vc_display.o
COBJS-$(CONFIG_BCM59055_BATTERY) += cmd_battery_mon.o
COBJS-$(CONFIG_TIMER) += timer.o
COBJS-$(CONFIG_KONA_KEYPAD) += chal_keypad.o
COBJS-$(CONFIG_CMD_ANDROID) += cmd_android.o

COBJS	:= $(COBJS-y)
COBJS	+= init.o wd.o
SOBJS	+= lowlevel_init.o reset.o cache.o secure.o

SRCS	:= $(START:.o=.S) $(SOBJS:.o=.S) $(COBJS:.o=.c)

OBJS	:= $(addprefix $(obj),$(COBJS) $(SOBJS))
START	:= $(addprefix $(obj),$(START))

all:	$(obj).depend $(LIB)

$(LIB):	$(OBJS)
	$(call cmd_link_o_target, $(OBJS))

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
