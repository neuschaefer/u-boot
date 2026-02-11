#ifndef __BCM11140_CORE_H
#define __BCM11140_CORE_H

#include <asm/sizes.h>

/* Architecture, CPU, etc */
#define CONFIG_ARMV7
#define CONFIG_CAPRI
#define CONFIG_KONA
#define CONFIG_KONA_GPIO
#define CONFIG_TIMER

#define CONFIG_LOADED_BY_CBOOT

/* SD/MMC Device number */
#define BCM11140_SYS_SD_DEV 1

#define CONFIG_CMD_CLOCK

#define CONFIG_ZERO_BOOTDELAY_CHECK	/* check for keypress on bootdelay==0 */

/* For custom actions e.g. debug LED control just before launching kernel */
#define CONFIG_BRCM_KERNEL_ENTRY_HOOK

#define CONFIG_PINMUX_DUMP /* Dump pinmux info symbolically - useful for reverse eng of dts file source */

#define CONFIG_BRCM_DTBLOB_TAG

/* FIXME - The CONFIG_OF_LIBFDT only works with initrd/ramdisk launching */
/* For the logic to work below we need to bring in the basic utilities without
 * changing the way the kernel is launched.
 */
#undef CONFIG_OF_LIBFDT
#define CONFIG_OF_LIBFDT_CMD

/*
Fix me

When DEBUG is enabled, need to disable both CACHE to make u-boot running
#define CONFIG_SYS_NO_ICACHE
#define CONFIG_SYS_NO_DCACHE
#define DEBUG

*/

#define CONFIG_L2_OFF			/* Disable L2 cache */
#define CONFIG_MACH_TYPE		0xa8d

#define CONFIG_MISC_INIT_R		/* Call board's misc_init_r function */
#define CONFIG_SYS_HZ			1000 

/* Memory Info */
#define KERNEL_SIZE_MAX			(8 * 1024 * 1024)       /* worst case size of an auImage (ie a compressed kernel) */
#define CONFIG_SYS_MALLOC_LEN 		(SZ_1M + KERNEL_SIZE_MAX)	/* see armv7/start.S. */
#define CONFIG_STACKSIZE		SZ_256K

#define CONFIG_PHYS_SDRAM_RSVD_SIZE	0x00000000	/* unused now */

#define CONFIG_SSRAM_BASE		MM_ADDR_IO_SRAM		
#define CONFIG_VC4BOOT_SRAM_OFFSET	0xc000

#define CONFIG_ENABLE_MMU
//#define CONFIG_SYS_ARM_CACHE_WRITETHROUGH

/* Convert a constant to string */
#define  XSTR(x)  #x
#define  STR(x)   XSTR(x)

#if defined(CONFIG_BCM11130_RAY) || defined(CONFIG_BCM11130_RAY_JFFS2) || defined(CONFIG_BCM11130_ROKU_AUSTIN) || defined(CONFIG_BCM28145_RAY)

#define CONFIG_LI_MODE

/* Little-capri 512MB VC4/ARM=128MB/384MB split */
/* Make SDRAM size smaller than default because we use 128 MB for videocore */
#define CONFIG_VCMEM_ADDR_HEX	80000000	/* Starting address of shared System EMI in arm space */
#define CONFIG_VCMEM_SIZE		0x08000000	/* Size of shared System EMI for vc4 usage */
#define CONFIG_PHYS_SDRAM_1		(0x80000000 + CONFIG_VCMEM_SIZE)	/* ARM starts here */
#ifdef CONFIG_MXC_LIGHT 
#define CONFIG_PHYS_SDRAM_1_SIZE        (0x10000000 - CONFIG_VCMEM_SIZE)        /* ARM memory reduced accordingly total 256MB */
#else
#define CONFIG_PHYS_SDRAM_1_SIZE	(0x20000000 - CONFIG_VCMEM_SIZE)	/* ARM memory reduced accordingly */
#endif
#define CONFIG_KNLBASE_RAY		"88000000"	/* ARM starts here */
#define CONFIG_KNLBASE_STR 		CONFIG_KNLBASE_RAY
#define CONFIG_RESCUEBASE_STR 		"8C000000"

