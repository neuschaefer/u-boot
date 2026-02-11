CONFIG_SYS_TEXT_BASE = 0x4280000

# Common directory includes
COMMON_DIR := $(TOPDIR)/board/$(VENDOR)/common
PLATFORM_CPPFLAGS += -I$(COMMON_DIR)

# VCEB Includes
VC_DIR := $(TOPDIR)/board/$(VENDOR)/common/vceb

PLATFORM_CPPFLAGS += -I$(VC_DIR)/common \
                     -I$(VC_DIR)/host/uboot
