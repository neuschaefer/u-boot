#ifndef __BCM11130_RAY_JFFS2_H
#define __BCM11130_RAY_JFFS2_H

/* We're basically an 11130 RAY with a few changes. */
#include "bcm11130_ray.h"

#define CONFIG_BRCM_DTBLOB_TAG

/* FIXME - The CONFIG_OF_LIBFDT only works with initrd/ramdisk launching */
/* For the logic to work below we need to bring in the basic utilities without
 * changing the way the kernel is launched.
 */
#undef CONFIG_OF_LIBFDT
#define CONFIG_OF_LIBFDT_CMD

#define CONFIG_LOADED_BY_CBOOT

#undef  CONFIG_EXTRA_ENV_SETTINGS
#define CONFIG_EXTRA_ENV_SETTINGS EXTRA_CORE_SETTINGS EXTRA_NET_SETTINGS \
MTD_BOOTCMD MTD_MTDPARTS MTD_BOOTARGS MTD_START_KNL \
MTD_LOAD_VC MTD_START_VC MTD_SET_ROOTDEV GET_DTBLOB PT_TO_DTBLOB MTD_PAGE_SHIFT_BASE10 MTD_BLOCK_SHIFT_BASE10 \
NFS_ROOT_DEFAULTS

#define MTD_PTN_FBI 		"mtdparts add nand0 256k fbi; "		/* 2 copies of fbi in this partition */
#define MTD_PTN_LOADER 		"mtdparts add nand0 256k loader; "
#define MTD_PTN_DTBLOB 		"mtdparts add nand0 256k dt-blob; "
#define MTD_PTN_UBOOT 		"mtdparts add nand0 2m u-boot; "
#define MTD_PTN_ROOT		"mtdparts add nand0 128m root; "
#define MTD_PTN_VC4		"mtdparts add nand0 8m vc4; "
#define MTD_PTN_KERNEL		"mtdparts add nand0 8m kernel; "
#define MTD_PTN_USERDATA	"mtdparts add nand0 64m userdata; "
/* Make sure we have enough space for 4 blocks reserved at the end for uboot-env and redundant copy */

#define MTD_PTN_LIST	\
MTD_PTN_FBI \
MTD_PTN_LOADER \
MTD_PTN_DTBLOB \
MTD_PTN_UBOOT \
MTD_PTN_ROOT \
MTD_PTN_VC4 \
MTD_PTN_KERNEL \
MTD_PTN_USERDATA \

/* NAND Chip configurations - pick one. */

/*
Mode    tRC/tWC
0	100
1	50
2	35
3	30
4	25
5	20
*/

#if 0
// Samsung K9F2G08U0C
#define CONFIG_NAND_ADDR_CYCLES		5	/* 4 or 5 only */
#define CONFIG_NAND_PAGE_SHIFT		11	/* 2 KiB */
#define CONFIG_NAND_BLOCK_SHIFT		17	/* 128 KiB */
#define CONFIG_NAND_BANK_SHIFT 		28	/* 256 MiB */
#define CONFIG_NAND_SECTOR_SIZE 	512	/* ECC data sector */
#define CONFIG_NAND_ECC_T 		8	/* ECC correction */
#define CONFIG_NAND_OOB_SIZE 		64	/* OOB size */
#define CONFIG_NAND_TIMING_SELECT	2	/* Use timing mode for controller */
#define CONFIG_NAND_TIMING_MODE		1	/* 50 nsec */
#endif

#if 0
// Micron MT29F16G08MAA
/* Even with ONFI, chip and block sizes must be known in order
 * to properly configure the environment regions */
#define CONFIG_NAND_BLOCK_SHIFT		19	/* 512 KiB */
#define CONFIG_NAND_BANK_SHIFT 		31	/* 2 GiB */
#define CONFIG_NAND_USE_ONFI			/* Get timing/geometry/ecc from device */
#endif