#define QOS_SETUP \
"qos_setup=memc bwc * 3F4;"\
"memc spr * 21140;"\
"memc apal SYS 05248;"\
"memc qos SYS 0 all lat:300;"\
"memc qos SYS 1 none lat:300;"\
"memc qos SYS 2 none lat:300;"\
"memc qos SYS 3 none lat:300;"\
"memc qos SYS 4 none lat:300;"\
"memc qos SYS 5 none lat:300;"\
"memc qos SYS 6 none lat:300;"\
"memc qos SYS 7 none lat:300;"\
"memc qos SYS 8 none lat:300;"\
"memc qos SYS 9 none lat:300;"\
"memc qos SYS 10 none lat:300;"\
"memc qos SYS 11 port:1 lat:200;"\
"memc qos SYS 12 port:2 lat:250;"\
"memc qos SYS 13 port:3 lat:70;"\
"memc qos SYS 14 pri:22 lat:150;"\
"memc qos SYS 15 pri:11 lat:0,pri;"\
"memc crc SYS\0"

#elif (defined(CONFIG_BCM11351_LI_RAY) || defined(CONFIG_BCM11351_LI_TABLET) || defined(CONFIG_BCM11140_LI_TABLET))

#define CONFIG_LI_MODE

/* Little-capri 1GB VC4/ARM=512MB/512MB split */
/* Make SDRAM size smaller than default because we use 256 MB for videocore */
#define CONFIG_VCMEM_ADDR_HEX	80000000	/* Starting address of shared System EMI in arm space */
#ifdef CONFIG_MXC_LIGHT 
#define CONFIG_VCMEM_SIZE		0x08000000	/* Size of shared System EMI for vc4 usage 128MB */
#else
#define CONFIG_VCMEM_SIZE               0x20000000      /* Size of shared System EMI for vc4 usage */
#endif
#define CONFIG_PHYS_SDRAM_1		(0x80000000 + CONFIG_VCMEM_SIZE)	/* ARM starts here */
#ifdef CONFIG_MXC_LIGHT 
#define CONFIG_PHYS_SDRAM_1_SIZE	(0x10000000 - CONFIG_VCMEM_SIZE)	/* ARM memory reduced accordingly total 256MB */
#else
#define CONFIG_PHYS_SDRAM_1_SIZE        (0x40000000 - CONFIG_VCMEM_SIZE)        /* ARM memory reduced accordingly */
#endif
#define CONFIG_KNLBASE_LI		"A0000000"	/* ARM starts here */
#define CONFIG_KNLBASE_STR		CONFIG_KNLBASE_LI

#define QOS_SETUP \
"qos_setup=memc bwc * 3F4;"\
"memc spr * 21140;"\
"memc apal SYS 05248;"\
"memc qos SYS 0 all lat:300;"\
"memc qos SYS 1 none lat:300;"\
"memc qos SYS 2 none lat:300;"\
"memc qos SYS 3 none lat:300;"\
"memc qos SYS 4 none lat:300;"\
"memc qos SYS 5 none lat:300;"\
"memc qos SYS 6 none lat:300;"\
"memc qos SYS 7 none lat:300;"\
"memc qos SYS 8 none lat:300;"\
"memc qos SYS 9 none lat:300;"\
"memc qos SYS 10 none lat:300;"\
"memc qos SYS 11 port:1 lat:200;"\
"memc qos SYS 12 port:2 lat:250;"\
"memc qos SYS 13 port:3 lat:70;"\
"memc qos SYS 14 pri:22 lat:150;"\
"memc qos SYS 15 pri:11 lat:0,pri;"\
"memc crc SYS\0"

#else

#undef CONFIG_LI_MODE  

