#ifndef __MAPISLANDSTONE_H
#define __MAPISLANDSTONE_H

#define  CONFIG_VCMEM_ADDR_HEX 40000000

#include "islandboard.h"

#undef  CONFIG_EXTRA_ENV_SETTINGS
#undef  CONFIG_SYS_MALLOC_LEN
#undef  CONFIG_ENV_SIZE
#undef  CONFIG_BOOTFILE
#undef  BOOTLINUX
#undef  CONFIG_BOOTCOMMAND

#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_SYS_MALLOC_LEN           SZ_4M
#define CONFIG_ENV_SIZE                (SZ_64K)
#define CONFIG_BOOTCOMMAND             "run bcm_bootcmd"
#define CONFIG_BOOT_VC
#define CONFIG_ISLAND

#define CONFIG_CMD_GPT
#define CONFIG_CMD_MATH

#define BASEINFO      \
    "loadaddr=80000000\0" \
    "emmcdev=0\0" \
    "sddev=1\0" \
    "vcmem=" CONFIG_VCMEM_STR "\0"

#define START_VC_MMC  "start_vc_mmc=gpt setenv vc4 ; mmc dev ${emmcdev}; mmc read ${vcmem} ${gpt_partition_addr} ${gpt_partition_size}; bootvc ${vcmem}\0"
#define START_KNL_MMC "start_knl_mmc=gpt setenv kernel ; mmc dev ${emmcdev}; mmc read 90000000 ${gpt_partition_addr} ${gpt_partition_size}; bootm 90000000\0"

#define RELOAD       "reload=mw 35004100 80000160; reset\0"

#define SD_UPDATE    "sd_update=mmc dev ${sddev}; mmcinfo; malloc tmp 1000; fatload mmc ${sddev} ${tmp} ${sd_update_prefix}.sd-update; source ${tmp}; free tmp\0"

#define CONSOLE_BOOTARGS "console_bootargs=console=ttyS0,115200n8 androidboot.console=ttyS0\0"

#define MMC_BOOTARGS "mmc_bootargs=root=/dev/mmcblk0p4 rootfstype=ext4 gpt rootwait " "\0"

#define MMC_BOOTCOMMAND        "mmc_bootcmd= run start_vc_mmc ; setenv setbootargs setenv bootargs ${console_bootargs} ${mmc_bootargs} ${extraargs}; run setbootargs ;run start_knl_mmc\0"


#define CONFIG_EXTRA_ENV_SETTINGS \
 "watchdog=off\0" \
 SD_UPDATE MMC_BOOTCOMMAND CONSOLE_BOOTARGS MMC_BOOTARGS \
  BASEINFO RELOAD START_VC_MMC START_KNL_MMC

#endif

