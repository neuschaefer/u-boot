/*
 * (C) Copyright 2003
 * Texas Instruments.
 * Kshitij Gupta <kshitij@ti.com>
 * Configuation settings for the TI OMAP Innovator board.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <asm/arch-arm926ejs/sizes.h>


/*-----------------------------------------------------------------------
 * SPI flash layout
 *-----------------------------------------------------------------------*/
/* 
 * Old
 * ----------------------------------
 * u-boot:      0x40000000 (192 KB)
 * u-boot env:  0x40030000 ( 64 KB)
 * rootfs:      0x40100000 (  3 MB)
 * kernel:      0x40400000 (  2 MB)
 * kernel bak:  0x40600000 (  2 MB)
 * jffs2:       0x40800000 (  4 MB)
 *
 * SVB
 * ----------------------------------
 * u-boot:      0x40000000 (192 KB)
 * u-boot env:  0x40030000 ( 32 KB)
 * u-boot ps:   0x40038000 ( 32 KB)
 * jffs2:       0x40040000 (768 KB)
 *
 * Whoville
 * ----------------------------------
 * u-boot:      0x40000000 (192 KB)
 * u-boot env:  0x40030000 ( 32 KB)
 * u-boot ps:   0x40038000 ( 32 KB)
 * jffs2:       0x40040000 (768 KB)
 */


/*-----------------------------------------------------------------------
 * NAND flash layout
 *-----------------------------------------------------------------------*/
/* 
 * SVB
 * ----------------------------------------------
 * reserve:     0x00000000 ( 1 MB)
 * kernel:      0x00100000 ( 2 MB) 
 * rootfs:      0x00300000 (13 MB)
 *
 * Whoville
 * ----------------------------------------------
 * reserve:     0x00000000 ( 1 MB)
 * kernel:      0x00100000 ( 2 MB) 
 * rootfs:      0x00300000 (13 MB)
 */


/*-----------------------------------------------------------------------
 * RAM layout
 *-----------------------------------------------------------------------*/
/* 
 * SVB
 * ----------------------------------------------
 * 0x00008000 : kernel
 * 0x01000000 : u-boot load address
 * 0x06700000 : u-boot relocate address
 *
 * Whoville
 * ----------------------------------------------
 * 0x00008000 : kernel
 * 0x01000000 : u-boot load address
 * 0x06700000 : u-boot relocate address
 */
 
 
/*-----------------------------------------------------------------------
 * Partition
 *-----------------------------------------------------------------------*/
/* 
 * 1 : kernel 1
 * 2 : rootfs 1
 * 3 : kernel 2
 * 4 : extended
 * 5 : rootfs 2
 * 6 : persistent data
 * 7 : maser
 */


/*-----------------------------------------------------------------------
 * CPU core
 *-----------------------------------------------------------------------*/
#define CONFIG_ARM926EJS    1   /* This is an arm926ejs CPU core  */


/*-----------------------------------------------------------------------
 * 
 *-----------------------------------------------------------------------*/
#define CONFIG_WPCM450      1


/*-----------------------------------------------------------------------
 * Platform
 *-----------------------------------------------------------------------*/
/* platform dependent, only can select one of them */
#if 0

#define CONFIG_WPCM450_SVB          1
#if 1
#define CONFIG_COMPATIBLE_Z1_CHIP   1   /* be compatible to Z1 chip */
#endif

#define CONFIG_WPCM450_PLATFORM_ID  "WEVB"

#else

#define CONFIG_WPCM450_WHOVILLE     1
#if 0
#define CONFIG_WPCM450_WHOVILLE_X00 1   /*  64 MB RAM */
#else
#define CONFIG_WPCM450_WHOVILLE_X01 1   /* 128 MB RAM */
#endif

#if 0
/* avoid crashing on non-muxed system */
#define CONFIG_WPCM450_WORKAROUND   1
#endif

#define CONFIG_WPCM450_PLATFORM_ID  "WHOV"

#endif


/*-----------------------------------------------------------------------
 * ram version
 *-----------------------------------------------------------------------*/
#if 0
#define CONFIG_WPCM450_RAM_VER      1
#endif