/* Big-capri */
/* Other capri boards dedicate 1GB to VC4, 1GB to ARM */
#define CONFIG_VCMEM_ADDR_HEX 40000000	/* Starting address of MM EMI in arm space */
#define CONFIG_PHYS_SDRAM_1		0x80000000	/* ARM starts here */
#define CONFIG_PHYS_SDRAM_1_SIZE	0x40000000 	/* 1 GB System EMI */
#define CONFIG_KNLBASE_BI		"80000000"	/* Big-capri mode - ARM base of system EMI */
#define CONFIG_KNLBASE_STR 		CONFIG_KNLBASE_BI

#define QOS_SETUP \
"qos_setup=memc bwc * 3f4;"\
"memc spr * 20100;"\
"memc apal VC4 6145a;"\
"memc apal SYS 06661;"\
"memc qos * 0 all lat:300;"\
"memc qos * 1 none lat:300;"\
"memc qos * 2 none lat:300;"\
"memc qos * 3 none lat:300;"\
"memc qos * 4 none lat:300;"\
"memc qos * 5 none lat:300;"\
"memc qos * 6 none lat:300;"\
"memc qos * 7 none lat:300;"\
"memc qos * 8 none lat:300;"\
"memc qos * 9 none lat:300;"\
"memc qos * 10 none lat:300;"\
"memc qos * 11 port:1 lat:200;"\
"memc qos * 12 port:2 lat:250;"\
"memc qos * 13 port:3 lat:70;"\
"memc qos * 14 pri:22 lat:150;"\
"memc qos * 15 pri:11 lat:0,pri;"\
"memc crc *\0"

#endif

#define  VC_XHEX(x)  0x ## x
#define  VC_HEX(x)   VC_XHEX(x)

#define  CONFIG_VCMEM_STR  STR(CONFIG_VCMEM_ADDR_HEX)
#define  CONFIG_VCMEM_ADDR VC_HEX(CONFIG_VCMEM_ADDR_HEX)

/* Where kernel is loaded to in memory */
#define CONFIG_SYS_LOAD_ADDR	0x84000000
#define LINUX_BOOT_PARAM_ADDR	(CONFIG_PHYS_SDRAM_1+CONFIG_PHYS_SDRAM_RSVD_SIZE)  /* ATAGS location */

#define CONFIG_SYS_MEMTEST_START	CONFIG_PHYS_SDRAM_1
#define CONFIG_SYS_MEMTEST_END		(CONFIG_PHYS_SDRAM_1+CONFIG_PHYS_SDRAM_1_SIZE)
#define CONFIG_NR_DRAM_BANKS				1

#define CONFIG_SYS_SDRAM_BASE		(CONFIG_PHYS_SDRAM_1 + CONFIG_PHYS_SDRAM_RSVD_SIZE)
/* This is the intial SP used only breifly for relocating the uboot image to top of SDRAM. */
/* After relocation uboot moves the stack to the right place. */
#define CONFIG_SYS_INIT_SP_ADDR		(CONFIG_SYS_SDRAM_BASE + CONFIG_STACKSIZE)

/* A9 MHz */
#define CONFIG_MHZ_A9_DEFAULT	1100 /* Best running frequency for all */

/* ARM A9 core voltages  - Capri RAY Only */
#define CONFIG_VDD_A9_DEFAULT	130 /* 1.23V */

/* ARM A9 core voltages  - Capri RAY Only */
#define CONFIG_VDD_VC4_DEFAULT	123 /* 1.23V */

/* Serial Info */
#define CONFIG_SYS_NS16550
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	(-4)	/* Post pad 3 bytes after each reg addr */
#define CONFIG_SYS_NS16550_CLK		13000000
#define CONFIG_CONS_INDEX		1
#define CONFIG_SYS_NS16550_COM1		0x3e000000

#define CONFIG_BAUDRATE			115200
#define CONFIG_SYS_BAUDRATE_TABLE	{9600, 19200, 38400, 57600, 115200, 230400, 460800, 921600} 

#define CONFIG_ENV_OVERWRITE	/* Allow serial# and ethernet mac address to be overwritten in nv storage */
#define CONFIG_ENV_SIZE			0x10000 /* 0xA0000-0xAFFFF */

