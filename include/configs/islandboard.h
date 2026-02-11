#ifndef __CONFIG_H
#define __CONFIG_H

#include <asm/sizes.h>
#include <asm/arch/hardware.h>
#include <asm/arch/brcm_rdb_sysmap.h>

/* Architecture, CPU, etc */
#define CONFIG_ARMV7
#define CONFIG_KONA
#define CONFIG_KONA_GPIO
#define CONFIG_TIMER
                                               
#define CONFIG_MACH_TYPE                0x09FA
#define CONFIG_ARCH_CPU_INIT

#define CONFIG_SYS_UBOOT_BASE           CONFIG_SYS_TEXT_BASE 

#define CONFIG_MISC_INIT_R

#define CONFIG_SYS_TIMERBASE		TIMER_BASE_ADDR 
#define CONFIG_SYS_HZ                   1000  

/* Memory Info */
#define CONFIG_SYS_MALLOC_LEN           (SZ_256K) 
#define PHYS_SDRAM_1			KONA_RAM_START
#define PHYS_SDRAM_1_SIZE		(SZ_512M)
#define CONFIG_SYS_SDRAM_BASE PHYS_SDRAM_1
#define CONFIG_SYS_SDRAM_SIZE PHYS_SDRAM_1_SIZE
/* Set loadaddr=0x82007fc0 which equals kernel's load address (0x82008000 - uImage hdrsize),
 *  so uboot can XIP the uImage instead of relocating the uImage
 */
#define CONFIG_LOADADDR		0x82007FC0

/* Modem Image */
#define ISLAND_MODEM_IMG_START	(KONA_RAM_START)
#define ISLAND_MODEM_IMG_SIZE	(SZ_32M)

/* VC Image */
#ifdef CONFIG_ISLAND_STONE_LI
#define ISLAND_VC_IMG_START	(KONA_RAM_START + SZ_32M)
#define ISLAND_VC_IMG_SIZE	(SZ_128M)
#else
#define ISLAND_VC_IMG_START	(0)
#define ISLAND_VC_IMG_SIZE	(0)
#endif


/* This value needs to be fixed */
#define CONFIG_SYS_INIT_SP_ADDR (CONFIG_SYS_SDRAM_BASE + CONFIG_SYS_SDRAM_SIZE - \
                            GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_MEMTEST_START	(PHYS_SDRAM_1 + ISLAND_MODEM_IMG_SIZE + ISLAND_VC_IMG_SIZE)
#define CONFIG_SYS_MEMTEST_END		(PHYS_SDRAM_1 + PHYS_SDRAM_1_SIZE)
#define CONFIG_NR_DRAM_BANKS		1         
#define CONFIG_STACKSIZE		(SZ_256K)

/* Serial Info */
#define CONFIG_SYS_NS16550
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	(-4)
#define CONFIG_SYS_NS16550_CLK		13000000
#define CONFIG_CONS_INDEX		1
#define CONFIG_SYS_NS16550_COM1		UARTB_BASE_ADDR

#define CONFIG_ENV_OVERWRITE
#define CONFIG_BAUDRATE			115200
#define CONFIG_SYS_BAUDRATE_TABLE       {9600, 19200, 38400, 57600, 115200, 230400, 460800, 921600} 

/* Flash and environment info */
#define CONFIG_SYS_NO_FLASH
#define CONFIG_ENV_SIZE		         (SZ_4K)

/* MMC configuration. */
#define CONFIG_MMC
#define CONFIG_GENERIC_MMC
#define CONFIG_KONA_MMC
#define CONFIG_DOS_PARTITION
#define CONFIG_EFI_PARTITION

#define MTDIDS_DEFAULT			""
#define MTDPARTS_DEFAULT		""