#if 0
// Hynix H27U8G85DTR
#define CONFIG_NAND_ADDR_CYCLES		5	/* 4 or 5 only */
#define CONFIG_NAND_PAGE_SHIFT		11	/* 2 KiB */
#define CONFIG_NAND_BLOCK_SHIFT		17	/* 128 KiB */
#define CONFIG_NAND_BANK_SHIFT 		30	/* 1 GiB */
#define CONFIG_NAND_SECTOR_SIZE 	512	/* ECC data sector */
#define CONFIG_NAND_ECC_T 		8	/* ECC correction */
#define CONFIG_NAND_OOB_SIZE 		64	/* OOB size */
#define CONFIG_NAND_TIMING_SELECT	2	/* Use timing mode for controller */
#define CONFIG_NAND_TIMING_MODE		4	/* 25 nsec */
#endif

#if 1
// Toshiba TC58NVG1S3ETA0
#define CONFIG_NAND_ADDR_CYCLES		5	/* 4 or 5 only */
#define CONFIG_NAND_PAGE_SHIFT		11	/* 2 KiB */
#define CONFIG_NAND_BLOCK_SHIFT		17	/* 128 KiB */
#define CONFIG_NAND_BANK_SHIFT 		28	/* 256 MiB */
#define CONFIG_NAND_SECTOR_SIZE 	512	/* ECC data sector */
#define CONFIG_NAND_ECC_T 		8	/* ECC correction */
#define CONFIG_NAND_OOB_SIZE 		64	/* OOB size */
#define CONFIG_NAND_TIMING_SELECT	2	/* Use timing mode for controller */
#define CONFIG_NAND_TIMING_MODE		4	/* 25 nsec */
#endif

#define CONFIG_NAND_CHIPSIZE (1<<CONFIG_NAND_BANK_SHIFT)
#define CONFIG_NAND_BLOCKSIZE (1<<CONFIG_NAND_BLOCK_SHIFT)

/* 
 * Place the uboot environment at the start of the last 2 blocks of SLC NAND 
 * Place the redundant uboot environment at the 3rd and 4th last blocks of SLC NAND
 */
#define CONFIG_ENV_RANGE (CONFIG_NAND_BLOCKSIZE)
#undef CONFIG_ENV_OFFSET 
#define CONFIG_ENV_OFFSET 0x180000
#undef CONFIG_ENV_SIZE
#define CONFIG_ENV_SIZE 0x2000 /* less wear on erase block - increase as required */

/* Get dt-blob from nand and place in RAM */

/* 
 * Due to ATAG usage, dt-blob is limited to less than 0x4000 or it will trample
 * the mmu table area. Use 0x3800 to leave room for other atags.
 * You can leave out a size argument for the fdt addr command but then you can't 
 * grow the blob for the new partition table information.
 */
#undef GET_DTBLOB
#define GET_DTBLOB \
"get_dtblob="\
"if testenv mtdparts;"\
"then;"\
"else;"\
	"run mtd_mtdparts;"\
"fi;"\
"mtdparts setenv dt-blob;"\
"setenv maxdtsize 3800;"\
"if testenv brcm_dt_loadaddr;"\
"then;"\
"else;"\
	"malloc brcm_dt_loadaddr ${maxdtsize};"\
"fi;"\
"if fatload mmc ${sddev} ${brcm_dt_loadaddr} dt-blob ${maxdtsize}; then; else;"\
"run setargs; run add${bootfs}; bootn; "\
"fi;"\
"fdt addr ${brcm_dt_loadaddr} ${maxdtsize}\0"

/* When the partition table is created, update the dt-blob if it exists */
/* Only add partitions that the loader needs to know about e.g. uboot, modem/dsp/etc */
#undef PT_TO_DTBLOB
#define PT_TO_DTBLOB \
"pt_to_dtblob="\
"if mtdparts setenv dt-blob;"\
"then;"\
	"run get_dtblob;"\
	"mtdparts enumerate;"\
	"fdt rm /partition_table;"\
	"fdt mknod / partition_table;"\
	"mtdparts setenv u-boot;"\
	"fdt set /partition_table ${mtdparts_name} ${mtdparts_addr} ${mtdparts_size};"\
	"mtdparts setenv dt-blob;"\
"else;"\
	"echo dt-blob partition does not exist, cannot put partition table information into it;"\
"fi\0"

#undef BCM_BOOTCMD
#define BCM_BOOTCMD "bcm_bootcmd=run mtd_bootcmd\0"