/* #define CONFIG_SKIP_LOWLEVEL_INIT */
/* #define CONFIG_SKIP_RELOCATE_UBOOT */


/*-----------------------------------------------------------------------
 * Boot Option
 *-----------------------------------------------------------------------*/
#ifdef CONFIG_WPCM450_SVB

#if 1
/* delay before autoboot */
#define CONFIG_BOOTDELAY    3
#else
/* autoboot disabled */
#define CONFIG_BOOTDELAY    -1
#endif

#else
#ifdef CONFIG_WPCM450_WHOVILLE

/* delay before autoboot */
#define CONFIG_BOOTDELAY    3

#endif
#endif


#if 0
#define CONFIG_PREBOOT  \
        "echo;" \
        "echo *******************************************************************;" \
        "echo 1. Type \"set ethaddr AA:BB:CC:DD:EE:FF\" to set MAC address;" \
        "echo 2. Type \"dchp\" to get net configuration or set variables manually;" \
        "echo -- \"set ipaddr 192.168.0.100\" for IP address;" \
        "echo -- \"set gatewayip 192.168.0.254\" for gateway IP address;" \
        "echo -- \"set netmask 255.255.255.0\" for network mask;" \
        "echo 3. Type \"set serverip 192.168.0.50\" for TFTP IP address;" \
        "echo 4. Type \"set rootpath user/\" to set sub-folder for TFTP;" \
        "echo 5. Type \"set nfsip 192.168.0.50\" to set NFS IP address;" \
        "echo 6. Type \"set nfsroot /nfs_root\" to set NFS root;" \
        "echo 7. Type \"saveenv\" to save changes to flash;" \
        "echo 8. Type \"run nfs_boot\" to mount root filesystem over NFS;" \
        "echo -- or \"fwu boot\" to boot from SD/MMC;" \
        "echo ;" \
        "echo Type \"? or help\" to get help;" \
        "echo *******************************************************************;" \
        "echo"
#endif


/* #define CONFIG_BOOTCOMMAND   "run spi_boot" */
/* #define CONFIG_BOOTCOMMAND   "run nand_boot" */

/*
#ifdef CONFIG_WPCM450_SVB
#define CONFIG_BOOTCOMMAND  "fwu boot"
#else
#ifdef CONFIG_WPCM450_WHOVILLE
#define CONFIG_BOOTCOMMAND  "run nand_boot"
#endif
#endif
#define CONFIG_BOOTCOMMAND  "fwu boot"
*/

/* define the boot command */
#define CONFIG_BOOTCOMMAND  "console init"


/* file to load */
#define CONFIG_BOOTFILE     ""


/*-----------------------------------------------------------------------
 * Version Identity
 *-----------------------------------------------------------------------*/

/* should be the same as U_BOOT_VERSION */
#define CONFIG_UBOOT_VERSION            "1"
#define CONFIG_UBOOT_PATCHLEVEL         "2"
#define CONFIG_UBOOT_SUBLEVEL           "0"

#define CONFIG_AVCT_VERSION             "1"
#define CONFIG_AVCT_PATCHLEVEL          "13"
#define CONFIG_AVCT_SUBLEVEL            "7"

#define CONFIG_IDENT_STRING_VERSION     " (" CONFIG_AVCT_VERSION "." \
                                             CONFIG_AVCT_PATCHLEVEL "." \
                                             CONFIG_AVCT_SUBLEVEL ")"

#ifdef CONFIG_WPCM450_SVB
#define CONFIG_IDENT_STRING_PLATFORM    " EVB"
#else
#ifdef CONFIG_WPCM450_WHOVILLE
#define CONFIG_IDENT_STRING_PLATFORM    " Whoville"
#endif
#endif

#ifdef CONFIG_WPCM450_RAM_VER
#define CONFIG_IDENT_STRING_MISC        " RAM"
#else
#define CONFIG_IDENT_STRING_MISC        ""
#endif

#define CONFIG_IDENT_STRING     " Avocent" \
                                CONFIG_IDENT_STRING_VERSION \
                                CONFIG_IDENT_STRING_PLATFORM \
                                CONFIG_IDENT_STRING_MISC


/*-----------------------------------------------------------------------
 * debug information
 *-----------------------------------------------------------------------*/
