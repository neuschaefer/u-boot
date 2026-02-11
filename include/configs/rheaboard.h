#ifndef __CONFIG_H
#define __CONFIG_H

#include <asm/sizes.h>
#include <asm/arch/hardware.h>
#include <asm/arch/brcm_rdb_sysmap.h>

/* U-Boot gets loaded by C-Boot */ 
#define CONFIG_LOADED_BY_CBOOT

/* Architecture, CPU, etc */
#define CONFIG_ARMV7
#define CONFIG_KONA
#define CONFIG_KONA_GPIO
#define CONFIG_TIMER
                                               
#define CONFIG_MACH_TYPE                0x0A8D
#define CONFIG_ARCH_CPU_INIT

#define CONFIG_MISC_INIT_R

#define CONFIG_SYS_TIMERBASE		TIMER_BASE_ADDR 
#define CONFIG_SYS_HZ                   1000  

/* Memory Info */
/* Set loadaddr to equal to the kernel's starting address - uImage hdrsize,
 * so uboot can XIP the uImage directly instead of relocating the uImage
 */
#if defined(CONFIG_RHEA_RAY) || defined(CONFIG_RHEA_FARADAY_EB10)
#define PHYS_SDRAM_1			0x80000000
#define PHYS_SDRAM_1_SIZE		SZ_128M
#define CONFIG_SYS_TEXT_BASE		0x82300000
#define CONFIG_LOADADDR		0x82007FC0
#define RAMDISK_LOADADDR		"ramdisk_loadaddr=0x84000000\0"

#elif defined(CONFIG_RHEA_RAY_EDN1X) || defined (CONFIG_RHEA_BERRI)
#define PHYS_SDRAM_1			0x80000000
#define PHYS_SDRAM_1_SIZE		SZ_512M
#define CONFIG_SYS_TEXT_BASE		0x82300000
#define CONFIG_LOADADDR		0x82007FC0
#define RAMDISK_LOADADDR		"ramdisk_loadaddr=0x84000000\0"

#elif  defined(CONFIG_RHEA_CLIPPER)
#define PHYS_SDRAM_1			0xA0000000
#define PHYS_SDRAM_1_SIZE		SZ_128M
#define CONFIG_SYS_TEXT_BASE		0xA2300000
#define CONFIG_LOADADDR		0xA2207FC0
#define RAMDISK_LOADADDR		"ramdisk_loadaddr=0xA4000000\0"

#elif defined(CONFIG_RHEA_CLIPPER_EDN1X)
#define PHYS_SDRAM_1			0xA0000000
#define PHYS_SDRAM_1_SIZE		SZ_512M
#define CONFIG_SYS_TEXT_BASE		0xA2300000
#define CONFIG_LOADADDR		0xA2007FC0
#define RAMDISK_LOADADDR		"ramdisk_loadaddr=0xA4000000\0"

#else
#error "Not supported project!"
#endif

#define BRCM_DT_SIZE			"brcm_dt_size=0x10\0"
#define BRCM_DT_ENABLE			"brcm_dt_enable=yes\0"
#define CONFIG_BRCM_DTBLOB_TAG
#if defined(CONFIG_RHEA_CLIPPER) || defined(CONFIG_RHEA_CLIPPER_EDN1X)
#define BRCM_DT_LOADADDR		"brcm_dt_loadaddr=0xA5000000\0"
#define CONFIG_BRCM_DT_LOADADDR	0xA5000000
#else
#define BRCM_DT_LOADADDR		"brcm_dt_loadaddr=0x85000000\0"
#define CONFIG_BRCM_DT_LOADADDR	0x85000000
#endif

#define CONFIG_SYS_SDRAM_BASE 		PHYS_SDRAM_1
#define CONFIG_SYS_SDRAM_SIZE 		PHYS_SDRAM_1_SIZE
#define CONFIG_SYS_MALLOC_LEN           (SZ_256K)

/* Modem Image */
#define RHEA_MODEM_IMG_START		(PHYS_SDRAM_1)
#define RHEA_MODEM_IMG_SIZE		(SZ_32M)


/* This value needs to be fixed */
#define CONFIG_SYS_INIT_SP_ADDR (CONFIG_SYS_SDRAM_BASE + CONFIG_SYS_SDRAM_SIZE - \
                            GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_MEMTEST_START	(PHYS_SDRAM_1 + RHEA_MODEM_IMG_SIZE)
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
#define CONFIG_SKIP_PRIMARY_GPT

#define MTDIDS_DEFAULT			""
#define MTDPARTS_DEFAULT		""

/* General U-Boot configuration */
#define CONFIG_BOOTFILE			"uImage"
#define CONFIG_SYS_PROMPT		"rheaboard> "
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
#define CONFIG_SYS_LOAD_ADDR		(PHYS_SDRAM_1 + RHEA_MODEM_IMG_SIZE + 0x2000000)

#define LINUX_BOOT_PARAM_ADDR		(PHYS_SDRAM_1 + RHEA_MODEM_IMG_SIZE + 0x100)

#define CONFIG_CMDLINE_TAG
#define CONFIG_INITRD_TAG

/* If kernel uImage is store on a raw eMMC device, we have to figure out uImage
 * size by checking its header 
 */ 
