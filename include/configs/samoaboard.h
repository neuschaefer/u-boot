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
                                               
#define CONFIG_MACH_TYPE                0x0A8E
#define CONFIG_ARCH_CPU_INIT

#define CONFIG_SYS_UBOOT_BASE           CONFIG_SYS_TEXT_BASE 

#define CONFIG_MISC_INIT_R

#define CONFIG_SYS_TIMERBASE		TIMER_BASE_ADDR 
#define CONFIG_SYS_HZ                   1000  

/* Memory Info */
#define CONFIG_SYS_MALLOC_LEN           (SZ_256K) 
#define PHYS_SDRAM_1			KONA_RAM_START 
#define PHYS_SDRAM_1_SIZE		SZ_128M
#define CONFIG_SYS_SDRAM_BASE PHYS_SDRAM_1
#define CONFIG_SYS_SDRAM_SIZE PHYS_SDRAM_1_SIZE

/* Modem Image */
#define SAMOA_MODEM_IMG_START	(KONA_RAM_START)
#define SAMOA_MODEM_IMG_SIZE	(SZ_32M)  


/* This value needs to be fixed */
#define CONFIG_SYS_INIT_SP_ADDR (CONFIG_SYS_SDRAM_BASE + CONFIG_SYS_SDRAM_SIZE - \
                            GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_MEMTEST_START	(PHYS_SDRAM_1 + SAMOA_MODEM_IMG_SIZE)
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
#define CONFIG_SYS_PROMPT		"samoaboard> "
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
#define CONFIG_SYS_LOAD_ADDR		(KONA_RAM_START + SAMOA_MODEM_IMG_SIZE + 0x2000000)

#define LINUX_BOOT_PARAM_ADDR		(KONA_RAM_START + SAMOA_MODEM_IMG_SIZE + 0x100)

#define CONFIG_CMDLINE_TAG
#define CONFIG_BOOTARGS     "console=ttyS0,115200n8 mem=96M gpt v3d_mem=33554432"
/* If kernel uImage is store on a raw eMMC device, we have to figure out uImage
 * size by checking its header 
 */ 
#define CONFIG_CMD_SETIMGSIZE

#define CONFIG_EXTRA_ENV_SETTINGS \
    "watchdog=off\0" \
    "loadaddr=0x84000000\0" \
    "boot_eMMC=1\0" \
    "bootlinux=" \
    "if test ${boot_eMMC} -eq 1; then "\
    	"if mmc read 0 ${loadaddr} 0x12000 1; then " \
    		"valid_linux_image=1; "\
		"else " \
			"echo Error: failed to load uImage header; " \
			"valid_linux_image=0; " \
		"fi;" \
		"if test ${valid_linux_image} -eq 1; then " \
			"set_img_size ${loadaddr};" \
			"mmc read 0 ${loadaddr} 0x12000 ${uimg_size};" \
			"set_watchdog off;" \
			"boost_arm_clk;" \
			"bootm ${loadaddr}; " \
		"fi;" \
	"else " \
	 	"if mmc rescan 1; then " \
			"if fatload mmc 1:1 ${loadaddr} firmware/uImage; then " \
				"valid_linux_image=1; " \
			"else " \
				"echo Error: failed to load uImage; " \
				"valid_linux_image=0; " \
			"fi;" \
			"if test ${valid_linux_image} -eq 1; then " \
				"set_watchdog off;" \
				"boost_arm_clk;" \
				"bootm ${loadaddr}; " \
			"fi; " \
		"else " \
			"echo Error initializing SD Card.;" \
		"fi;" \
	"fi;\0" 

#define CONFIG_BOOTCOMMAND	\
            "run bootlinux; " 

#define CONFIG_BOOTDELAY		1

#include <config_cmd_default.h>
#define CONFIG_CMD_ENV
#define CONFIG_CMD_ASKENV
#define CONFIG_CMD_SAVES
#define CONFIG_CMD_MMC
#define CONFIG_CMD_FAT
#define CONFIG_CMD_GPIO

#undef CONFIG_CMD_NET
#undef CONFIG_CMD_NFS

#define BOARD_LATE_INIT 		1
#define CONFIG_L2_OFF			1

#undef DEBUG

/* SD/MMC Device number */
#define SAMOABOARD_SYS_SD_DEV 1

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


/* Fastboot writes to eMMC */
#define CONFIG_FASTBOOT
#define CONFIG_FASTBOOT_BOARDNAME	"Samoa Board"
#define CFG_FASTBOOT_TRANSFER_BUFFER (PHYS_SDRAM_1 + SAMOA_MODEM_IMG_SIZE)
#define CFG_FASTBOOT_TRANSFER_BUFFER_SIZE (PHYS_SDRAM_1_SIZE - SAMOA_MODEM_IMG_SIZE - SZ_1M)

#define CONFIG_STORAGE_EMMC		1
#define CFG_FASTBOOT_MMC_NO		0

#if 0

#define CONFIG_BOOST_ARM_CLK
#define DEFAULT_PLL_RATE		1300000000
#define XTAL_RATE			26000000
#define DEFAULT_PLL_DIV			3

#define CONFIG_CMD_CLOCK
#endif

#endif /* __CONFIG_H */