/* General U-Boot configuration */
#define CONFIG_BOOTFILE			"uImage"
#define CONFIG_SYS_PROMPT		"islandboard> "
#define CONFIG_SYS_CBSIZE		(SZ_1K)
#define CONFIG_SYS_MAXARGS		64
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE
#define CONFIG_VERSION_VARIABLE
#define CONFIG_AUTO_COMPLETE
#define CONFIG_SYS_HUSH_PARSER
#define CONFIG_SYS_PROMPT_HUSH_PS2	"> "
#define CONFIG_CMDLINE_EDITING
#define CONFIG_SYS_LONGHELP
#define CONFIG_CRC32_VERIFY
#define CONFIG_MX_CYCLIC
#define CONFIG_SYS_PBSIZE		(CONFIG_SYS_CBSIZE + sizeof(CONFIG_SYS_PROMPT) + 16)
#define CONFIG_SYS_LOAD_ADDR		(KONA_RAM_START + ISLAND_MODEM_IMG_SIZE + ISLAND_VC_IMG_SIZE + 0x4000000)

#define LINUX_BOOT_PARAM_ADDR		(KONA_RAM_START + ISLAND_MODEM_IMG_SIZE + ISLAND_VC_IMG_SIZE + 0x100)

#define CONFIG_CMDLINE_TAG
#define CONFIG_INITRD_TAG

#ifdef CONFIG_ISLAND_STONE_LI
#define CONFIG_BOOTARGS     "console=ttyS0,115200n8 mem=352M gpt androidboot.console=ttyS0"
#else
#define CONFIG_BOOTARGS     "console=ttyS0,115200n8 mem=416M gpt androidboot.console=ttyS0"
#endif

#define DEFAULT_BOOT_MODE	"boot_mode=android\0"

/* If kernel uImage is store on a raw eMMC device, we have to figure out uImage
 * size by checking its header 
 */ 
#define CONFIG_CMD_SETIMGSIZE

#ifdef CONFIG_ISLAND_SV
#define BOOTLINUX \
	"bootlinux=" \
		"if vc boot; then " \
			"if loadimg kern 0; then " \
				"set_watchdog off; "\
				"gpio clear 114 ; " \
				"if test ${ramdisk_size} -eq 0; then "\
					"echo WARNING: Kernel is now booting without any ramdisk;" \
					"bootm ${loadaddr}; " \
				"else " \
					"bootm ${loadaddr} ${ramdisk_loadaddr} ${ramdisk_size}; " \
				"fi;" \
			"else " \
				"echo Error: failed to load kern image; " \
			"fi;" \
		"else " \
			"echo Error: failed to load VC image; " \
		"fi;\0"
#else
#define BOOTLINUX \
	"bootlinux=" \
		"battery_mon ${voltage_threshold_uv}; " \
		"if vc boot; then " \
			"if loadimg kern 0; then " \
				"set_watchdog off; "\
				"gpio clear 114 ; " \
				"if test ${ramdisk_size} -eq 0; then "\
					"echo WARNING: Kernel is now booting without any ramdisk;" \
					"bootm ${loadaddr}; " \
				"else " \
					"bootm ${loadaddr} ${ramdisk_loadaddr} ${ramdisk_size}; " \
				"fi;" \
			"else " \
				"echo Error: failed to load kern image; " \
			"fi;" \
		"else " \
			"echo Error: failed to load VC image; " \
		"fi;\0"
#endif

#ifdef CONFIG_ISLAND_STONE_LI
#define CONFIG_EXTRA_ENV_SETTINGS \
	"watchdog=off\0" \
	DEFAULT_BOOT_MODE \
	"ramdisk_loadaddr=0x8F000000\0" \
	"loadaddr=0x90000000\0" \
	"voltage_threshold_uv=3550000\0" \
	"mm_hardreset=mw 0x35001f08 0x1DD;\0" \
	BOOTLINUX