#define CONFIG_CMD_SETIMGSIZE

#if (defined(CONFIG_RHEA_RAY_EDN1X) || defined(CONFIG_RHEA_BERRI)) && !defined(CONFIG_RHEALC_BERRI)
#define CONFIG_BOOTARGS		"console=ttyS0,115200n8 mem=392M gpt pmem=88M@0x9A800000"
#define DEFAULT_BOOT_MODE	"boot_mode=android\0"
#elif defined(CONFIG_RHEA_CLIPPER)
#define CONFIG_BOOTARGS		"console=ttyS0,115200n8 mem=48M gpt pmem=48M@0xA5000000" 
#define DEFAULT_BOOT_MODE	"boot_mode=broadcom\0"
#elif defined(CONFIG_RHEA_CLIPPER_EDN1X)
#define CONFIG_BOOTARGS		"console=ttyS0,115200n8 mem=392M gpt pmem=88M@0xBA800000"
#define DEFAULT_BOOT_MODE	"boot_mode=android\0"
#else
#define CONFIG_BOOTARGS		"console=ttyS0,115200n8 mem=48M gpt pmem=48M@0x85000000"
#define DEFAULT_BOOT_MODE	"boot_mode=broadcom\0"
#endif
#ifdef CONFIG_RHEA_LC_SILICON
#define CONFIG_ARM_PLL_RATE "arm_pll_rate=1196000000\0"
#else
#define CONFIG_ARM_PLL_RATE "arm_pll_rate=1404000000\0"
#endif
#define CONFIG_EXTRA_ENV_SETTINGS \
	"watchdog=off\0" \
	RAMDISK_LOADADDR \
	BRCM_DT_ENABLE \
	BRCM_DT_LOADADDR \
	BRCM_DT_SIZE \
	DEFAULT_BOOT_MODE \
	CONFIG_ARM_PLL_RATE \
	"arm_pll_div=2\0" \
	"bootlinux=" \
		"if loadimg kern 0; then " \
			"if loadimg dt 0; then " \
				"echo Loading DT Blob...; " \
			"fi; "\
			"set_watchdog off;" \
			"boost_arm_clk;" \
			"if test ${ramdisk_size} -eq 0; then "\
				"echo WARNING: Kernel is now booting without any ramdisk;" \
				"bootm ${loadaddr}; " \
			"else " \
				"bootm ${loadaddr} ${ramdisk_loadaddr} ${ramdisk_size}; " \
			"fi; "\
		"else " \
			"echo Error: failed to load uImage header; " \
		"fi;\0"

#define CONFIG_BOOTDELAY		5

#define CONFIG_BOOTCOMMAND	\
            "run bootlinux; " 

#include <config_cmd_default.h>
#define CONFIG_CMD_ENV
#define CONFIG_CMD_ASKENV
#define CONFIG_CMD_SAVES
#define CONFIG_CMD_MMC
#define CONFIG_CMD_FAT
#define CONFIG_CMD_GPIO
#define CONFIG_CMD_LOADIMG

#define CONFIG_CMD_SOURCE

#undef CONFIG_CMD_NET
#undef CONFIG_CMD_NFS

#define BOARD_LATE_INIT 		1
#define CONFIG_L2_OFF			1

#undef DEBUG

/* SD/MMC Device number */
#define RHEABOARD_SYS_SD_DEV 1

/* Non volatile configuration.*/
#define CONFIG_ENV_IS_IN_MMC 
#define CONFIG_CMD_SAVEENV 

#define CONFIG_SYS_MMC_ENV_DEV 0
#define CONFIG_ENV_OFFSET      0x2340000 

/* I2C configuration */
#define CONFIG_CMD_I2C		
#define CONFIG_SYS_I2C_SPEED		50000
#define CONFIG_I2C_MULTI_BUS		1 
#define CONFIG_SYS_I2C_INITIAL_BUS	0
#define CONFIG_DRIVER_KONA_I2C	1

/* #define CONFIG_HW_WATCHDOG */

#if 0
#define CONFIG_BCM59055_BATTERY
#define CONFIG_BOOT_VC
#define CONFIG_SPLASH_SCREEN
#endif

/* Fastboot writes to eMMC */
#define CONFIG_FASTBOOT
#define CONFIG_FASTBOOT_BOARDNAME	"Rhea Board"
#define CFG_FASTBOOT_TRANSFER_BUFFER (PHYS_SDRAM_1 + RHEA_MODEM_IMG_SIZE)
#define CFG_FASTBOOT_TRANSFER_BUFFER_SIZE (PHYS_SDRAM_1_SIZE - RHEA_MODEM_IMG_SIZE - SZ_1M)

#define CONFIG_FASTBOOT_GPT_TABLE_IN_FLASH

#define CONFIG_STORAGE_EMMC		1
#define CFG_FASTBOOT_MMC_NO		0

#define CONFIG_BOOST_ARM_CLK
#define DEFAULT_PLL_RATE		1300000000
#define XTAL_RATE			26000000
#define DEFAULT_PLL_DIV			3

#define CONFIG_CMD_CLOCK

/* Enable PMU watchdog driver */
#ifndef CONFIG_RHEA_FARADAY_EB10
#define CONFIG_BCM59055_WDT
#endif
#endif /* __CONFIG_H */