/* NO flash */
#define CONFIG_SYS_NO_FLASH		/* Not using NAND/NOR unmanaged flash */

/* Keypad */
#define CONFIG_KONA_KEYPAD

/* MMC configuration. */
#define CONFIG_MMC
#define CONFIG_GENERIC_MMC
#define CONFIG_KONA_MMC
#define CONFIG_SYS_MMC_ENV_DEV	0 	/* Device 0 is not a SDIO slot, just the first enumeration of a device */

#define CONFIG_EMMC_8BIT		/* Only configure eMMC as 8-bit. SD remain at 4-bit maximum */

#define CONFIG_EFI_PARTITION		/* One partition type must be defined for part.c */
#define CONFIG_DOS_PARTITION

/* Ethernet configuration on by default, disable in non-ethernet board include files */
#define CONFIG_NET_MULTI
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_PING
#define CONFIG_CMD_DNS

/* General U-Boot configuration */
#define CONFIG_SYS_CBSIZE		1024	/* Console buffer size */
#define CONFIG_SYS_PBSIZE		(CONFIG_SYS_CBSIZE +	sizeof(CONFIG_SYS_PROMPT) + 16) /* Printbuffer size */
#define CONFIG_SYS_MAXARGS		64
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE

#define CONFIG_VERSION_VARIABLE	/* Enabled UBOOT build date/time id string */
#define CONFIG_AUTO_COMPLETE
#define CONFIG_SYS_HUSH_PARSER
#define CONFIG_SYS_PROMPT_HUSH_PS2	"> "
#define CONFIG_CMDLINE_EDITING
#define CONFIG_SYS_LONGHELP

#define CONFIG_CRC32_VERIFY		/* Add crc32 to memory verify commands */
#define CONFIG_MX_CYCLIC		/* Memory display cyclic */

#define CONFIG_CMDLINE_TAG		/* ATAG_CMDLINE setup */
#define CONFIG_SETUP_MEMORY_TAGS	/* ATAG_MEM setup */

/* The bootdelay is set to 3 to give the ethernet driver time to initialize and stabilize. This makes nfs more reliable */
#define CONFIG_BOOTDELAY		3	/* User can hit a key to abort kernel boot and stay in uboot cmdline */
#define CFG_BOOTN_BOOT_SIZE           ( 2 * 1024 * 1024)
#define CFG_BOOTN_UBOOT_SIZE          ( 512 * 1024)
#define CFG_BOOTN_ID_SIZE             ( 2 * nand_info[0].erasesize )
#define CFG_BOOTN_FS_SIZE             (48 * 1024 * 1024)
#define CFG_BOOTN_MAC_OFFSET          ((CFG_BOOTN_BOOT_SIZE - CFG_BOOTN_ID_SIZE) + 351)

#define CONFIG_BOOTCOMMAND 		"run bcm_bootcmd"
#define CONFIG_SYS_PROMPT		"u-boot> "  

#include <config_cmd_default.h>
#define CONFIG_CMD_ASKENV		/* askenv command to change env variables */
#define CONFIG_CMD_SAVES		/* Save serial data */
#define CONFIG_CMD_MMC			/* mmc subsystem commands */
#define CONFIG_CMD_EXT2
#define CONFIG_CMD_FAT
#define CONFIG_CMD_GPIO

#define CONFIG_ENV_IS_IN_MMC 		/* Save env in eMMC */
#define CONFIG_CMD_SAVEENV		/* saveenv command enabled */ 

// kernel reboots currently with this set
//#define CONFIG_HW_WATCHDOG

#define BOARD_LATE_INIT

/* Use this for all Capri parts, for now. */
#define CONFIG_USE_SECWD2_FOR_RESET

