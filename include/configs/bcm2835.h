#ifndef __CONFIG_H
#define __CONFIG_H

#include <asm/sizes.h>
#include <asm/arch/hardware.h>

/* Architecture, CPU, etc */
#define CONFIG_ARM1176

#define CONFIG_BCM2835
#define CONFIG_BCM2708
#define CONFIG_MACH_TYPE 2708

//#define CONFIG_VCEB
//#define CONFIG_VCEB_DEBUG
// Shared memory VC base address
//#define CONFIG_VCEB_SHARED_MEM_BASE 0x07800000
// Shared memory as seen by ARM
//#define CONFIG_VCEB_ARM_PHYS_BASE   0x0A000000

#define CONFIG_BCM2835_MAILBOX

#define CONFIG_SYS_UBOOT_BASE       CONFIG_SYS_TEXT_BASE
#define CONFIG_ARCH_CPU_INIT
#define CONFIG_ARCH_MISC_INIT
#define CONFIG_MISC_INIT_R

#define CONFIG_SHOW_BOOT_PROGRESS

//#define CONFIG_PERIPORT_REMAP
#define CONFIG_PERIPORT_BASE    0x08000000
#define CONFIG_PERIPORT_SIZE    0x13

#define CONFIG_SYS_TIMERBASE		ST_BASE
#define CONFIG_SYS_HZ			1000000

/* Memory Info */
#define CONFIG_SYS_MALLOC_LEN		(SZ_1M)
/* #define CONFIG_SYS_GBL_DATA_SIZE	128 */
#define PHYS_SDRAM_1			BCM2708_SDRAM_BASE
#define PHYS_SDRAM_1_SIZE		0x06000000
#define CONFIG_SYS_SDRAM_BASE PHYS_SDRAM_1

/* This value needs to be fixed */
#define CONFIG_SYS_INIT_SP_ADDR (CONFIG_SYS_SDRAM_BASE + 0x1000 - \
                            GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_MEMTEST_START	PHYS_SDRAM_1
#define CONFIG_SYS_MEMTEST_END		(PHYS_SDRAM_1 + 16*1024*1024)
#define CONFIG_NR_DRAM_BANKS		1
#define CONFIG_STACKSIZE		(SZ_1M)

/* serial port (PL011) configuration */
#define CONFIG_SYS_SERIAL0      UART0_BASE
#define CONFIG_SYS_SERIAL1      UART1_BASE
#define CONFIG_PL011_SERIAL
#define CONFIG_PL011_CLOCK 3000000
#define CONFIG_CONS_INDEX   0
#define CONFIG_BAUDRATE   115200
#define CONFIG_SYS_BAUDRATE_TABLE   { 9600, 19200, 38400, 57600, 115200 }

#define CONFIG_PL01x_PORTS              \
    {(void *)CONFIG_SYS_SERIAL0,    \
     (void *)CONFIG_SYS_SERIAL1 }



/* Flash and environment info */
#define CONFIG_SYS_NO_FLASH

#define CONFIG_ENV_IS_IN_NAND
#define CONFIG_ENV_SIZE				(SZ_256K)
#define CONFIG_ENV_OFFSET			(0xc00000)
#define CONFIG_ENV_OFFSET_REDUND	(0xc40000)
#define CONFIG_SPLASH_OFFSET		(CONFIG_ENV_OFFSET_REDUND + CONFIG_ENV_SIZE)
 
#define CONFIG_SYS_MAX_NAND_DEVICE	1
#define NAND_MAX_CHIPS			1
#define CONFIG_NAND_BCM2708_DRIVER
#define CONFIG_SYS_NAND_BASE	0 // For now.
/*
#define CONFIG_SYS_NAND_HW_ECC
#define CONFIG_SYS_NAND_1BIT_ECC
#define CONFIG_SYS_NAND_CS		2
#define CONFIG_SYS_NAND_USE_FLASH_BBT

#define CONFIG_SYS_CLE_MASK		0x10
#define CONFIG_SYS_ALE_MASK		0x8

#define CONFIG_MTD_PARTITIONS
#define CONFIG_CMD_MTDPARTS
#define CONFIG_MTD_DEVICE
#define CONFIG_JFFS2_NAND

#define CONFIG_ENV_OFFSET		0x180000
#define DEF_BOOTM			""
*/

#define CONFIG_MMC
#define CONFIG_GENERIC_MMC
#define CONFIG_BCM2708_MMC
#define CONFIG_DOS_PARTITION