#else
#define CONFIG_EXTRA_ENV_SETTINGS \
	"watchdog=off\0" \
	DEFAULT_BOOT_MODE \
	"ramdisk_loadaddr=0x85000000\0" \
	"splash_size=0xb50ae\0" \
	"splash_size_in_block=0x5a9\0" \
	"voltage_threshold_uv=3550000\0" \
	"mm_hardreset=mw 0x35001f08 0x1DD;\0" \
	BOOTLINUX
#endif	

#define CONFIG_BOOTCOMMAND	\
            "run bootlinux; " 

#define CONFIG_BOOTDELAY		1

#include <config_cmd_default.h>
#define CONFIG_CMD_ENV
#define CONFIG_CMD_ASKENV
#define CONFIG_CMD_SAVES
#define CONFIG_CMD_MMC
#define CONFIG_CMD_EXT2
#define CONFIG_CMD_FAT
#define CONFIG_CMD_GPIO
#define CONFIG_CMD_LOADIMG
#define CONFIG_CMD_CLOCK

#define CONFIG_CMD_SOURCE

#undef CONFIG_CMD_NET
#undef CONFIG_CMD_NFS

#define BOARD_LATE_INIT 		1
#define CONFIG_L2_OFF			1

#undef DEBUG

#define ISLAND_SETUP_CLOCK_FREQS_ON_CHIP 1 

/*
 * NS16550 related changes are also needed in case of FPGA.
 * I don't know if these changes are need for chip.
 */
#define ISLAND_NS16550_FPGA 1

/* SD/MMC Device number */
#define ISLANDBOARD_SYS_SD_DEV 1

/* Non volatile configuration.*/
#define CONFIG_ENV_IS_IN_MMC 
#define CONFIG_CMD_SAVEENV 

#define CONFIG_SYS_MMC_ENV_DEV 0
#define CONFIG_ENV_OFFSET      0x140000

/* I2C configuration */
#define CONFIG_CMD_I2C		

#define CONFIG_HARD_I2C			1
#define CONFIG_SYS_I2C_SPEED		50000
#define CONFIG_SYS_I2C_SLAVE		0x08
#define CONFIG_SYS_I2C_BUS		0
#define CONFIG_SYS_I2C_BUS_SELECT	1
/* #define CONFIG_I2C_MULTI_BUS		1 */
#define CONFIG_SYS_I2C_INITIAL_BUS	2
#define CONFIG_DRIVER_KONA_I2C	1

#define CONFIG_HW_WATCHDOG

#define CONFIG_BCM59055_BATTERY
#define CONFIG_BOOT_VC
#define CONFIG_SPLASH_SCREEN

/* Fastboot writes to eMMC */
#define CONFIG_FASTBOOT
#define CONFIG_FASTBOOT_BOARDNAME	"Island Board"
#define CFG_FASTBOOT_TRANSFER_BUFFER (PHYS_SDRAM_1 + ISLAND_MODEM_IMG_SIZE + ISLAND_VC_IMG_SIZE)
#define CFG_FASTBOOT_TRANSFER_BUFFER_SIZE (PHYS_SDRAM_1_SIZE - ISLAND_MODEM_IMG_SIZE - ISLAND_VC_IMG_SIZE - SZ_1M)

#define CONFIG_FASTBOOT_GPT_TABLE_IN_FLASH

#define CONFIG_STORAGE_EMMC		1
#define CFG_FASTBOOT_MMC_NO		0

#if !defined( CONFIG_VCMEM_ADDR_HEX )
#define CONFIG_VCMEM_ADDR_HEX  40000000
#endif

#define  VC_XSTR(x)  #x
#define  VC_STR(x)   VC_XSTR(x)

#define  VC_XHEX(x)  0x ## x
#define  VC_HEX(x)   VC_XHEX(x)

#define  CONFIG_VCMEM_STR  VC_STR(CONFIG_VCMEM_ADDR_HEX)
#define  CONFIG_VCMEM_ADDR VC_HEX(CONFIG_VCMEM_ADDR_HEX)

#endif /* __CONFIG_H */