#if 0
#define DEBUG           1
#endif


/*-----------------------------------------------------------------------
 * 
 *-----------------------------------------------------------------------*/
#define CONFIG_DISPLAY_CPUINFO      1   /* display cpu info (and speed) */
#define CONFIG_DISPLAY_BOARDINFO    1   /* display board info       */


/*-----------------------------------------------------------------------
 * 
 *-----------------------------------------------------------------------*/
/* input clock of PLL */
/* the WPCM450 has 12MHz input clock */
#define CONFIG_SYS_CLK_FREQ     12000000

#define CFG_TIMERBASE           0xB8001000


/*-----------------------------------------------------------------------
 * IRQ
 *-----------------------------------------------------------------------*/
#undef CONFIG_USE_IRQ   /* we don't need IRQ/FIQ stuff */
/* #define CONFIG_INTEGRATOR 1 */


/*-----------------------------------------------------------------------
 * Miscellaneous
 *-----------------------------------------------------------------------*/
#define CONFIG_MISC_INIT_R


/*-----------------------------------------------------------------------
 * TAGs
 *-----------------------------------------------------------------------*/
#define CONFIG_BOOTARGS         ""
#define CONFIG_CMDLINE_TAG      1       /* enable passing of ATAGs  */
/* #define CONFIG_INITRD_TAG       1 */

/* #define CONFIG_BOOTARGS         "root=/dev/mmcblk0p2 mem=120M console=ttyS0" */
/* #define CONFIG_BOOTARGS         "ramdisk_size=49152 root=/dev/ram0 rw rdinit=/initsh mem=120M console=ttyS0" */

/*-----------------------------------------------------------------------
 * Memory Allocate
 *-----------------------------------------------------------------------*/
#define CFG_MALLOC_LEN      (CFG_ENV_SECT_SIZE + CONFIG_STACKSIZE)
#define CFG_GBL_DATA_SIZE   128 /* size in bytes reserved for initial data */


/*-----------------------------------------------------------------------
 * 
 *-----------------------------------------------------------------------*/
#define CFG_HZ  100000          /* 1 tick = 10 us */ 


/*-----------------------------------------------------------------------
 * Serial Configuration
 *-----------------------------------------------------------------------*/
#define  CONFIG_UART0_CONSOLE   1

/* allow to overwrite serial and ethaddr */
#define CONFIG_ENV_OVERWRITE

#define CONFIG_CONS_INDEX       1
#define CONFIG_BAUDRATE         115200
#define CFG_BAUDRATE_TABLE      {9600, 19200, 38400, 57600, 115200}


/*-----------------------------------------------------------------------
 * Command Support
 *-----------------------------------------------------------------------*/
#ifdef CONFIG_WPCM450_SVB

#define CONFIG_COMMANDS ((CONFIG_CMD_DFL \
                          | CFG_CMD_NET \
                          | CFG_CMD_ENV \
                          | CFG_CMD_DHCP \
                          | CFG_CMD_PING \
                          | CFG_CMD_NAND \
                          | CFG_CMD_MMC) \
                         & ~CFG_CMD_AUTOSCRIPT \
                         & ~CFG_CMD_LOADS \
                         & ~CFG_CMD_CONSOLE \
                        )

#else
#ifdef CONFIG_WPCM450_WHOVILLE

#define CONFIG_COMMANDS ((CONFIG_CMD_DFL \
                          | CFG_CMD_NET \
                          | CFG_CMD_ENV \
                          | CFG_CMD_DHCP \
                          | CFG_CMD_PING \
                          | CFG_CMD_MMC) \
                         & ~CFG_CMD_AUTOSCRIPT \
                         & ~CFG_CMD_LOADS \
                         & ~CFG_CMD_CONSOLE \
                        )

#endif
#endif


/*
#define CONFIG_COMMANDS (CONFIG_CMD_DFL \
                         | CFG_CMD_NET \
                         | CFG_CMD_ENV \
                         | CFG_CMD_DHCP \
                         | CFG_CMD_PING \
                         | CFG_CMD_NAND \
                         | CFG_CMD_JFFS2 \
                         | CFG_CMD_FAT \
                        )
*/