/* Fastboot writes to eMMC */
#define CONFIG_FASTBOOT
#define CONFIG_FASTBOOT_NO_PMU	/* No PMU so VBUS detection is different */
#define CONFIG_FASTBOOT_BOARDNAME "Capri Board"
#define CFG_FASTBOOT_TRANSFER_BUFFER CONFIG_PHYS_SDRAM_1
#define CFG_FASTBOOT_TRANSFER_BUFFER_SIZE (CONFIG_PHYS_SDRAM_1_SIZE - CONFIG_PHYS_SDRAM_RSVD_SIZE - SZ_1M)

#define CONFIG_STORAGE_EMMC	1
#define CFG_FASTBOOT_MMC_NO	0

/* Setup environment variables based on GPT table */
#define CONFIG_CMD_GPT

/* Enable environment variable math support */
#define CONFIG_CMD_MATH

//#define CONFIG_CMD_SPM        /* (Takes 500 msec extra to boot) */
#define CONFIG_PRINT_DDR_INFO /* (Takes 100 msec extra to boot) */
#define CONFIG_CMD_STRCMP

/* Read GPT from flash and register these partitions */
#define CONFIG_FASTBOOT_GPT_TABLE_IN_FLASH

/* I2C configuration */
#define CONFIG_CMD_I2C			/* enable cmd_i2c.c */
#define CONFIG_HARD_I2C	1		/* calls i2c_init in board.c */
#define CONFIG_SYS_I2C_SPEED 50000	/* used by cmd_i2c.c */
#define CONFIG_SYS_I2C_SLAVE 0x08	/* used by cmd_i2c.c */
#define CONFIG_SYS_I2C_INITIAL_BUS 2	/* pmu bus */
#define CONFIG_DRIVER_KONA_I2C 1	/* enable kona i2c driver */

/* Default GPIOs for the LCD panel - FIXME This is board specific */
#define CONFIG_VC_LCD_POWER_ENABLE	142
#define CONFIG_VC_LCD_RESET		146
#define CONFIG_VC_LCD_BL_POWER_ENABLE	141
#define CONFIG_VC_LCD_BL_ENABLE		69
#define CONFIG_VC_LCD_BL_PWM		145
/* Default GPIOs for the CAMERAS - FIXME This is board specific */
#define CONFIG_VC_LCD_

/* Backup uboot uses same environment area as primary uboot */
#define CONFIG_ENV_OFFSET	0x02340000	/* Must match gpt table layout */

/* 
 * Due to ATAG usage, dt-blob is limited to less than 0x4000 or it will trample
 * the mmu table area. Use 0x3800 to leave room for other atags.
 */
#define GET_DTBLOB \
"get_dtblob=gpt setenv dt-blob;"\
"setenv maxdtsize 3800;"\
"mmc dev ${emmcdev}; "\
"malloc brcm_dt_loadaddr ${maxdtsize};"\
"math div maxdtblks ${maxdtsize} 200;"\
"mmc read ${brcm_dt_loadaddr} ${gpt_partition_addr} ${maxdtblks};"\
"fdt addr ${brcm_dt_loadaddr} ${maxdtsize}\0"

/* Extract the root filesystem partiton number to avoid having to hardcode /dev/mmcblk0pN */
/* Note that Android uses "debug" for its root, otherwise we use "root". */
#define MMC_SET_ROOTDEV \
"mmc_set_rootdev="\
"if gpt setenv root;"\
"then;"\
"else;"\
	"gpt setenv debug;"\
"fi;"\
"setenv rootdev /dev/mmcblk0p${gpt_partition_entry}\0"

/* Reboot into UART mode */
#define RELOAD \
"reload=mw 35004100 80000160;"\
"reset\0"

/* Common bootargs macros */
#define CONSOLE_BOOTARGS "console_bootargs=console=ttyS0,115200n8 androidboot.console=ttyS0\0"
#define IP_CFG_BOOTARGS	"ip=${ipaddr}::${gatewayip}:${netmask}::${ethif}:off bcmmac=${ethaddr}"

/* Common mmc bootargs, bootcommand and startup macros */
#define MMC_BOOTARGS 	"mmc_bootargs=root=${rootdev} rootfstype=ext4 gpt rootwait " IP_CFG_BOOTARGS "\0"

