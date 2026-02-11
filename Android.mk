ifeq ($(strip $(BUILD_UBOOT)),true)

LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
PRIVATE_UBOOT_DIR := $(LOCAL_PATH)

define run-in-its-directory
(cd $(dir $(1)); $(notdir $(1)) $(2) $(3))
endef

PRIVATE_BINARIES_WITH_EXTENSION := u-boot.bin
PRIVATE_BINARIES_WITH_PATH  := $(addprefix $(PRIVATE_UBOOT_DIR)/, $(PRIVATE_BINARIES_WITH_EXTENSION))
$(call add-prebuilt-files, OPTIONAL_EXECUTABLES, $(PRIVATE_BINARIES_WITH_EXTENSION))

$(PRIVATE_BINARIES_WITH_PATH): build_uboot

build_uboot:
	@echo "Building U-Boot from $(PRIVATE_UBOOT_DIR)"
	@$(call run-in-its-directory, $(PRIVATE_UBOOT_DIR)/make bcm2835_config)
	@$(call run-in-its-directory, $(PRIVATE_UBOOT_DIR)/make all)
	@$(call run-in-its-directory, $(PRIVATE_UBOOT_DIR)/cp $(PRIVATE_BINARIES_WITH_EXTENSION) arm_boot2.bin)

endif