#define MTDIDS_DEFAULT			""
#define MTDPARTS_DEFAULT		""

/* General U-Boot configuration */
#define CONFIG_BOOTFILE			"uImage"
#define CONFIG_SYS_PROMPT		"bcm2835 > "
#define CONFIG_SYS_CBSIZE		1024
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
#define CONFIG_SYS_PBSIZE		(CONFIG_SYS_CBSIZE +		\
					 sizeof(CONFIG_SYS_PROMPT) + 16)
#define CONFIG_SYS_LOAD_ADDR		(CONFIG_SYS_MEMTEST_START + 0x7FC0)
#define LINUX_BOOT_PARAM_ADDR		(CONFIG_SYS_MEMTEST_START + 0x100)
#define CONFIG_CMDLINE_TAG
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_INITRD_TAG
#define CONFIG_BOOTARGS     "mem=80M console=tty0 dwc_otg.otg_cap=2 dwc_otg.phy_utmi_width=8 dwc_otg.data_fifo_size=4080 dwc_otg.host_rx_fifo_size=774 dwc_otg.host_nperio_tx_fifo_size=256 dwc_otg.host_perio_tx_fifo_size=512 dwc_otg.lpm_enable=0 dwc_otg.dma_enable=1 dwc_otg.debug=0x0 dwc_otg.dma_desc_enable=0 kgdb=ttyAMA0,38400 kgdboc=ttyAMA0,115200,halt console=ttyAMA0,115200"


#define CONFIG_EXTRA_ENV_SETTINGS \
    "vc_image=firmware.bin\0" \
    "vc_loadaddr=0x05000000\0" \
    "loadaddr=0x7fc0\0" \
    "bootsd=" \
        "if mmc rescan 0; then " \
            "if fatload mmc 0:1 ${vc_loadaddr} ${vc_image}; then " \
				"if fatload mmc 0:1 ${loadaddr} uImage; then " \
					"valid_linux_image=1; " \
				"else " \
					"echo Error: failed to load uImage; " \
					"valid_linux_image=0; " \
				"fi;" \
				"bootvc ${vc_loadaddr}; " \
				"sleep 3; " \
                "vctvcontrol auto; " \
				"if test ${valid_linux_image} -eq 1; then " \
					"bootm ${loadaddr}; " \
				"fi; " \
            "else " \
                "echo Error: failed to load VC image ${vc_image}; " \
            "fi; " \
        "else " \
            "echo Error initializing SD Card.;" \
        "fi;\0" \
    "flash_nand=echo Flashing nand.. ; " \
        "if mmc rescan 0; then " \
			"if fatload mmc 0 0x0 splash.264; then " \
				"setenv splash_size ${filesize};" \
				"saveenv;" \
				"nand erase 0xC80000 0x180000;" \
				"nand write 0x0 0xC80000 0x100000;" \
			"fi;" \
            "if fatload mmc 0 0x0 uimage; then " \
				"nand erase 0xE00000 0x800000;"	\
                "nand write 0x0 0xE00000 0x800000; " \
			"fi;" \
            "if fatload mmc 0 0x0 firmware.bin; then " \
				"nand erase 0x1A00000 0x200000;" \
                "nand write 0x0 0x1A00000 0x200000; " \
            "fi;" \
        "fi;\0" \
    "show_splash=echo Starting splash screen..; " \
        "if nand read 0x4600000 0xC80000 0x100000; then " \
                "vctvcontrol auto; " \
                "vcsplash 0x4600000 ${splash_size}; " \
        "fi;\0" \
    "vc_init=" \
        "if nand read ${vc_loadaddr} 0x1A00000 0x200000; then " \
            "bootvc ${vc_loadaddr}; " \
        "fi;\0"


#define CONFIG_BOOTCOMMAND \
    "run vc_init; " \
    "run show_splash; " \
    "nboot ${loadaddr} 0 0xe00000; " \
    "bootm ${loadaddr};"

#define CONFIG_BOOTDELAY		1



#include <config_cmd_default.h>
#define CONFIG_CMD_VC
#define CONFIG_CMD_ENV
#define CONFIG_CMD_ASKENV
#define CONFIG_CMD_SAVES
#define CONFIG_CMD_MMC
#define CONFIG_CMD_NAND
#define CONFIG_CMD_EXT2
#define CONFIG_CMD_FAT
//#define CONFIG_CMD_CRAMFS

#undef CONFIG_CMD_NET
#undef CONFIG_CMD_NFS

#endif /* __CONFIG_H */
