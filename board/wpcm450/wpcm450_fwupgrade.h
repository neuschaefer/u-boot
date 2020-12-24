/*
 * $RCSfile$
 * $Revision$
 * $Date$
 * $Author$
 *
 * WPCM450 firmware upgrade driver.
 *  
 * Copyright (C) 2007 Avocent Corp.
 *
 * This file is subject to the terms and conditions of the GNU 
 * General Public License. This program is distributed in the hope 
 * that it will be useful, but WITHOUT ANY WARRANTY; without even 
 * the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
 * PURPOSE.  See the GNU General Public License for more details.
 */


/* 
 flash layout
+---------------------+
| MBR                 | place in the first sector of flash, struct mbr
+---------------------+
| primary partition 1 | each primary partition offset is recorded in the MBR
+---------------------+
| primary partition 2 |
+---------------------+
| primary partition 3 |
+---------------------+
| primary partition 4 |
+---------------------+
                      
 image layout
+---------------------+
| image header        | struct image_header + structimage_info * n
+---------------------+
| image 1             | each image offset is recorded in the image header
+---------------------+
| image 2             |
+---------------------+
|       :             |
+---------------------+
| image n             |
+---------------------+
*/


#pragma pack(1)


#ifdef CONFIG_WPCM450_BOOT_STATUS
typedef struct message  
{
    UINT32 crc32;
    
    /* initial version is 1 */ 
    UINT8 ver; 
    /* 0x01->"initial version" */
    
    /* U-Boot uses only */
    UINT8 boot_image; 
    /* 0x01->"N:1st boot" 
       0x02->"N:2nd boot" 
       0x03->"N:3rd boot" 
       0x11->"N-1:1st boot" 
       0x12->"N-1:2nd boot" 
       0x13->"N-1:3rd boot" */
    
    /* U-Boot to Kernel */
    UINT8 boot_status; 
    /* 0x00->"no status"
       bit0->"boot from N-1 image" - RLG_BOOT_N_IMG_BAD
       bit1->"detect watchdog timeout" - RLG_BOOT_WATCHDOG
       bit2->"user initiated reset" - RLG_BOOT_RESET
       bit3->
       bit4->
       bit5->
       bit6->
       bit7-> */
    
    /* Kernel to U-Boot */
    UINT8 system_status; 
    /* 0x00->"no status" - U-Boot will clear to 0x00 before booting
       0x10->"kernel supports to report status"
       0x20->"root mounted" - Not to use this time
       0x40->"reboot"
       0x80->"all applictaions are running" */
    
    UINT8 reserved[8];
} message_type;
#endif


/* structure of a primary partition, 16 bytes */
typedef struct primary_partition_table
{
    UINT8 status;           /* 0x80 - bootable, active */
    UINT8 start_chs[3];     /* cyliner-head-sector address of the first sector */
    UINT8 type;             /* partition type */
    UINT8 end_chs[3];       /* cyliner-head-sector address of the last sector */
    UINT8 lba[4];           /* logical block address of the first secotr in the partition */
    UINT8 len[4];           /* length of the partition in sectors */
} partition_table_type;


/* structure of a Master Boot Record, MBR, 512 bytes */
typedef struct fwu_mbr
{
    UINT8 code[440];                    /* 0x000 - 0x1B7 */
    UINT8 disk_signature[4];            /* 0x1B8, 0x41 0x56 0x43 0x54 (AVCT) */
    UINT8 reserved[2];                  /* 0x1BC, 0x00 0x00 */
    partition_table_type partition[4];  /* 0x1BE */
    UINT8 mbr_signature[2];             /* 0x1FE, 0x55 0xAA */
} fwu_mbr_type;


/* structure of an Extended Boot Record, EBR, 512 bytes */ 
typedef struct fwu_ebr
{
    UINT8 reserved[446];                /* 0x000, generally unused */
    partition_table_type partition[2];  /* 0x1BE, first and seconed entries */
    partition_table_type res_part[2];   /* 0x1DE */
    UINT8 ebr_signature[2];             /* 0x1FE, 0x55 0xAA */
} fwu_ebr_type;


/* structure of a image information, 12 */
typedef struct fwu_image_info
{
    UINT32 offset;
    UINT32 size;
    UINT32 crc32;
} fwu_image_info_type;


#if 0
/* structure of a image header, 512 bytes */
typedef struct fwu_image_header 
{
    UINT32 crc32;
    UINT16 header_ver;
    UINT16 num_of_image;
    UINT32 version;
    UINT32 img_size;
} fwu_image_header_type;
#endif


/* structure of a image header, 512 bytes */
typedef struct fwu_image_header 
{
    UINT32 crc32;           /* checksum */
    UINT8 header_ver;       /* header version, 1 */
    UINT8 img_type;         /* image type, 1 is for iBMC */
    UINT8 num_of_image;     /* number of image */
    UINT8 reserved_0;
    UINT32 version;         /* use for application */
    UINT32 img_size;        /* total size of image file */
    /* UINT32 reserved_1; */
    /* UINT32 reserved_2; */
    UINT8 uboot_ver[8];     /* 0-2: major 4-6: minor, in BCD */
    /* UINT32 reserved_3; */
    UINT8 platform_id[4];   /* 0: platfrom id, 0xFFFFFFFF -> unspecified */
    UINT32 reserved_4;
} fwu_image_header_type;


#pragma pack()


#ifdef CONFIG_WPCM450_BOOT_STATUS
/* boot status definitions */
#define FWU_BOOT_STATUS_ADDR    0xC6000000