/* Export page shift and block shift for external use */
#define MTD_PAGE_SHIFT_BASE10 "mtd_page_shift_base10=" STR(CONFIG_NAND_PAGE_SHIFT) "\0"
#define MTD_BLOCK_SHIFT_BASE10 "mtd_block_shift_base10=" STR(CONFIG_NAND_BLOCK_SHIFT) "\0"

/* Only save the mtd_mtdparts variable once, not every bootup */
#define MTD_BOOTCMD \
"mtd_bootcmd="\
"if testenv mtdparts;"\
"then;"\
"else;"\
	"run mtd_mtdparts;"\
"fi;"\
"run get_dtblob;"\
"if fatload mmc ${sddev} ${initrd_addr} rescue.irfs; then "\
  "setenv initrd_size ${filesize};"\
  "setenv bootfs rescue;"\
"else "\
  "run mtd_set_rootdev;"\
"fi;"\
"run mtd_start_vc;"\
"run setargs; run add${bootfs};"\
"run mtd_start_knl\0"

/* Setup the mtd partitions. Extend this by adding kernel, bootloaders, etc. */
#define MTD_MTDPARTS \
"mtd_mtdparts=setenv mtdids nand0=bcmnand; "\
"mtdparts delall; " MTD_PTN_LIST "; "\
"mtdparts spread;"\
"setenv brcm_dt_loadaddr\0"

/* Extract the root filesystem partiton number to avoid having to hardcode /dev/mmcblk0pN */
#define MTD_SET_ROOTDEV \
"mtd_set_rootdev="\
"mtdparts setenv root;"\
"setenv rootdev /dev/mtdblock${mtdparts_entry}\0"

/* Dynamically figure out the correct mtdblock# for the rootfs. */
#define MTD_BOOTARGS "mtd_bootargs=ip=${ipaddr}:${serverip}:${gatewayip}:::${ethport} root=/dev/nfs rw nfsroot=${serverip}:${rootpath},tcp,nfsvers=3\0"

#define NFS_ROOT_DEFAULTS \
    "ethport=usb0" "\0" \
    "rootpath=/rootfs" "\0" \
    "ipaddr=192.168.0.2" "\0" \
    "gatewayip=192.168.0.1" "\0" \
    "serverip=192.168.0.1" "\0" \
    "bootfs=cramfs" "\0" \
    "addcramfs=set bootargs ${bootargs} root=/dev/mtdblock_robbs1 ro" "\0" \
    "addnfs=set bootargs ${bootargs} ip=${ipaddr}:${serverip}:${gatewayip}:::${ethport} root=/dev/nfs rw nfsroot=${serverip}:${rootpath},tcp,nfsvers=3" "\0" \
    "addrescue=set bootargs console=ttyS0,115200 ${bootargs} initrd=0x${initrd_addr},0x${initrd_size}" "\0" \
    "setargs=set bootargs console=ttyS0,0 ${bootargs}" "\0"

#define MTD_START_KNL \
"mtd_start_knl="\
"math add tmp loadaddr ${knloffs}; "\
"fatload mmc ${sddev} ${tmp} uImage;"\
"bootm ${tmp}\0"

/* Display splash screen by loading videocore firmware and launching it */
#define MTD_LOAD_VC \
"mtd_load_vc="\
"fatload mmc ${sddev} ${vcmem} ${sd_update_prefix}.vc4;\0"

#define MTD_START_VC \
"mtd_start_vc=run mtd_load_vc;"\
"vc bootfs add_dtblob; "\
"vc run ${vcmem}\0"

#define CONFIG_MTD_DEVICE                      1
#define CONFIG_CMD_MTDPARTS_SPREAD             1
#define CONFIG_CMD_NAND                        1
#define CONFIG_CMD_MTDPARTS                    1
#define CONFIG_SYS_MAX_NAND_DEVICE             1
#define CONFIG_SYS_NAND_BASE                   0
#define CONFIG_NAND_DRIVER_DOES_SCAN           1
#define CONFIG_BCMCAPRI_NAND                   1

#define CONFIG_ENV_IS_IN_NAND
#undef CONFIG_ENV_IS_IN_MMC
 
#undef CONFIG_IDENT_STRING
#define CONFIG_IDENT_STRING " - bcm11130_ray_jffs2"

#endif /* __BCM11130_RAY_JFFS2_H */