#define MMC_START_KNL \
"mmc_start_knl=gpt setenv kernel;"\
"math add tmp loadaddr ${knloffs}; "\
"mmc dev ${emmcdev}; "\
"math div maxknlblocks maxknlsize 200;"\
"mmc read ${tmp} ${gpt_partition_addr} ${maxknlblocks};"\
"bootm ${tmp}\0"

/* Display splash screen by loading videocore firmware and launching it */
#define MMC_LOAD_VC \
"mmc_load_vc=gpt setenv vc4;"\
"mmc dev ${emmcdev};"\
"math div maxvcblocks maxvcsize 200;"\
"mmc read ${vcmem} ${gpt_partition_addr} ${maxvcblocks};\0"

#define MMC_START_VC \
"mmc_start_vc=run mmc_load_vc;"\
"vc bootfs add_dtblob;"\
"vc run ${vcmem}\0"

/* Update emmc partitions from SD card */
#define SD_UPDATE \
"sd_update=mmc dev ${sddev};"\
"mmcinfo;"\
"malloc tmp 2000;"\
"fatload mmc ${sddev} ${tmp} ${sd_update_prefix}.sd-update;"\
"source ${tmp};"\
"free tmp\0"

/* MMC bootcommands will start the kernel image or flash the mmc when the VOL_DOWN pressed */
#define MMC_BOOTCMD \
"mmc_bootcmd="\
"if key VOL_DOWN;"\
"then;"\
	"run sd_update;"\
	"reset;"\
"fi;"\
"mmc rescan; mmc dev 1; mmcinfo;"\
"malloc temp 1000;"\
"if fatload mmc 1 ${temp} ${sd_update_prefix}.autoboot;"\
"then;"\
	"source ${temp};"\
"fi;"\
"free temp;"\
"run qos_setup;"\
"run get_dtblob;"\
"run mmc_start_vc;"\
"run mmc_set_rootdev;"\
"setenv setbootargs setenv bootargs ${console_bootargs} ${mmc_bootargs} ${extraargs};"\
"run setbootargs;"\
"run mmc_start_knl\0"

#define EXTRAARGS ""

/* Add some defaults to help out form-factor boards without UART consoles. */
#define SD_UPDATE_PREFIX "sd_update_prefix=bin\0"
#define BCM_BOOTCMD "bcm_bootcmd=run mmc_bootcmd\0"

/* 
 * The knloffs is the offset to the start of the uImage header from 
 * the base of kernel DDR.  For example, if the kernel DDR starts at
 * 80000000, the compressed image is put at 80008000, but since the
 * actual kernel code starts there, we adjust the offset of the header
 * back by 40 hex to get 80007fc0 so that the uncompressing can XIP.
 * This removes an extra kernel copy during the startup. The performance 
 * gain is about 435 msec in time for a 3MB kernel.
 */
#define BASEINFO \
"maxknlsize=400000\0"\
"maxvcsize=400000\0"\
"maxinitrdsize=200000\0"\
"loadaddr=" CONFIG_KNLBASE_STR "\0" \
"initrd_addr=" CONFIG_RESCUEBASE_STR "\0" \
"knloffs=7fc0\0" \
"emmcdev=0\0" \
"sddev=1\0" \
"vcmem=" CONFIG_VCMEM_STR "\0" \
"raid=noautodetect\0" \
"boot_mode=broadcom\0"

#if 0
/* This logic is mainly for NAND, but is shown here for consistency
 * in case we ever want to update the dt-blob with the partition table
 * information. Since the gpt table is already created, we can't actually
 * call this automatically anyways. */