#define FWU_BOOT_N_IMG_1ST      0x01
#define FWU_BOOT_N_IMG_2ND      0x02
#define FWU_BOOT_N_IMG_3RD      0x03
#define FWU_BOOT_N_1_IMG_1ST    0x11
#define FWU_BOOT_N_1_IMG_2ND    0x12
#define FWU_BOOT_N_1_IMG_3RD    0x13

#define FWU_BOOT_NO_STATUS      0x00

#define FWU_BOOT_N_IMG_BAD      0
#define FWU_BOOT_WATCHDOG       1
#define FWU_BOOT_RESET          2

#define FWU_BOOT_KERNEL_SUPPORT 0x10
#define FWU_BOOT_ROOT_MOUNTED   0x20
#define FWU_BOOT_USER_REBOOT    0x40
#define FWU_BOOT_ALL_RUNNING    0x80
#endif


/* partition number for kernel and rootfs */
#define FWU_KERNEL_1            0       /* partition 1 */
#define FWU_ROOTFS_1            1       /* partition 2 */
#define FWU_KERNEL_2            2       /* partition 3 */
#define FWU_EXTENDED            3       /* partition 4 */
#define FWU_ROOTFS_2            4       /* partition 5 */
#define FWU_SCRATCHPAD          5       /* partition 6 */
#define FWU_OEM                 6       /* partition 7 */

/* default partition size for kernel and rootfs */
#define FWU_KERNEL_PART_SIZE        8       /* 8 MB */
#define FWU_ROOTFS_PART_SIZE        56      /* 56 MB */
#define FWU_SCRATCHPAD_PART_SIZE    64      /* 64 MB */
#define FWU_OEM_PART_SIZE           640     /* minimum 640 MB */

/* default partition size for kernel and rootfs in blocks, block = 512 bytes */
#define FWU_KERNEL_PART_BLOCKS      (FWU_KERNEL_PART_SIZE * 1024 * 1024 / 512)
#define FWU_ROOTFS_PART_BLOCKS      (FWU_ROOTFS_PART_SIZE * 1024 * 1024 / 512)
#define FWU_SCRATCHPAD_PART_BLOCKS  (FWU_SCRATCHPAD_PART_SIZE * 1024 * 1024 / 512)
#define FWU_OEM_PART_BLOCKS         (FWU_OEM_PART_SIZE * 1024 * 1024 / 512)

/* default partition count */
#define FWU_PRIMARY_PART_COUNT      3       /* number of primary partition */
#define FWU_EXTENDED_PART_COUNT     1       /* number of extended partition */
#define FWU_LOGICAL_PART_COUNT      3       /* number of logical partition */

/* firmware upgarde device */
#define FWU_DEV_UNKNOWN         0
#define FWU_DEV_EVB             1
#define FWU_DEV_MASER           2
#define FWU_DEV_AMEA            3
#define FWU_DEV_SPI             4
#define FWU_DEV_AMEA_NO_MUX     5
#define FWU_DEV_LAST            5

/* partition type */
#define FWU_PRIMARY_PART        0
#define FWU_EXTEND_PART         1
#define FWU_LOGIC_PART          2

#define FWU_ADD_PART            1
#define FWU_DEL_PART            0

#define FWU_SPI_BOOTLOADER      0
#define FWU_SPI_KERNEL          1
#define FWU_SPI_ROOTFS          2

#define FWU_CRAMFS_MAGIC        0x28cd3d45
#define FWU_CRAMFS_SIGNATURE    "Compressed ROMFS"

#define FWU_SQUASHFS_MAGIC      0x73717368      /* sqsh */
#define FWU_SQUASHFS_SIGNATURE  "SquashFS"

#pragma pack(1)

/* crasfs super block */
typedef struct fwu_cramfs_super {
    unsigned int magic;
    unsigned int size;
    unsigned int flags;
    unsigned int future;
    unsigned char signature[16];
} fwu_cramfs_super_type;

/* typedef long long		squashfs_inode_t; */

/* squashfs super block */
typedef struct squashfs_super_block {
    unsigned int    s_magic;
    unsigned int    inodes;
    unsigned int    bytes_used_2;
    unsigned int    uid_start_2;
    unsigned int    guid_start_2;
    unsigned int    inode_table_start_2;
    unsigned int    directory_table_start_2;
    unsigned int    s_major:16;
    unsigned int    s_minor:16;
    unsigned int    block_size_1:16;
    unsigned int    block_log:16;
    unsigned int    flags:8;
    unsigned int    no_uids:8;
    unsigned int    no_guids:8;
    unsigned int    mkfs_time       /* time of filesystem creation */;
    long long       root_inode;     /* squashfs_inode_t    root_inode; */
    unsigned int    block_size;
    unsigned int    fragments;
    unsigned int    fragment_table_start_2;
    long long       bytes_used;     /* filesystem size in bytes */
    long long       uid_start;
    long long       guid_start;
    long long       inode_table_start;
    long long       directory_table_start;
    long long       fragment_table_start;
    long long       lookup_table_start;
} fwu_squashfs_super_type;

#pragma pack()


/* structure of firmware upgarde information */
typedef struct fwu_info
{
    /* partition */
    UINT32 part_lba[12];    /* partition logical block address, 3P 1E 8L */
    UINT32 part_size[12];   /* partition size */
    UINT32 ebr_lba[8];      /* EBR's logical block address, max 8 */
    UINT32 size;            /* size of device */
    UINT32 free_space;      /* free space of device */
    UINT8 bootable[12];     /* partition status byte, 0x80=bootable */
    UINT8 type[12];         /* partition type */
    UINT8 num_of_pp;        /* number of primary partition */
    UINT8 num_of_ep;        /* number of extended partition */
    UINT8 num_of_lp;        /* number of logical partition */ 
    UINT8 init;             /* init status */
    int dev;                /* device */
} fwu_info_type;