/* this must be included AFTER the definition of CONFIG_COMMANDS (if any) */
#include <cmd_confdefs.h>


/*-----------------------------------------------------------------------
 * 
 *-----------------------------------------------------------------------*/
#define CONFIG_BOOTP_MASK   CONFIG_BOOTP_DEFAULT


/*-----------------------------------------------------------------------
 * Ethernet Configuration
 *-----------------------------------------------------------------------*/
#define CONFIG_WPCM450NIC           1
#define CONFIG_WPCM450NIC_USE_EMC2  1           /* use emc2 by defualt */
#define CONFIG_NET_MULTI

#ifdef CONFIG_WPCM450_SVB
#define CONFIG_DP83848C_PHY
#else
#ifdef CONFIG_WPCM450_WHOVILLE
#define CONFIG_BCM5221_PHY
#endif
#endif


/* set tftp retry count to 100 * 2 = 200 */
#define CONFIG_NET_RETRY_COUNT      100

/* #define CONFIG_ETH_DEVS 2 */

#define CONFIG_NETMASK      255.255.255.0       /* talk on local net */
#define CONFIG_IPADDR       192.168.0.120       /* static IP */
#define CONFIG_SERVERIP     192.168.0.100       /* current server IP */
#define CONFIG_GATEWAYIP    192.168.0.1         /* gateway IP */
#define CONFIG_ETHADDR      02:00:00:00:00:01   /* MAC address */
#define CONFIG_ROOTPATH                         /* default root path */

/* #define CONFIG_MII       1 */
/* #define CONFIG_PHY_TYPE  PHY_DP83848C */

/* #define CONFIG_MAC_PARTITION */


/*-----------------------------------------------------------------------
 * 
 *-----------------------------------------------------------------------*/
#if (CONFIG_COMMANDS & CFG_CMD_KGDB)
#define CONFIG_KGDB_BAUDRATE    115200  /* speed to run kgdb serial port */
#define CONFIG_KGDB_SER_INDEX   1   /* which serial port to use */
#endif


/*-----------------------------------------------------------------------
 * NAND Flash
 *-----------------------------------------------------------------------*/
#define CFG_MAX_NAND_DEVICE     1
#define CFG_NAND_BASE           0x45000000
#define NAND_MAX_CHIPS          CFG_MAX_NAND_DEVICE
#define CFG_NAND_BASE_LIST      {CFG_NAND_BASE}

/* 
 * X-Bus 0x44000000-0x4400007F nCS0
 *       0x44800000-0x4480007F nCS1
 *       0x45000000-0x4500007F nCS2
 */


/*-----------------------------------------------------------------------
 * JFFS2 partitions
 *-----------------------------------------------------------------------*/
/* mtdparts command line support */
#define CONFIG_JFFS2_CMDLINE

#ifdef CONFIG_WPCM450_SVB
#define MTDIDS_DEFAULT      "nand0=NAND512W3A"
#define MTDPARTS_DEFAULT    "mtdparts=NAND512W3A:3m@1m(kernel),12m(rootfs),48m(data)"
#else
#ifdef CONFIG_WPCM450_WHOVILLE
#define MTDIDS_DEFAULT      "nand0=NAND08GW3B"
#define MTDPARTS_DEFAULT    "mtdparts=NAND08GW3B:3m@1m(kernel),12m(rootfs),48m(data)"
#endif
#endif


#define CONFIG_JFFS2_NAND

#define CONFIG_JFFS2_DEV            "nand0"
#define CONFIG_JFFS2_PART_OFFSET    0x00100000
#define CONFIG_JFFS2_PART_SIZE      0x01D00000


/*-----------------------------------------------------------------------
 * Miscellaneous configurable options
 *-----------------------------------------------------------------------*/
#define CFG_LONGHELP                            /* undef to save memory       */
#define CFG_PROMPT          "[uboot_wpcm450]# " /* Monitor Command Prompt     */

#if (CONFIG_COMMANDS & CFG_CMD_KGDB)
#define CFG_CBSIZE		1024	                /* Console I/O Buffer Size  */
#else
#define CFG_CBSIZE		256	                    /* Console I/O Buffer Size  */
#endif