#define PT_TO_DTBLOB \
"pt_to_dtblob="\
"if gpt setenv dt-blob;"\
"then;"\
	"math mul maxdtbytes ${gpt_partition_size} 200;"\
	"run get_dtblob;"\
	"fdt addr ${brcm_dt_loadaddr} ${maxdtbytes};"\
	"gpt enumerate;"\
	"fdt rm /partition_table;"\
	"fdt mknod / partition_table;"\
	"for part in ${gpt_partition_list};"\
	"do;"\
		"gpt setenv ${part};"\
		"fdt set /partition_table ${gpt_partition_name} ${gpt_partition_addr} ${gpt_partition_size};"\
	"done;"\
	"mmc dev ${emmcdev};"\
	"gpt setenv dt-blob;"\
	"mmc write ${brcm_dt_loadaddr} ${gpt_partition_addr} ${gpt_partition_size};"\
"else;"\
	"echo dt-blob partition does not exist, cannot put partition table information into it;"\
"fi\0"
#endif


#define EXTRA_CORE_SETTINGS \
BASEINFO \
MMC_SET_ROOTDEV \
RELOAD \
EXTRAARGS \
CONSOLE_BOOTARGS \
MMC_BOOTARGS \
MMC_BOOTCMD \
MMC_START_KNL \
MMC_LOAD_VC \
MMC_START_VC \
GET_DTBLOB \
SD_UPDATE \
SD_UPDATE_PREFIX \
BCM_BOOTCMD \
QOS_SETUP \

#ifdef CONFIG_BCM11140_ETH

/* 
 * Get the dt-blob from NFS rather than flash.
 */
#define NFS_GET_DTBLOB \
"nfs_get_dtblob=setenv maxdtsize 3800;"\
"malloc brcm_dt_loadaddr ${maxdtsize};"\
"nfs ${brcm_dt_loadaddr} ${nfs_root}/flash/bin.dt-blob;"\
"fdt addr ${brcm_dt_loadaddr} ${maxdtsize}\0"

/* Network update macros */
#define NFS_LOAD_VC \
"nfs_load_vc=nfs ${vcmem} ${nfs_root}/flash/bin.vc4;\0"

#define NFS_START_VC \
"nfs_start_vc=run nfs_load_vc;"\
"vc bootfs add_dtblob;"\
"vc run ${vcmem}\0"

#define NFS_START_KNL \
"nfs_start_knl=math add tmp loadaddr ${knloffs}; "\
"nfs ${tmp} ${nfs_root}/flash/bin.kernel;"\
"bootm ${tmp}\0"

#define NFS_UPDATE \
"nfs_update="\
"malloc tmp 2000;"\
"nfs ${tmp} ${nfs_update_prefix}.nfs-update;"\
"source ${tmp};"\
"free tmp\0"

#define NFS_BOOTARGS "nfs_bootargs=root=/dev/nfs rw nfsroot=${serverip}:${nfs_root},tcp rootwait " IP_CFG_BOOTARGS "\0"

#define NFS_BOOTCMD \
"nfs_bootcmd="\
"sleep 2;"\
"run qos_setup;"\
"run nfs_get_dtblob;"\
"run nfs_start_vc;"\
"setenv setbootargs setenv bootargs ${console_bootargs} ${nfs_bootargs} ${extraargs};"\
"run setbootargs;"\
"run nfs_start_knl\0"

#define TFTP_UPDATE \
"tftp_update="\
"malloc tmp 2000;"\
"tftp ${tmp} ${tftp_update_prefix}.tftp-update;"\
"source ${tmp};"\
"free tmp\0"

#define EXTRA_NET_SETTINGS \
NFS_LOAD_VC \
NFS_START_VC \
NFS_START_KNL \
NFS_UPDATE \
NFS_BOOTARGS \
NFS_BOOTCMD \
NFS_GET_DTBLOB \
TFTP_UPDATE \


#define CONFIG_EXTRA_ENV_SETTINGS EXTRA_CORE_SETTINGS EXTRA_NET_SETTINGS

#else

#define CONFIG_EXTRA_ENV_SETTINGS EXTRA_CORE_SETTINGS

#endif

/* Android specific board setup override. It will alter mmc boot command
*/
#if defined(ANDROID)
	#include "bcmboard_android.h"
#endif


#endif /* __BCM11140_CORE_H */