/* Print Buffer Size */
#define CFG_PBSIZE          (CFG_CBSIZE + sizeof(CFG_PROMPT) + 16)
#define CFG_MAXARGS         16                  /* max number of command args */
#define CFG_BARGSIZE        CFG_CBSIZE          /* Boot Argument Buffer Size  */

#define CONFIG_CMDLINE_EDITING

#define CFG_MEMTEST_START   0x00000000  /* memtest works on */
#define CFG_MEMTEST_END     0x00200000  /*  MB in DRAM    */

#undef  CFG_CLKS_IN_HZ                  /* everything, incl board info, in Hz */

#define CFG_LOAD_ADDR       0x01000000  /* default load address */

/*Physical start address of boot monitor code */
#define CFG_MONITOR_BASE    TEXT_BASE 


/*-----------------------------------------------------------------------
 * Stack sizes
 *-----------------------------------------------------------------------*/
#define CONFIG_STACKSIZE        (1024*1024) /* regular stack */

#ifdef CONFIG_USE_IRQ
#define CONFIG_STACKSIZE_IRQ    (4*1024)    /* IRQ stack */
#define CONFIG_STACKSIZE_FIQ    (4*1024)    /* FIQ stack */
#endif


/*-----------------------------------------------------------------------
 * Physical Memory Map
 *-----------------------------------------------------------------------*/
#define CONFIG_NR_DRAM_BANKS    1   /* we have 1 bank of DRAM */
#define PHYS_SDRAM_1            0x00000000  /* SDRAM Bank #1 */
#define PHYS_SDRAM_1_SIZE       0x08000000  /* 128 MB */


/*-----------------------------------------------------------------------
 * 
 *-----------------------------------------------------------------------*/
#define PHYS_FLASH_1            0x40000000  /* Flash Bank #1 */
#define PHYS_FLASH_2            0x40400000  /* Flash Bank #2 */
#define PHYS_FLASH_3            0x40800000  /* Flash Bank #3 */
#define PHYS_FLASH_4            0x40C00000  /* Flash Bank #4 */

#define CFG_FLASH_BASE          PHYS_FLASH_1


/*-----------------------------------------------------------------------
 * SPI Flash
 *-----------------------------------------------------------------------*/
#define CONFIG_WPCM450_SPI_DRIVER 1

#define CFG_MAX_FLASH_SECT        256   /* allow to support 256*64 K = 16 M */

#ifdef CONFIG_WPCM450_SVB
#define CFG_MAX_FLASH_BANKS       4
#define CFG_FLASH_BANKS_LIST      {PHYS_FLASH_1, PHYS_FLASH_2, PHYS_FLASH_3, PHYS_FLASH_4}
#else
#ifdef CONFIG_WPCM450_WHOVILLE
#define CFG_MAX_FLASH_BANKS       1
#define CFG_FLASH_BANKS_LIST      {PHYS_FLASH_1}
#endif
#endif


/*-----------------------------------------------------------------------
 * U-boot Environment Data
 *-----------------------------------------------------------------------*/
#define CFG_ENV_IS_IN_FLASH     1
#define CFG_ENV_SECT_SIZE       0x10000 /* the size of one sector */
#define CFG_ENV_SIZE            0x02000 /* Total Size of Environment Sector */
#define CFG_ENV_OFFSET          0x30000 /* environment starts here  */
#define CFG_ENV_ADDR            (CFG_FLASH_BASE + CFG_ENV_OFFSET) /* addr of environment */


/*-----------------------------------------------------------------------
 * U-boot Persistent Storage
 *-----------------------------------------------------------------------*/
#define CFG_PS_SECT_SIZE        0x10000 /* the size of one sector */
#define CFG_PS_SIZE             0x00100 /* Total Size of PS Sector */
#define CFG_PS_OFFSET           0x34000 /* PS starts here  */
#define CFG_PS_ADDR             (CFG_FLASH_BASE + CFG_PS_OFFSET) /* addr of PS */


/*-----------------------------------------------------------------------
 * SD/MMC 
 *-----------------------------------------------------------------------*/
#define CONFIG_MMC              1
#define CFG_MMC_BASE            0x50000000
#define CFG_MMC_MAPPING_SIZE    0x20000000  /* 512 MB */

/* #define CONFIG_DOS_PARTITION */
/* #define CONFIG_SUPPORT_VFAT */


/*-----------------------------------------------------------------------
 * Boot Status Support
 *-----------------------------------------------------------------------*/
#define CONFIG_WPCM450_BOOT_STATUS  1


/*-----------------------------------------------------------------------
 * 
 *-----------------------------------------------------------------------*/
#ifdef CONFIG_WPCM450_SVB

#define CFG_AU "au=run spi_au\0"
#define CFG_BU "bu=run spi_bu\0"
#define CFG_KU "ku=run spi_ku\0"
#define CFG_RU "ru=run spi_ru\0"

#else
#ifdef CONFIG_WPCM450_WHOVILLE

#define CFG_AU "au=run fwu_au erase_ps erase_env\0"
#define CFG_BU "bu=run spi_bu\0"
#define CFG_KU
#define CFG_RU

#endif
#endif


#if (CONFIG_COMMANDS & CFG_CMD_NAND)
#define CONFIG_WPCM450_STORAGE_NAND     1
#endif

#ifdef CONFIG_WPCM450_STORAGE_NAND
#define CFG_STORAGE_NAND \
        "n_kernel_load=308000\0" \
        "n_kernel_start=100000\0" \
        "n_kernel_size=300000\0" \
        "n_rootfs_load=2600000\0" \
        "n_rootfs_start=300000\0" \
        "n_rootfs_size=d00000\0" \
        "nand_ku=set bootfile $(rootpath)kernel.bin;" \
                "set fl_start $(n_kernel_start);" \
                "set fl_size $(n_kernel_size);" \
                "run n_uploadfile\0" \
        "nand_ru=set bootfile $(rootpath)rootfs.bin;" \
                "set fl_start $(n_rootfs_start);" \
                "set fl_size $(n_rootfs_size);" \
                "run n_uploadfile\0" \
        "nand_boot=nand read $(n_kernel_load) $(n_kernel_start) $(n_kernel_size);" \
                  "nand read $(n_rootfs_load) $(n_rootfs_start) $(n_rootfs_size);" \
                  "bootm $(n_kernel_load) $(n_rootfs_load)\0" \
        "nand_kernel=nand read $(n_kernel_load) $(n_kernel_start) $(n_kernel_size);" \
                    "bootm $(n_kernel_load)\0" \
        "n_uploadfile=$(interface) $(offset);" \
                     "nand protect off;" \
                     "nand erase $(fl_start) $(fl_size);" \
                     "nand write $(offset) $(fl_start) $(fl_size);" \
                     "nand protect on\0"
#else
#define CFG_STORAGE_NAND
#endif


#ifdef CONFIG_WPCM450_STORAGE_RAW_SD
#define CFG_STORAGE_RAW_SD \
        "s_kernel_load=308000\0" \
        "s_kernel_start=50100000\0" \
        "s_kernel_size=200000\0" \
        "s_rootfs_load=2600000\0" \
        "s_rootfs_start=50300000\0" \
        "s_rootfs_size=d00000\0" \
        "sd_ku=set bootfile $(rootpath)kernel.bin;" \
              "set fl_start $(s_kernel_start);" \
              "set fl_size $(s_kernel_size);" \
              "run s_uploadfile\0" \
        "sd_ru=set bootfile $(rootpath)rootfs.bin;" \
              "set fl_start $(s_rootfs_start);" \
              "set fl_size $(s_rootfs_size);" \
              "run s_uploadfile\0" \
        "sd_boot=cp.b $(s_kernel_start) $(s_kernel_load) $(s_kernel_size);" \
                "cp.b $(s_rootfs_start) $(s_rootfs_load) $(s_rootfs_size);" \
                "bootm $(s_kernel_load) $(s_rootfs_load)\0" \
        "s_uploadfile=$(interface) $(offset);" \
                     "cp.b $(offset) $(fl_start) $(fl_size)\0" \
                     "sd_kernel=cp.b $(s_kernel_start) $(s_kernel_load) $(s_kernel_size);" \
                     "bootm $(s_kernel_load)\0"
#else
#define CFG_STORAGE_RAW_SD
#endif


#define CONFIG_EXTRA_ENV_SETTINGS \
    "uboot_start=40000000\0" \
    "ps_start=40040000\0" \
    "ps_size=c0000\0" \
    "oem_ps_start=40100000\0" \
    "oem_ps_size=100000\0" \
    "kernel_load=0\0" \
    "kernel_start=40200000\0" \
    "kernel_size=180000\0" \
    "rootfs_load=508000\0" \
    "rootfs_start=40380000\0" \
    "rootfs_size=580000\0" \
    "fwu_kernel_offset=0\0" \
    "fwu_kernel_load=5100000\0" \
    "fwu_kernel_start=0\0" \
    "fwu_kernel_size=0\0" \
    "fwu_rootfs_offset=0\0" \
    "fwu_rootfs_load=0\0" \
    "fwu_rootfs_start=0\0" \
    "fwu_rootfs_size=0\0" \
    "fwu_uboot_offset=0\0" \
    "fwu_uboot_size=0\0" \
    "erase_env=ps erase env\0" \
    "erase_ps=set fl_start $(ps_start);" \
             "set fl_size $(ps_size);" \
             "run erase\0" \
    "erase_oem_ps=set fl_start $(oem_ps_start);" \
                 "set fl_size $(oem_ps_size);" \
                 "run erase\0" \
    "erase=protect off $(fl_start) +$(fl_size);" \
          "erase $(fl_start) +$(fl_size);" \
          "protect on $(fl_start) +$(fl_size)\0" \
    CFG_AU \
    CFG_BU \
    CFG_KU \
    CFG_RU \
    CFG_STORAGE_NAND \
    CFG_STORAGE_RAW_SD \
    "spi_au=set bootfile $(rootpath)all.bin;" \
           "set fl_start $(uboot_start);" \
           "run uploadfile\0" \
    "spi_bu=set bootfile $(rootpath)bootloader.bin;" \
           "set fl_start $(uboot_start);" \
           "run uploadfile\0" \
    "spi_ku=set bootfile $(rootpath)kernel.bin;" \
           "set fl_start $(kernel_start);" \
           "run uploadfile\0" \
    "spi_ru=set bootfile $(rootpath)rootfs.squashfs;" \
           "set fl_start $(rootfs_start);" \
           "run uploadfile\0" \
    "fwu_au=$(interface) $(offset) $(rootpath)$(fwuimage);" \
           "fwu update $(offset)\0" \
    "spi_boot=bootm $(kernel_start) $(rootfs_start)\0" \
    "fwu_boot=fwu boot\0" \
    "nfs_boot=run nfsargs;" \
             "run load_kernel\0" \
    "spi_kernel=bootm $(kernel_start)\0" \
    "fwu_kernel=fwu read $(fwu_kernel_load) $(fwu_kernel_start) $(fwu_kernel_size);" \
               "bootm $(fwu_kernel_load)\0" \
    "load_kernel=run spi_kernel\0" \
    "interface=tftp\0" \
    "offset=1000000\0" \
    "uploadfile=$(interface) $(offset);" \
               "protect off $(fl_start) +$(filesize);" \
               "erase $(fl_start) +$(filesize);" \
               "cp.b $(offset) $(fl_start) $(filesize);" \
               "protect on $(fl_start) +$(filesize)\0" \
    "fwudev=\0" \
    "fwuimage=firmimg.d6\0" \
    "console=ttyS0\0" \
    "mem=112M\0" \
    "netdev=eth0\0" \
    "nfsip=\0" \
    "nfsroot=/nfsroot\0" \
    "initrdargs=set bootargs console=$(console) root=$(rootpath);" \
               "bootm $(kernel_start) $(rootfs_start)\0" \
    "hdargs=set bootargs console=$(console) root=/dev/hda1 rw;" \
           "bootm $(kernel_start)\0" \
    "nfsargs=set bootargs " \
                "console=$(console) " \
                "mem=$(mem) " \
                "nfsroot=$(nfsip):$(nfsroot) root=/dev/nfs rw " \
                "ip=$(ipaddr):$(nfsip):$(gatewayip):$(netmask):$(hostname):$(netdev):off\0" \
    "init_mac=0\0"


#endif  /* __CONFIG_H */
