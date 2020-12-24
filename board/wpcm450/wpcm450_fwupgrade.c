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


#include "common.h"
#include "command.h"
#include "config.h"

#include "cdefs.h"
#include "com_defs.h"

#include "wpcm450_gctrl_regs.h"
#include "wpcm450_fwupgrade.h"
#include "wpcm450_ps.h"
#include "wpcm450_console.h"

#include "image.h"
#include "asm/byteorder.h"


#if 0
#define PRINTD(fmt,args...) printf("FWU: " fmt ,##args)
#else
#define PRINTD(fmt,args...)
#endif

#if 0
#define FWU_DEBUG
#endif


#ifdef CONFIG_MMC
extern unsigned long mmc_berase(int dev, 
                                unsigned long start,    /* start block */
                                lbaint_t blkcnt);

extern unsigned long mmc_bread(int dev, 
                               unsigned long start,     /* start block */
                               lbaint_t blkcnt,
                               unsigned long *buffer);

extern unsigned long mmc_bwrite(int dev, 
                                unsigned long start,    /* start block */
                                lbaint_t blkcnt,
                                unsigned long *buffer);

extern int mmc_get_dev_size(int dev, 
                            UINT32 *size);

extern int mmc_init(int verbose);
#endif


extern int flash_write (char *src, ulong addr, ulong cnt);
extern int flash_sect_erase (ulong addr_first, ulong addr_last);
extern int flash_sect_protect (int p, ulong addr_first, ulong addr_last);


/* firmware upgarde information */
fwu_info_type fwu;


extern int do_bootm (cmd_tbl_t *, int, int, char *[]);


#if 0
static block_dev_desc_t *get_dev (char* ifname, int dev)
{
#if (CONFIG_COMMANDS & CFG_CMD_IDE)
    if (strncmp(ifname, "ide", 3) == 0)
    {
        extern block_dev_desc_t * ide_get_dev(int dev);
        return(ide_get_dev(dev));
    }
#endif
#if (CONFIG_COMMANDS & CFG_CMD_SCSI)
    if (strncmp(ifname, "scsi", 4) == 0)
    {
        extern block_dev_desc_t * scsi_get_dev(int dev);
        return(scsi_get_dev(dev));
    }
#endif
#if ((CONFIG_COMMANDS & CFG_CMD_USB) && defined(CONFIG_USB_STORAGE))
    if (strncmp(ifname, "usb", 3) == 0)
    {
        extern block_dev_desc_t * usb_stor_get_dev(int dev);
        return(usb_stor_get_dev(dev));
    }
#endif
#if defined(CONFIG_MMC)
    if (strncmp(ifname, "mmc", 3) == 0)
    {
        extern block_dev_desc_t *  mmc_get_dev(int dev);
        return(mmc_get_dev(dev));
    }
#endif
#if defined(CONFIG_SYSTEMACE)
    if (strcmp(ifname, "ace") == 0)
    {
        extern block_dev_desc_t *  systemace_get_dev(int dev);
        return(systemace_get_dev(dev));
    }
#endif
    return NULL;
}
#endif


int fwu_block_erase(int dev, 
                    unsigned long start,     /* start block or start address */
                    lbaint_t blkcnt)
{
    UINT32 end;
    
    PRINTD("fwu_block_erase\n");
    
    switch (dev)
    {
#ifdef CONFIG_WPCM450_SVB
        case FWU_DEV_EVB:
#endif
#ifdef CONFIG_WPCM450_WHOVILLE
        case FWU_DEV_MASER:
        case FWU_DEV_AMEA:
        case FWU_DEV_AMEA_NO_MUX:
#endif
            
#ifdef CONFIG_MMC
            if (mmc_berase(dev, start, blkcnt) != blkcnt)
            {
                printf("*** fail to erase block from device ***\n");
                return -1;
            }
#endif
            break;
            
        case FWU_DEV_SPI:
            
            end = (start + blkcnt - 1) | 0xFFFF;
            
            /* un-protect sector */
            flash_sect_protect (0, start, end);
            
            /* erase sector */
            flash_sect_erase(start, end);
            
            /* re-protect sector */
            flash_sect_protect (1, start, end);
            
            break;
            
        default:
            break;
    }
    
    return 0;
}


int fwu_block_read(int dev, 
                   unsigned long start,     /* start block or start address */
                   lbaint_t blkcnt,
                   UINT8 *buffer)
{
    PRINTD("fwu_block_read\n");

    UINT32 u32CLKDIVReg;
    
    switch (dev)
    {
#ifdef CONFIG_WPCM450_SVB
        case FWU_DEV_EVB:
#endif
#ifdef CONFIG_WPCM450_WHOVILLE
        case FWU_DEV_MASER:
        case FWU_DEV_AMEA:
        case FWU_DEV_AMEA_NO_MUX:
#endif
            
#ifdef CONFIG_MMC
            if (mmc_bread(dev, start, blkcnt, (unsigned long *) buffer) != blkcnt)
            {
                printf("*** fail to read block from device ***\n");
                return -1;
            }
#endif
            break;
            
        case FWU_DEV_SPI:
#if 0 
// disable SPI run 50MHz becuase it is over spec.
            u32CLKDIVReg = *((UINT32 *)0xB0000208);
            PRINTD("u32CLKDIVReg : 0x%x\n", u32CLKDIVReg);
            PRINTD("change AHB3CKDIV to equal AHBCLK/2\n");
            *((UINT32 *)0xB0000208) &= 0xFFFFFDFF;
            PRINTD("u32CLKDIVReg : 0x%x\n", *((UINT32 *)0xB0000208));
#endif            
            /* handle odd address */
            while (start & 0x03)
            {
                PRINTD("start address is odd\n");
                
                *(buffer) = *((UINT8 *)start);
                buffer++;
                start++;
                blkcnt--;
            }
            
            /* move 4 bytes once to increase performance */
            while (blkcnt > 3)
            {
                *((UINT32 *) buffer) = *((UINT32 *)start);
                buffer = buffer + 4;
                start = start + 4;
                blkcnt = blkcnt - 4;
            }
            
            /* handle rest of odd address */
            while (blkcnt & 0x03)
            {
                PRINTD("rest of count is odd\n");
                
                *(buffer) = *((UINT8 *)start);
                buffer++;
                start++;
                blkcnt--;
            }
#if 0
// disable SPI run 50MHz becuase it is over spec.
            PRINTD("change AHB3CKDIV back to original value\n");   
            *((UINT32 *)0xB0000208) = u32CLKDIVReg;
            PRINTD("u32CLKDIVReg : 0x%x\n", *((UINT32 *)0xB0000208));
#endif            
            break;
            
        default:
            break;
    }
    
    return 0;
}


int fwu_block_write(int dev, 
                    unsigned long start,     /* start block or start address */
                    lbaint_t blkcnt,
                    UINT8 *buffer)
{
    UINT32 end;
    
    PRINTD("fwu_block_write\n");
    
    switch (dev)
    {
#ifdef CONFIG_WPCM450_SVB
        case FWU_DEV_EVB:
#endif
#ifdef CONFIG_WPCM450_WHOVILLE
        case FWU_DEV_MASER:
        case FWU_DEV_AMEA:
        case FWU_DEV_AMEA_NO_MUX:
#endif
            
#ifdef CONFIG_MMC
            if (mmc_bwrite(dev, start, blkcnt, (unsigned long *) buffer) != blkcnt)
            {
                printf("*** fail to write block to device ***\n");
                return -1;
            }
#endif
            break;
            
        case FWU_DEV_SPI:
            
            end = (start + blkcnt - 1) | 0xFFFF;
            
            /* un-protect sector */
            flash_sect_protect (0, start, end);
            
            /* erase sector */
            flash_sect_erase(start, end);
            
            /* write data to SPI flash */
            flash_write(buffer, start, blkcnt);
            
            /* re-protect sector */
            flash_sect_protect (1, start, end);
            
            break;
            
        default:
            break;
    }
    
    return 0;
}


/* size is in block, 1 block = 512 bytes */
int fwu_get_dev_size(int dev, 
                     UINT32 *size)
{
    switch (dev)
    {
#ifdef CONFIG_WPCM450_SVB
        case FWU_DEV_EVB:
#endif
#ifdef CONFIG_WPCM450_WHOVILLE
        case FWU_DEV_MASER:
        case FWU_DEV_AMEA:
        case FWU_DEV_AMEA_NO_MUX:
#endif
            
#ifdef CONFIG_MMC
            if (mmc_get_dev_size(dev, size))
            {
                printf("*** fail to get device size ***\n");
                return -1;
            }
#endif
            break;
            
        case FWU_DEV_SPI:
            
            *size = 0;
            
            break;
            
        default:
            break;
    }
    
    return 0;
}


int fwu_dev_init(int dev)
{
    switch (dev)
    {
#ifdef CONFIG_WPCM450_SVB
        case FWU_DEV_EVB:
#endif
#ifdef CONFIG_WPCM450_WHOVILLE
        case FWU_DEV_MASER:
        case FWU_DEV_AMEA:
        case FWU_DEV_AMEA_NO_MUX:
#endif

#ifdef CONFIG_MMC
            if (mmc_init(dev))
            {
                printf("*** fail to initiate device ***\n");
                return -1;
            }
#endif
            break;
            
        case FWU_DEV_SPI:
            
            break;
            
        default:
            break;
    }
    
    return 0;
}


int fwu_set_dev_type(int dev)
{
    switch (dev)
    {
#ifdef CONFIG_WPCM450_SVB
        case FWU_DEV_EVB:
            setenv("fwudev", "evb");
            break;
#endif
#ifdef CONFIG_WPCM450_WHOVILLE
        case FWU_DEV_MASER:
            setenv("fwudev", "maser");
            break;
        case FWU_DEV_AMEA:
            setenv("fwudev", "amea");
            break;
        case FWU_DEV_AMEA_NO_MUX:
            setenv("fwudev", "amea_no_mux");
            break;
#endif
        case FWU_DEV_SPI:
            setenv("fwudev", "spi");
            break;
        default:
            printf("not support fwu device\n");
            break;
    }
    
    return 0;
}


int fwu_get_dev_type(int *dev)
{
    char parameter[32];
    
    sprintf(parameter, "%s", getenv("fwudev"));
    
    PRINTD("fwudev=%s\n", getenv("fwudev"));
    
    if (strcmp (parameter, "maser") == 0)
    {
        *dev = FWU_DEV_MASER;
    }
    else if (strcmp(parameter, "amea") == 0)
    {
        *dev = FWU_DEV_AMEA;
    }
    else if (strcmp(parameter, "amea_no_mux") == 0)
    {
        *dev = FWU_DEV_AMEA_NO_MUX;
    }
    else if (strcmp(parameter, "spi") == 0)
    {
        *dev = FWU_DEV_SPI;
    }
    else if (strcmp(parameter, "evb") == 0)
    {
        *dev = FWU_DEV_EVB;
    }
    else
    {
#ifdef CONFIG_WPCM450_SVB
        setenv("fwudev", "evb");
        *dev = FWU_DEV_EVB;
#else
#ifdef CONFIG_WPCM450_WHOVILLE
        
        PRINTD("GP6DIN=%lx, GPI111=%lx\n", GP6DIN, GPI111);
        PRINTD("READ_BIT(GP6DIN, GPI111)=%lx\n", READ_BIT(GP6DIN, GPI111));
        
        /* MASER DC presence, 0 -> present */
        if (READ_BIT(GP6DIN, GPI111))
        {
            /* maser lite */
            setenv("fwudev", "spi");
            *dev = FWU_DEV_SPI;
            
#if 0
            /* AMEA presence, 0x44000012 bit 1 -> 0, CPLD base 0x44000000 */
            if (*((UINT8 *) 0x44000012) & 0x02)
            {
                /* maser lite */
                setenv("fwudev", "spi");
                *dev = FWU_DEV_SPI;
            }
            else
            {
#ifdef CONFIG_WPCM450_WORKAROUND
                /* non muxed amea */
                setenv("fwudev", "amea_no_mux");
                *dev = FWU_DEV_AMEA_NO_MUX;
#else
                /* amea */
                setenv("fwudev", "amea");
                *dev = FWU_DEV_AMEA;
#endif
            }
#endif
        }
        else
        {
            /* maser */
            setenv("fwudev", "maser");
            *dev = FWU_DEV_MASER;
        }
#endif
#endif
    }
    
    PRINTD("fwudev=%s, dev=%d\n", getenv("fwudev"), *dev);
    
    return 0;
}


int fwu_print_dev_list(void)
{
    printf("No.  Device\n");
    printf("---------------------\n");
#ifdef CONFIG_WPCM450_SVB
    printf("%2d   EVB SD/MMC\n", FWU_DEV_EVB);
#endif
#ifdef CONFIG_WPCM450_WHOVILLE
    printf("%2d   MASER\n", FWU_DEV_MASER);
    printf("%2d   AMEA\n", FWU_DEV_AMEA);
#endif
    printf("%2d   SPI\n", FWU_DEV_SPI);
#ifdef CONFIG_WPCM450_WHOVILLE
    printf("%2d   AMEA_NO_MUX\n", FWU_DEV_AMEA_NO_MUX);
#endif
    
    printf("\nCurrent Device ==> %d\n\n", fwu.dev);
    
    return 0;
}


#if 0
UINT32 fwu_cal_sectors(UINT32 bytes)
{
    UINT32 blocks;
    
    if (bytes == 0)
    {
        return 0;
    }
    
    if (bytes & 0xFFFF)
    {
        blocks = ((bytes & 0x0FFFFFFF) >> 16) + 1;
    }
    else
    {
        blocks = (bytes & 0x0FFFFFFF) >> 16;
    }
    
    PRINTD("bytes=%lx, blocks=%lx\n", bytes, blocks);
    
    return blocks;
}
#endif


UINT32 fwu_cal_blocks(UINT32 bytes)
{
    UINT32 blocks;
    
    if (bytes == 0)
    {
        return 0;
    }
    
    if (bytes & 0x1FF)
    {
        blocks = (bytes >> 9) + 1;
    }
    else
    {
        blocks = bytes >> 9;
    }
    
    PRINTD("bytes=%lx, blocks=%lx\n", bytes, blocks);
    
    return blocks;
}


int fwu_get_image_size(int dev, UINT8 partition, UINT32 *size)
{
    UINT8 buffer[512] __attribute__ ((aligned(32)));
    image_header_t *image;
    fwu_cramfs_super_type *cramfs;
    fwu_squashfs_super_type *squashfs;
    UINT32 checksum;
    
    PRINTD("fwu_get_image_size\n");
    
    switch (dev)
    {
#ifdef CONFIG_WPCM450_SVB
        case FWU_DEV_EVB:
#endif
#ifdef CONFIG_WPCM450_WHOVILLE
        case FWU_DEV_MASER:
        case FWU_DEV_AMEA:
        case FWU_DEV_AMEA_NO_MUX:
#endif
            /* check partition number */
            if ((partition == 0) 
                || (partition > (fwu.num_of_pp + fwu.num_of_ep + fwu.num_of_lp)))
            {
                PRINTD("partition number is not correct, %d\n", partition);
                return -1;
            }
            
            PRINTD("fwu.part_lba[%d]=%lx\n", partition, fwu.part_lba[partition]);
            
            if (fwu_block_read(dev, fwu.part_lba[partition - 1], 1, &buffer[0]))
            {
                return -1;
            }
            
            break;
            
        case FWU_DEV_SPI:
            
            switch (partition)
            {
                case FWU_SPI_KERNEL:
                    
                    PRINTD("FWU_SPI_KERNEL\n");
                    
                    memcpy((void *) buffer, 
                           (const void *) simple_strtoul(getenv("kernel_start"),
                                                         NULL, 16), 
                           64);
                    
                    break;
                    
                case FWU_SPI_ROOTFS:
                    
                    PRINTD("FWU_SPI_ROOTFS\n");
                    
                    memcpy((void *) buffer, 
                           (const void *) simple_strtoul(getenv("rootfs_start"),
                                                         NULL, 16), 
                           512);
                    
                    break;
                    
                default:
                    
                    *size = 0;
                    return 0;
            }
            
            break;
            
        default:
            break;
    }
    
    image = (image_header_t *) buffer;
    cramfs = (fwu_cramfs_super_type *) buffer;
    squashfs = (fwu_squashfs_super_type *) buffer;
    
    PRINTD("image=%p, buffer=%p, partition=%d\n", 
           image, buffer, partition);
    
    /* uimage magic number is 0x27051956 */
    if (ntohl(image->ih_magic) == IH_MAGIC)
    {
        PRINTD("uimage magic number is match\n");
        
        checksum = ntohl(image->ih_hcrc);
        image->ih_hcrc = 0;
        
        if (crc32 (0, (UINT8 *) image, sizeof(image_header_t)) != checksum) 
        {
            PRINTD("checksum is %lx not %lx\n", 
                   crc32 (0, (UINT8 *) image, sizeof(image_header_t)), checksum);
            return -1;
        }
        
        /* get the image size from image header */
        *size = ntohl(image->ih_size) + sizeof(image_header_t);
    }
    
    /* cramfs magic number is 0x28cd3d45 */
    else if (cramfs->magic == FWU_CRAMFS_MAGIC)
    {
        PRINTD("cramfs magic number is match\n");
        
        if (strcmp (&cramfs->signature[0], FWU_CRAMFS_SIGNATURE))
        {
            PRINTD("cramfs magic signature is match\n");
            *size = cramfs->size;
        }
        else
        {
            return -1;
        }
    }
    else if (squashfs->s_magic == FWU_SQUASHFS_MAGIC)
    {
        PRINTD("squashfs magic number is match\n");
        PRINTD("squashfs bytes_used is %lx\n", squashfs->bytes_used);
        *size = (UINT32) squashfs->bytes_used;
    }
    else
    {
        PRINTD("uimage magic number is %lx not 0x27051956\n", image->ih_magic);
        PRINTD("cramfs magic number is %lx not 0x28cd3d45\n", cramfs->magic);
        PRINTD("squashfs magic number is %lx not 0x73717368\n", squashfs->s_magic);
        return -1;
    }
    
    PRINTD("image size=%lx\n", *size);
    
    return 0;
}


static unsigned char fwu_bin2bcd(unsigned int n)
{
    return (((n / 10) << 4) | (n % 10));
}


int fwu_print_system(UINT8 type)
{
    switch(type)
    {
        case 0x00:
            printf("    Empty    \n");
            break;
        case 0x05:
            printf("    Extended \n");
            break;
        case 0x06:
            printf("    FAT16    \n");
            break;
        case 0x0B:
            printf("    W95 FAT32\n");
            break;
        case 0x83:
            printf("    Linux    \n");
            break;
        default:
            printf("\n");
            break;
    }
    
    return 0;   
}


int fwu_print_size(UINT32 size)
{
    if (size < (1024 / 512))
    {
        /* (size * 512) */
        printf("%6ld B ", size << 9);
    }
    else if (size < (1024 * 1024 / 512))
    {
        /* (size * 512 / 1024) */
        printf("%6ld KB", (size >> 1));
    }
    else if (size < (1024 * 1024 * 1024 / 512))
    {
        /* (size * 512 / 1024 / 1024) */
        printf("%6ld MB", (size >> 1 >> 10));
    }
    else
    {
        /* (size * 512 / 1024 / 1024 / 1024) */
        printf("%6ld GB", (size >> 1 >> 10 >> 10));
    }
    
    return 0;
}


void fwu_print_type (image_header_t *hdr)
{
    char *os, *arch, *type, *comp;
    
    switch (hdr->ih_os) 
    {
        case IH_OS_INVALID:     os = "Invalid OS";      break;
        case IH_OS_NETBSD:      os = "NetBSD";          break;
        case IH_OS_LINUX:       os = "Linux";           break;
        case IH_OS_VXWORKS:     os = "VxWorks";         break;
        case IH_OS_QNX:         os = "QNX";             break;
        case IH_OS_U_BOOT:      os = "U-Boot";          break;
        case IH_OS_RTEMS:       os = "RTEMS";           break;
#ifdef CONFIG_ARTOS
        case IH_OS_ARTOS:       os = "ARTOS";           break;
#endif
#ifdef CONFIG_LYNXKDI
        case IH_OS_LYNXOS:      os = "LynxOS";          break;
#endif
        default:                os = "Unknown OS";      break;
    }

    switch (hdr->ih_arch) 
    {
        case IH_CPU_INVALID:    arch = "Invalid CPU";           break;
        case IH_CPU_ALPHA:      arch = "Alpha";                 break;
        case IH_CPU_ARM:        arch = "ARM";                   break;
        case IH_CPU_AVR32:      arch = "AVR32";                 break;
        case IH_CPU_I386:       arch = "Intel x86";             break;
        case IH_CPU_IA64:       arch = "IA64";                  break;
        case IH_CPU_MIPS:       arch = "MIPS";                  break;
        case IH_CPU_MIPS64:     arch = "MIPS 64 Bit";           break;
        case IH_CPU_PPC:        arch = "PowerPC";               break;
        case IH_CPU_S390:       arch = "IBM S390";              break;
        case IH_CPU_SH:         arch = "SuperH";                break;
        case IH_CPU_SPARC:      arch = "SPARC";                 break;
        case IH_CPU_SPARC64:    arch = "SPARC 64 Bit";          break;
        case IH_CPU_M68K:       arch = "M68K";                  break;
        case IH_CPU_MICROBLAZE: arch = "Microblaze";            break;
        case IH_CPU_NIOS:       arch = "Nios";                  break;
        case IH_CPU_NIOS2:      arch = "Nios-II";               break;
        default:                arch = "Unknown Architecture";  break;
    }

    switch (hdr->ih_type) 
    {
        case IH_TYPE_INVALID:   type = "Invalid Image";         break;
        case IH_TYPE_STANDALONE:type = "Standalone Program";    break;
        case IH_TYPE_KERNEL:    type = "Kernel Image";          break;
        case IH_TYPE_RAMDISK:   type = "RAMDisk Image";         break;
        case IH_TYPE_MULTI:     type = "Multi-File Image";      break;
        case IH_TYPE_FIRMWARE:  type = "Firmware";              break;
        case IH_TYPE_SCRIPT:    type = "Script";                break;
        case IH_TYPE_FLATDT:    type = "Flat Device Tree";      break;
        default:                type = "Unknown Image";         break;
    }

    switch (hdr->ih_comp) 
    {
        case IH_COMP_NONE:      comp = "uncompressed";          break;
        case IH_COMP_GZIP:      comp = "gzip compressed";       break;
        case IH_COMP_BZIP2:     comp = "bzip2 compressed";      break;
        default:                comp = "unknown compression";   break;
    }

    printf("%s %s %s (%s)", arch, os, type, comp);
}


int fwu_update_env(int dev)
{
    char parameter[128];
    UINT32 size;
    
    switch (dev)
    {
#ifdef CONFIG_WPCM450_SVB
        case FWU_DEV_EVB:
#endif
#ifdef CONFIG_WPCM450_WHOVILLE
        case FWU_DEV_MASER:
        case FWU_DEV_AMEA:
        case FWU_DEV_AMEA_NO_MUX:
#endif
            
            /* check if the 7th bit of status bit is set */
            if (fwu.bootable[FWU_KERNEL_1] & 0x80)
            {
                /* setup bootargs from mem and console variables */
                sprintf(parameter, 
                        "root=/dev/mmcblk0p2 mem=%s console=%s quiet", 
                        getenv("mem"),
                        getenv("console"));
                setenv("bootargs", parameter);
                
                /* setenv("bootargs", "root=/dev/mmcblk0p2 mem=112M console=ttyS0"); */
                /* setenv("bootdelay", "1"); */
                
                /* update the kernel start address */
                sprintf(parameter, "%lx", fwu.part_lba[FWU_KERNEL_1]);
                setenv("fwu_kernel_start", parameter);
                
                /* update the rootfs start address */
                sprintf(parameter, "%lx", fwu.part_lba[FWU_ROOTFS_1]);
                setenv("fwu_rootfs_start", parameter);
                
                /* update the kernel size from the image at partition 1 */
                if (fwu_get_image_size(dev, 1, &size))
                {
                    /* use partition size as default */
                    /* sprintf(parameter, "%lx", fwu.part_size[FWU_KERNEL_1]); */
                    /* setenv("fwu_kernel_size", parameter); */
                    
                    /* set kernel size to 0 if the image type can't be recognized */
                    setenv("fwu_kernel_size", "0");
                }
                else
                {
                    /* move to next boundary and divide by 512 */
                    size = fwu_cal_blocks(size);
                    
                    /* get exact size from image */
                    sprintf(parameter, "%lx", size);
                    setenv("fwu_kernel_size", parameter);
                }
                
                /* update the rootfs size from the image at partition 2*/
                if (fwu_get_image_size(dev, 2, &size))
                {
                    /* use partition size as default */
                    /* sprintf(parameter, "%lx", fwu.part_size[FWU_ROOTFS_1]); */
                    /* setenv("fwu_rootfs_size", parameter); */
                    
                    /* set rootfs size to 0 if the image type can't be recognized */
                    setenv("fwu_rootfs_size", "0");
                }
                else
                {
                    /* move to next boundary and divide by 512 */
                    size = fwu_cal_blocks(size);
                    
                    /* get exact size from image */
                    sprintf(parameter, "%lx", size);
                    setenv("fwu_rootfs_size", parameter);
                }
            }
            else if (fwu.bootable[FWU_KERNEL_2] & 0x80)
            {
                /* setup bootargs from mem and console variables */
                sprintf(parameter, 
                        "root=/dev/mmcblk0p5 mem=%s console=%s quiet", 
                        getenv("mem"),
                        getenv("console"));
                setenv("bootargs", parameter);
                
                /* setenv("bootargs", "root=/dev/mmcblk0p5 mem=112M console=ttyS0"); */
                /* setenv("bootdelay", "1"); */
                
                /* update the kernel start address */
                sprintf(parameter, "%lx", fwu.part_lba[FWU_KERNEL_2]);
                setenv("fwu_kernel_start", parameter);
                
                /* update the rootfs start address */
                sprintf(parameter, "%lx", fwu.part_lba[FWU_ROOTFS_2]);
                setenv("fwu_rootfs_start", parameter);
                
                /* update the kernel size from the image at partition 3 */
                if (fwu_get_image_size(dev, 3, &size))
                {
                    /* use partition size as default */
                    /* sprintf(parameter, "%lx", fwu.part_size[FWU_KERNEL_2]); */
                    /* setenv("fwu_kernel_size", parameter); */
                    
                    /* set kernel size to 0 if the image type can't be recognized */
                    setenv("fwu_kernel_size", "0");
                }
                else
                {
                    /* move to next boundary and divide by 512 */
                    size = fwu_cal_blocks(size);
                    
                    /* get exact size from image */
                    sprintf(parameter, "%lx", size);
                    setenv("fwu_kernel_size", parameter);
                }
                
                /* update the rootfs size from the image at partition 5*/
                if (fwu_get_image_size(dev, 5, &size))
                {
                    /* use partition size as default */
                    /* sprintf(parameter, "%lx", fwu.part_size[FWU_ROOTFS_2]); */
                    /* setenv("fwu_rootfs_size", parameter); */
                    
                    /* set rootfs size to 0 if the image type can't be recognized */
                    setenv("fwu_rootfs_size", "0");
                }
                else
                {
                    /* move to next boundary and divide by 512 */
                    size = fwu_cal_blocks(size);
                    
                    /* get exact size from image */
                    sprintf(parameter, "%lx", size);
                    setenv("fwu_rootfs_size", parameter);
                }
            }
            else
            {
                /* reset bootargs to default */
                /* setenv("bootargs", CONFIG_BOOTARGS"\0"); */
                
                /* force to stay in U-boot if there is no active partition */
                /* setenv("bootdelay", "-1"); */
                
                /* reset variables */
                setenv("fwu_kernel_start", "0");
                setenv("fwu_kernel_size", "0");
                setenv("fwu_rootfs_start", "0");
                setenv("fwu_rootfs_size", "0");
            }
            
            break;
            
        case FWU_DEV_SPI:
            
            /* setup bootargs from mem and console variables */
            sprintf(parameter, 
                    "root=/dev/ram0 rw initrd=0x508000,0xC00000 " \
                    "ramdisk_size=65536 " \
                    "mem=%s console=%s quiet", 
                    getenv("mem"),
                    getenv("console"));
            setenv("bootargs", parameter);
            
            /* update fwu environment variables */
            
            /* get kernel start address */
            setenv("fwu_kernel_start", getenv("kernel_start"));
            
            /* update kernel size from the image */
            if (fwu_get_image_size(dev, FWU_SPI_KERNEL, &size))
            {
                PRINTD("default kernel size\n");
                /* setenv("fwu_kernel_size", getenv("kernel_size")); */
                
                /* set kernel size to 0 if the image type can't be recognized */
                setenv("fwu_kernel_size", "0");
            }
            else
            {
                /* move to next boundary and divide by 64K */
                /* size = fwu_cal_sectors(size); */
                
                PRINTD("kernel size=%lx\n", size);
                sprintf(parameter, "%lx", size);
                setenv("fwu_kernel_size", parameter);
            }
            
            /* get rootfs start address */
            setenv("fwu_rootfs_start", getenv("rootfs_start"));
            
            /* update rootfs size from the image */
            if (fwu_get_image_size(dev, FWU_SPI_ROOTFS, &size))
            {
                PRINTD("default rootfs size\n");
                /* setenv("fwu_rootfs_size", getenv("rootfs_size")); */
                
                /* set rootfs size to 0 if the image type can't be recognized */
                setenv("fwu_rootfs_size", "0");
            }
            else
            {
                /* move to next boundary and divide by 64K */
                /* size = fwu_cal_sectors(size); */
                
                PRINTD("rootfs size=%lx\n", size);
                sprintf(parameter, "%lx", size);
                setenv("fwu_rootfs_size", parameter);
            }
            
            break;
            
        default:
            break;
    }
    
    return 0;
}


int fwu_list_partition(int dev)
{
    UINT8 buffer[512] __attribute__ ((aligned(32)));
    image_header_t *image;
    fwu_cramfs_super_type *cramfs;
    fwu_squashfs_super_type *squashfs;
    int i;
    UINT32 checksum;
    
    printf("No.  Type  Status    Start      Size             System\n");
    printf("-----------------------------------------------------------\n");
    
    /* print information for primary partition */
    for (i = 0; i < fwu.num_of_pp; i++)
    {
        /* No. */
        printf("%2d   ", (i + 1));
        
        /* Type */
        printf("  P     ");
        
        /* Status */
        printf("%02x ", fwu.bootable[i]);
        
        /* Start */
        printf("%10lx", fwu.part_lba[i]);
        
        /* Size */
        printf("%10lx", fwu.part_size[i]);
        
        /* human size */
        fwu_print_size(fwu.part_size[i]);
        
        /* system */
        fwu_print_system(fwu.type[i]);
    }
    
    /* print information for extended partition */
    if (fwu.num_of_ep)
    {
        /* No. */
        printf("%2d   ", (fwu.num_of_pp + 1));
        
        /* Type */
        printf("  E     ");
        
        /* Status */
        printf("%02x ", fwu.bootable[fwu.num_of_pp]);
        
        /* Start */
        printf("%10lx", fwu.part_lba[fwu.num_of_pp]);
        
        /* Size */
        printf("%10lx", fwu.part_size[fwu.num_of_pp]);
        
        /* human size */
        fwu_print_size(fwu.part_size[fwu.num_of_pp]);
        
        /* system */
        fwu_print_system(fwu.type[fwu.num_of_pp]);
    }
    
    /* print information for logical partition */
    for (i = 0; i < fwu.num_of_lp; i++)
    {
        /* No. */
        printf("%2d   ", (fwu.num_of_pp + fwu.num_of_ep + i + 1));
        
        /* Type */
        printf("  L     ");
        
        /* Status */
        printf("%02x ", fwu.bootable[fwu.num_of_pp + fwu.num_of_ep + i]);
        
        /* Start */
        printf("%10lx", fwu.part_lba[fwu.num_of_pp + fwu.num_of_ep + i]);
        
        /* Size */
        printf("%10lx", fwu.part_size[fwu.num_of_pp + fwu.num_of_ep + i]);
        
        /* human size */
        fwu_print_size(fwu.part_size[fwu.num_of_pp + fwu.num_of_ep + i]);
        
        /* system */
        fwu_print_system(fwu.type[fwu.num_of_pp + fwu.num_of_ep + i]);
    }
    
    for (i = 0; i < (fwu.num_of_pp + fwu.num_of_ep + fwu.num_of_lp); i++)
    {
        /* No. */
        printf("\n%2d     ", (i + 1));
        
        if (fwu_block_read(dev, fwu.part_lba[i], 1, &buffer[0]))
        {
            return -1;
        }
        
        image = (image_header_t *) buffer;
        cramfs = (fwu_cramfs_super_type *) buffer;
        squashfs = (fwu_squashfs_super_type *) buffer;
        
        /* uimage magic number is 0x27051956 */
        if (ntohl(image->ih_magic) == IH_MAGIC)
        {
            checksum = ntohl(image->ih_hcrc);
            image->ih_hcrc = 0;
            
            if (crc32 (0, (UINT8 *) image, sizeof(image_header_t)) != checksum) 
            {
                printf("Unknown Image\n");
                continue;
            }
            
            printf("Name: %.*s", IH_NMLEN, image->ih_name);
            printf("\n       Type: "); 
            fwu_print_type(image);
            printf("\n       Data Size: 0x%lx bytes (0x%lx blocks)",
                   ntohl(image->ih_size),
                   fwu_cal_blocks(ntohl(image->ih_size)));
            printf("\n       Load Address: %08x"
                   "\n       Entry Point:  %08x\n",
                   ntohl(image->ih_load), 
                   ntohl(image->ih_ep));
        }
        
        /* cramfs magic number is 0x28cd3d45 */
        else if (cramfs->magic == FWU_CRAMFS_MAGIC)
        {
            PRINTD("cramfs magic number is match\n");
            
            if (strcmp (&cramfs->signature[0], FWU_CRAMFS_SIGNATURE))
            {
                PRINTD("cramfs magic signature is match\n");
                
                printf("Type: %s", FWU_CRAMFS_SIGNATURE); 
                printf("\n       Data Size: 0x%lx bytes (0x%lx blocks)\n", 
                       cramfs->size,
                       fwu_cal_blocks(cramfs->size));
            }
            else
            {
                printf("Unknown Image\n");
                continue;
            }
        }
        
        /* squashfs magic number is 0x73717368 */
        else if (squashfs->s_magic == FWU_SQUASHFS_MAGIC)
        {
            PRINTD("squashfs magic number is match\n");
            
            printf("Type: %s", FWU_SQUASHFS_SIGNATURE); 
            printf("\n       Data Size: 0x%lx bytes (0x%lx blocks)\n", 
                   (UINT32) squashfs->bytes_used,
                   fwu_cal_blocks((UINT32) squashfs->bytes_used));
        }
        
        /* unsupported image */
        else
        {
            printf("Unknown Image\n");
            continue;
        }
        

    }
    
    printf("\n");
    
#ifdef FWU_DEBUG
    printf("\n\n");
    
    for (i = 0; i < 12; i++)
    {
        /* No. */
        printf("%2d     ", (i + 1));
        
        /* Type */
        switch (i)
        {
            case 0:
            case 1:
            case 2:
                printf("P     ");
                break;
            case 3:
                printf("E     ");
                break;
            default:
                printf("L     ");
                break;
        }
        
        /* Status */
        printf("%02x ", fwu.bootable[i]);
        
        /* Start */
        printf("%10lx", fwu.part_lba[i]);
        
        /* Size */
        printf("%10lx", fwu.part_size[i]);
        
        if (fwu.part_size[i] < (1024 / 512))
        {
            /* (size * 512) */
            printf("%6ld B\n", fwu.part_size[i] << 9);
        }
        else if (fwu.part_size[i] < (1048576 / 512))
        {
            /* (size * 512 / 1024) */
            printf("%6ld KB\n", (fwu.part_size[i] >> 1));
        }
        else
        {
            /* (size * 512 / 1024 / 1024) */
            printf("%6ld MB\n", (fwu.part_size[i] >> 1 >> 10));
        }
    }
#endif
    
    return 0;
}


int fwu_dump_block(int dev, UINT32 start, UINT32 blocks)
{
    UINT8 buffer[512] __attribute__ ((aligned(32)));
    UINT32 i;
    
    if (blocks == 0)
    {
        return -1;
    }
    
    while (blocks)
    {
        if (fwu_block_read(dev, start, 1, &buffer[0]))
        {
            return -1;
        }
        
        printf("%08lx : ", start << 9);
        printf("00 01 02 03 04 05 06 07 08 09 0a 0b 0c 0d 0e 0f\n");
        printf("----------------------------------------------------------");
        
        for (i = 0; i < 512; i++)
        {
            if ((i % 16) == 0)
            {
                printf("\n%08x : ", (i + (start << 9)));
            }
            
            printf("%02x ", buffer[i]);
        }
        
        printf("\n\n");
        
        blocks--;
        start++;
    }
    
    return 0;
}


int fwu_activate_partition(int dev, UINT8 partition, UINT8 activate)
{
    UINT8 buffer[512] __attribute__ ((aligned(32)));
    fwu_mbr_type *mbr; 
    
    /* check if partition is less than 4 */
    if ((partition > 4) || (partition == 0))
    {
        printf("*** partition %d can not set as primary parition ***\n", 
               partition);
        return -1;
    }
    
    /* read MBR to buffer */
    if (fwu_block_read(dev, 0, 1, &buffer[0]))
    {
        printf("*** fail to read MBR ***\n");
        return -1;
    }
    
    mbr = (fwu_mbr_type *) buffer;
    
    PRINTD("bf status=%lx, partition=%d\n", 
           mbr->partition[partition - 1].status, 
           partition);
    
    /* change the active bit in MBR */
    if (activate)
    {
        /* reset all active bits, only one partition can be set as active */
        mbr->partition[0].status = 0x00;
        mbr->partition[1].status = 0x00;
        mbr->partition[2].status = 0x00;
        mbr->partition[3].status = 0x00;
        
        PRINTD("set the active bit\n");
        
        /* set the active bit for selected partition */
        mbr->partition[partition - 1].status = 0x80;
    }
    else
    {
        PRINTD("clear the active bit\n");
        mbr->partition[partition - 1].status = 0x00;
    }
    
    PRINTD("af status=%lx\n", mbr->partition[partition - 1].status);
    
    /* update MBR */
    if(fwu_block_write(dev, 0, 1, &buffer[0]))
    {
        printf("*** fail to write MBR ***\n");
        return -1;
    }
    
    /* update internal structure */
    fwu.bootable[0] = mbr->partition[0].status;
    fwu.bootable[1] = mbr->partition[1].status;
    fwu.bootable[2] = mbr->partition[2].status;
    fwu.bootable[3] = mbr->partition[3].status;
    
    /* update environment */
    fwu_update_env(dev);
    
    return 0;
}


int fwu_swap_act_partition(int dev)
{
    int ret;
    
    /* swap active partition */
    if (fwu.bootable[0] & 0x80)
    {
        /* set partition 3 as active */
        ret = fwu_activate_partition(dev, 3, 1);
    }
    else
    {
        /* set partition 1 as active */
        ret = fwu_activate_partition(dev, 1, 1);
    }
    
    if (ret) 
    {
        printf("*** fail to swap active partition ***\n");
        return ret;
    }
    
    return 0;
}


/* calculate c-h-s from lba */
int fwu_cal_chs(UINT32 lba, UINT8 *chs)
{
    UINT16 c;               /* cylinder, 10 bits*/
    UINT8 h;                /* head, 8 bits */
    UINT8 s;                /* sector, 6 bits */
    UINT16 num_of_c = 1024; /* 0 base */
    UINT8 num_of_h = 255;   /* 0 base */
    UINT8 num_of_s = 63;    /* 1 base */
    
    if (lba >= (num_of_c * num_of_h * num_of_s))
    {
        PRINTD("lba >= (num_of_c * num_of_h * num_of_s)\n");
        c = num_of_c - 1;
        h = num_of_h - 1;
        s = num_of_s;
    }
    else
    {
        c = lba / (num_of_h * num_of_s);
        h = (lba % (num_of_h * num_of_s)) / num_of_s;
        s = (lba % (num_of_h * num_of_s)) % num_of_s + 1;
    }
    
    /* *chs, h-cs-c, cs: bit 9-8 of c, c: bit 7-0 of c */
    *chs = h;
    *(chs + 1) = (UINT8) (((c & 0x0300) >> 2) | s);
    *(chs + 2) = (UINT8) (c & 0x00FF);
    
    PRINTD("lba=%lx, c=%lx, h=%lx, s=%lx\n", lba, c, h, s);
    
    return 0;   
}


int fwu_gen_mbr(UINT8 is_add, 
                UINT8 part_type, 
                UINT32 size, 
                UINT8 fs_type, 
                UINT8 *buffer)
{
    UINT32 blocks;
    UINT32 first_lba;
    UINT32 last_lba;
    fwu_mbr_type *mbr;
    
    mbr = (fwu_mbr_type *) buffer;
        
    if (is_add == FWU_ADD_PART)
    {
        if ((fwu.num_of_pp < 4) && (fwu.num_of_ep == 0))
        {
            if (fwu.num_of_pp == 0)
            {
                /* mbr used one block */
                first_lba = 1;
            }
            else
            {
                /* calculate current start lba from last primary partition */
                first_lba = fwu.part_lba[fwu.num_of_pp - 1] 
                            + fwu.part_size[fwu.num_of_pp - 1];
            }
            
            /* 0x00 == rest size of device */
            if (size != 0)
            {
                /* size in MBs */
                blocks = size * 1024 * 1024 / 512;
            }
            else
            {
                /* get rest size of device */
                blocks = fwu.size - first_lba;
            }
            
            /* calculate the last block */
            last_lba = first_lba + blocks - 1;
            
            PRINTD("first_lba=%lx, last_lba=%lx, blocks=%lx\n", 
                   first_lba, last_lba, blocks);
            
            if (fwu.num_of_pp == 0)
            {
                mbr->disk_signature[0] = 0x41;
                mbr->disk_signature[1] = 0x56;
                mbr->disk_signature[2] = 0x43;
                mbr->disk_signature[3] = 0x54;
                
                /* mbr signature */
                mbr->mbr_signature[0] = 0x55;
                mbr->mbr_signature[1] = 0xAA;
            }
            
            /* offset 0x00 */
            mbr->partition[fwu.num_of_pp].status = 0;
            
            /* offset 0x01 */
            fwu_cal_chs(first_lba, &mbr->partition[fwu.num_of_pp].start_chs[0]);
            
            PRINTD("start_chs[0]=%lx %lx %lx\n", 
                   mbr->partition[fwu.num_of_pp].start_chs[0],
                   mbr->partition[fwu.num_of_pp].start_chs[1],
                   mbr->partition[fwu.num_of_pp].start_chs[2]);
            
            /* offset 0x04 */
            if (part_type == FWU_EXTEND_PART)
            {
                mbr->partition[fwu.num_of_pp].type = 0x05;
            }
            else
            {
                mbr->partition[fwu.num_of_pp].type = fs_type;
            }
            
            /* offset 0x05 */
            fwu_cal_chs(last_lba, &mbr->partition[fwu.num_of_pp].end_chs[0]);
            
            PRINTD("end_chs[0]=%lx %lx %lx\n", 
                   mbr->partition[fwu.num_of_pp].end_chs[0],
                   mbr->partition[fwu.num_of_pp].end_chs[1],
                   mbr->partition[fwu.num_of_pp].end_chs[2]);
            
            /* offset 0x08 */
            mbr->partition[fwu.num_of_pp].lba[3] = (UINT8) (first_lba >> 24); 
            mbr->partition[fwu.num_of_pp].lba[2] = (UINT8) ((first_lba & 0x00FF0000) >> 16);
            mbr->partition[fwu.num_of_pp].lba[1] = (UINT8) ((first_lba & 0x0000FF00) >> 8);
            mbr->partition[fwu.num_of_pp].lba[0] = (UINT8) (first_lba & 0x000000FF);
            
            /* offset 0x0C */
            mbr->partition[fwu.num_of_pp].len[3] = (UINT8) (blocks >> 24);
            mbr->partition[fwu.num_of_pp].len[2] = (UINT8) ((blocks & 0x00FF0000) >> 16);
            mbr->partition[fwu.num_of_pp].len[1] = (UINT8) ((blocks & 0x0000FF00) >> 8);
            mbr->partition[fwu.num_of_pp].len[0] = (UINT8) (blocks & 0x000000FF);
            
            /* update internal variable */
            fwu.part_lba[fwu.num_of_pp] = first_lba;
            fwu.part_size[fwu.num_of_pp] = blocks;
            fwu.type[fwu.num_of_pp] = fs_type;
            
           if (part_type == FWU_EXTEND_PART)
            {
                fwu.num_of_ep++;
            }
            else
            {
                fwu.num_of_pp++;
            }
            
            PRINTD("pp=%d, ep=%d, lp=%d\n", 
                   fwu.num_of_pp, fwu.num_of_ep, fwu.num_of_lp);
        }
        else
        {
            PRINTD("*** can't add primary partition ***\n");
            return -1;
        }
    }
    else
    {
        if (part_type == FWU_PRIMARY_PART)
        {
            if ((fwu.num_of_pp != 0) && (fwu.num_of_ep == 0))
            {
                /* decrease the count of primary partition */
                fwu.num_of_pp--;
                
                /* offset 0x00 */
                mbr->partition[fwu.num_of_pp].status = 0;
                
                /* offset 0x01 */
                mbr->partition[fwu.num_of_pp].start_chs[0] = 0;
                mbr->partition[fwu.num_of_pp].start_chs[1] = 0;
                mbr->partition[fwu.num_of_pp].start_chs[2] = 0;
                
                /* offset 0x04 */
                mbr->partition[fwu.num_of_pp].type = 0;
                
                /* offset 0x05 */
                mbr->partition[fwu.num_of_pp].end_chs[0] = 0;
                mbr->partition[fwu.num_of_pp].end_chs[1] = 0;
                mbr->partition[fwu.num_of_pp].end_chs[2] = 0;
                
                /* offset 0x08 */
                mbr->partition[fwu.num_of_pp].lba[3] = 0; 
                mbr->partition[fwu.num_of_pp].lba[2] = 0;
                mbr->partition[fwu.num_of_pp].lba[1] = 0;
                mbr->partition[fwu.num_of_pp].lba[0] = 0;
                
                /* offset 0x0C */
                mbr->partition[fwu.num_of_pp].len[3] = 0;
                mbr->partition[fwu.num_of_pp].len[2] = 0;
                mbr->partition[fwu.num_of_pp].len[1] = 0;
                mbr->partition[fwu.num_of_pp].len[0] = 0;
                
                /* update internal variable */
                
                fwu.part_lba[fwu.num_of_pp] = 0;
                fwu.part_size[fwu.num_of_pp] = 0;
            }
            
            else
            {
                PRINTD("*** can't remove primary partition ***\n");
                return -1;
            }
        }
        else if (part_type == FWU_EXTEND_PART)
        {
            if (fwu.num_of_ep != 0)
            {
                /* offset 0x00 */
                mbr->partition[fwu.num_of_pp].status = 0;
                
                /* offset 0x01 */
                mbr->partition[fwu.num_of_pp].start_chs[0] = 0;
                mbr->partition[fwu.num_of_pp].start_chs[1] = 0;
                mbr->partition[fwu.num_of_pp].start_chs[2] = 0;
                
                /* offset 0x04 */
                mbr->partition[fwu.num_of_pp].type = 0;
                
                /* offset 0x05 */
                mbr->partition[fwu.num_of_pp].end_chs[0] = 0;
                mbr->partition[fwu.num_of_pp].end_chs[1] = 0;
                mbr->partition[fwu.num_of_pp].end_chs[2] = 0;
                
                /* offset 0x08 */
                mbr->partition[fwu.num_of_pp].lba[3] = 0; 
                mbr->partition[fwu.num_of_pp].lba[2] = 0;
                mbr->partition[fwu.num_of_pp].lba[1] = 0;
                mbr->partition[fwu.num_of_pp].lba[0] = 0;
                
                /* offset 0x0C */
                mbr->partition[fwu.num_of_pp].len[3] = 0;
                mbr->partition[fwu.num_of_pp].len[2] = 0;
                mbr->partition[fwu.num_of_pp].len[1] = 0;
                mbr->partition[fwu.num_of_pp].len[0] = 0;
                
                /* update internal variable */
                fwu.part_lba[fwu.num_of_pp] = 0;
                fwu.part_size[fwu.num_of_pp] = 0;
                fwu.num_of_ep--;
            }
            else
            {
                PRINTD("*** can't remove extended partition ***\n");
                return -1;
            }
        }
        else
        {
            PRINTD("*** can't remove logical partition ***\n");
            return -1;
        }
    }
    
    return 0;   
}


int fwu_gen_ebr(int dev, 
                UINT8 is_add, 
                UINT8 part_type, 
                UINT32 size, 
                UINT8 fs_type, 
                UINT8 *buffer)
{
    fwu_ebr_type *ebr;
    UINT32 lp_start;
    UINT32 lp_size;
    UINT32 next_ebr_start;
    UINT32 next_ebr_size;
    
#ifdef FWU_DEBUG
    UINT16 i;
#endif
    
    ebr = (fwu_ebr_type *) buffer;
    
    PRINTD("fwu.num_of_lp=%d\n", fwu.num_of_lp);
    
    if (part_type == FWU_LOGIC_PART)
    {
        if (is_add == FWU_ADD_PART)
        {
            /* limit the number of logical partition */
            if (fwu.num_of_lp >= 8)
            {
                printf("*** exceed the maximum capability of logical partition ***\n");
                return -1;
            }
            
            /* prepare ebr information */
            lp_start = 1;
            
            if (size != 0)
            {
                lp_size = (size * 1024 * 1024 / 512);
            }
            else
            {
                if (fwu.num_of_lp == 0)
                {
                    /* 1 -> size of the first ebr */
                    lp_size = fwu.part_size[fwu.num_of_pp] - 1;
                }
                else
                {
                    lp_size = fwu.part_size[fwu.num_of_pp] 
                              - (fwu.ebr_lba[fwu.num_of_lp - 1] - fwu.ebr_lba[0])
                              - fwu.part_size[fwu.num_of_pp + fwu.num_of_ep 
                                              + fwu.num_of_lp - 1]
                              - 1 
                              - 1;
                    
                    PRINTD("lp_size = %lx - %lx - %lx - 1 -1 = %lx\n", 
                            fwu.part_size[fwu.num_of_pp],
                            (fwu.ebr_lba[fwu.num_of_lp - 1] - fwu.ebr_lba[0]),
                            fwu.part_size[fwu.num_of_pp 
                                          + fwu.num_of_ep 
                                          + fwu.num_of_lp 
                                          - 1],
                            lp_size);
                }
            }
            
            if (fwu.num_of_lp == 0)
            {
                next_ebr_start = 0;
            }
            else
            {
                next_ebr_start = fwu.ebr_lba[fwu.num_of_lp - 1] 
                                 - fwu.ebr_lba[0] 
                                 + fwu.part_size[fwu.num_of_pp + fwu.num_of_ep 
                                                 + fwu.num_of_lp - 1] 
                                 + 1;
                
                PRINTD("next_ebr_start = %lx - %lx + %lx + 1 = %lx\n",
                        fwu.ebr_lba[fwu.num_of_lp - 1],
                        fwu.ebr_lba[0],
                        fwu.part_size[fwu.num_of_pp 
                                      + fwu.num_of_ep 
                                      + fwu.num_of_lp 
                                      - 1],
                        next_ebr_start);
                
            }
            
            next_ebr_size = lp_start + lp_size;
            
            PRINTD("lp_start=%lx, lp_size=%lx, next_ebr_start=%lx, next_ebr_size=%lx\n", 
                   lp_start, lp_size, next_ebr_start, next_ebr_size);
            
            /* if there is not only one logical partition */
            if (fwu.num_of_lp > 0)
            {
                /* read ebr */
                if (fwu_block_read(dev, fwu.ebr_lba[fwu.num_of_lp - 1], 1, 
                                   &buffer[0]))
                {
                    printf("*** can't read from device %d ***\n", dev);
                    return -1;
                }
                
                /* set start CHS */
                fwu_cal_chs(next_ebr_start + fwu.part_lba[fwu.num_of_pp], 
                            &ebr->partition[1].start_chs[0]);
                
                /* set end CHS */
                fwu_cal_chs(next_ebr_start + next_ebr_size - 1 
                            + fwu.part_lba[fwu.num_of_pp], 
                            &ebr->partition[1].end_chs[0]);
                
                /* set lba */
                ebr->partition[1].lba[3] = (UINT8) (next_ebr_start >> 24); 
                ebr->partition[1].lba[2] = (UINT8) ((next_ebr_start & 0x00FF0000) >> 16);
                ebr->partition[1].lba[1] = (UINT8) ((next_ebr_start & 0x0000FF00) >> 8);
                ebr->partition[1].lba[0] = (UINT8) (next_ebr_start & 0x000000FF);
                
                /* set size */
                ebr->partition[1].len[3] = (UINT8) (next_ebr_size >> 24); 
                ebr->partition[1].len[2] = (UINT8) ((next_ebr_size & 0x00FF0000) >> 16);
                ebr->partition[1].len[1] = (UINT8) ((next_ebr_size & 0x0000FF00) >> 8);
                ebr->partition[1].len[0] = (UINT8) (next_ebr_size & 0x000000FF); 
                
                /* set type, extended */
                ebr->partition[1].type = 0x05;
                
#ifdef FWU_DEBUG
                for (i = 0; i < 512; i++)
                {
                    if ((i % 16) == 0)
                    {
                        printf("\n%08x : ", i);
                    }
                    
                    printf("%02x ", buffer[i]);
                }
                printf("\n");
#endif
                
                /* update ebr */
                if (fwu_block_write(dev, 
                                    fwu.ebr_lba[fwu.num_of_lp - 1], 
                                    1, 
                                    &buffer[0]))
                {
                    printf("*** can't write to device %d ***\n", dev);
                    return -1;
                }
            }
            
            /* reset buffer */
            memset(buffer, 0, 512);
            
            /* add ebr sinature */
            ebr->ebr_signature[0] = 0x55;
            ebr->ebr_signature[1] = 0xaa;
            
            /* set start CHS */
            fwu_cal_chs(lp_start + fwu.part_lba[fwu.num_of_pp],
                        &ebr->partition[0].start_chs[0]);
            
            /* set end CHS */
            fwu_cal_chs(lp_start + lp_size - 1 + fwu.part_lba[fwu.num_of_pp], 
                        &ebr->partition[0].end_chs[0]);
            
            /* set lba */
            ebr->partition[0].lba[3] = (UINT8) (lp_start >> 24); 
            ebr->partition[0].lba[2] = (UINT8) ((lp_start & 0x00FF0000) >> 16);
            ebr->partition[0].lba[1] = (UINT8) ((lp_start & 0x0000FF00) >> 8);
            ebr->partition[0].lba[0] = (UINT8) (lp_start & 0x000000FF);
            
            /* set size */
            ebr->partition[0].len[3] = (UINT8) (lp_size >> 24); 
            ebr->partition[0].len[2] = (UINT8) ((lp_size & 0x00FF0000) >> 16);
            ebr->partition[0].len[1] = (UINT8) ((lp_size & 0x0000FF00) >> 8);
            ebr->partition[0].len[0] = (UINT8) (lp_size & 0x000000FF);
            
            /* set type */
            ebr->partition[0].type = fs_type;
            
            /* update internal structure */
            if (fwu.num_of_lp == 0)
            {
                /* record the first ebr's lba */
                fwu.ebr_lba[0] = fwu.part_lba[fwu.num_of_pp];
                
                fwu.part_lba[fwu.num_of_pp + fwu.num_of_ep] 
                = fwu.ebr_lba[0] + lp_start;
            }
            else
            {
                fwu.ebr_lba[fwu.num_of_lp] = fwu.ebr_lba[0] + next_ebr_start;
                
                fwu.part_lba[fwu.num_of_pp + fwu.num_of_ep + fwu.num_of_lp]
                = fwu.ebr_lba[fwu.num_of_lp] + lp_start;
            }
            
#ifdef FWU_DEBUG
            for (i = 0; i < 512; i++)
            {
                if ((i % 16) == 0)
                {
                    printf("\n%08x : ", i);
                }
                
                printf("%02x ", buffer[i]);
            }
            printf("\n");
#endif
            
            PRINTD("fwu.ebr_lba[0]=%lx\n", fwu.ebr_lba[0]);
            PRINTD("fwu.ebr_lba[1]=%lx\n", fwu.ebr_lba[1]);
            PRINTD("fwu.ebr_lba[2]=%lx\n", fwu.ebr_lba[2]);
            PRINTD("fwu.ebr_lba[3]=%lx\n", fwu.ebr_lba[3]);
            
            fwu.part_size[fwu.num_of_pp + fwu.num_of_ep + fwu.num_of_lp] 
            = lp_size;
            
            fwu.type[fwu.num_of_pp + fwu.num_of_ep + fwu.num_of_lp] = fs_type;
            
            /* increase the number of logical partition */
            fwu.num_of_lp++;
            
            /* add new ebr */
            if (fwu_block_write(dev, 
                                fwu.ebr_lba[fwu.num_of_lp - 1], 
                                1, 
                                &buffer[0]))
            {
                printf("*** can't write to device %d ***\n", dev);
                return -1;
            }
        }
        else
        {
            if (fwu.num_of_lp > 0)
            {
                /* if there is not only one logical partition */
                if (fwu.num_of_lp > 1)
                {
                    /* read ebr */
                    if (fwu_block_read(dev, 
                                       fwu.ebr_lba[fwu.num_of_lp - 2], 
                                       1, 
                                       &buffer[0]))
                    {
                        printf("*** can't read from device %d ***\n", dev);
                        return -1;
                    }
                    
                    /* delete information of next ebr */
                    ebr->partition[1].lba[3] = 0;
                    ebr->partition[1].lba[2] = 0;
                    ebr->partition[1].lba[1] = 0;
                    ebr->partition[1].lba[0] = 0;
                    
                    ebr->partition[1].len[3] = 0;
                    ebr->partition[1].len[2] = 0;
                    ebr->partition[1].len[1] = 0;
                    ebr->partition[1].len[0] = 0;
                    
                    /* update ebr */
                    if (fwu_block_write(dev, 
                                        fwu.ebr_lba[fwu.num_of_lp - 2], 
                                        1, 
                                        &buffer[0]))
                    {
                        printf("*** can't write to device %d ***\n", dev);
                        return -1;
                    }
                }
                
                /* reset buffer */
                memset(buffer, 0, 512);
                
                /* erase ebr */
                if (fwu_block_write(dev, 
                                    fwu.ebr_lba[fwu.num_of_lp - 1], 
                                    1, 
                                    &buffer[0]))
                {
                    printf("*** can't write to device %d ***\n", dev);
                    return -1;
                }
                
                /* reset internal variable */
                fwu.ebr_lba[fwu.num_of_lp - 1] = 0;
                fwu.part_lba[fwu.num_of_pp + fwu.num_of_ep + fwu.num_of_lp - 1] 
                = 0;
                fwu.part_size[fwu.num_of_pp + fwu.num_of_ep + fwu.num_of_lp - 1] 
                = 0;
                
                /* decrease the number of logical partition */
                fwu.num_of_lp--;
            }
            else
            {
                PRINTD("*** can delete logical partition ***\n");
                return -1;
            }
        }
    }
    else
    {
        PRINTD("*** can only generate logical partition ***\n");
        return -1;
    }
    
    return 0;   
}


int fwu_modify_partition(int dev, 
                         UINT8 is_add, 
                         UINT8 part_type, 
                         UINT32 size, 
                         UINT8 fs_type)
{
    UINT8 buffer[512] __attribute__ ((aligned(32)));
    int ret;
    
    PRINTD("dev=%d, is_add=%d, part_type=%x, size=%x, fs_type=%x\n",
            dev, is_add, part_type, size, fs_type);
    
    if (part_type != FWU_LOGIC_PART)
    {
        /* read mbr */
        if (fwu_block_read(dev, 0, 1, &buffer[0]))
        {
            printf("*** fail to read MBR ***\n");
            return -1;
        }
        
        ret =  fwu_gen_mbr(is_add, part_type, size, fs_type, buffer);
        
        if (ret)
        {
            printf("*** fail to modify MBR ***\n");
            return -1;
        }
        
        /* update mbr */
        if (fwu_block_write(dev, 0, 1, &buffer[0]))
        {
            printf("*** can't write to device %d ***\n", dev);
            return -1;
        }
    }
    else
    {
        if ((fwu.num_of_lp < 8) && (fwu.num_of_ep == 1))
        {
            ret =  fwu_gen_ebr(dev, is_add, part_type, size, fs_type, buffer);
            
            if (ret)
            {
                printf("*** fail to modify EBR ***\n");
                return -1;
            }
        }
        else
        {
            printf("*** can't add logical partition ***\n");
            return -1;
        }
    }
    
    return 0;
}


int fwu_default_partition(int dev)
{
    UINT8 buffer[512] __attribute__ ((aligned(32)));
    fwu_mbr_type *mbr;
    
#ifdef FWU_DEBUG
    UINT16 i;
#endif
    
    /* reset buffer */
    memset(buffer, 0, 512);
    
    /* reset fwu vaiables */
    fwu.num_of_pp = 0;
    fwu.num_of_ep = 0;
    fwu.num_of_lp = 0;
    
    mbr = (fwu_mbr_type *) buffer;
    
    /* generate mbr */
    fwu_gen_mbr(FWU_ADD_PART, FWU_PRIMARY_PART, FWU_KERNEL_PART_SIZE, 0x83, buffer);
    fwu_gen_mbr(FWU_ADD_PART, FWU_PRIMARY_PART, FWU_ROOTFS_PART_SIZE, 0x00, buffer);
    fwu_gen_mbr(FWU_ADD_PART, FWU_PRIMARY_PART, FWU_KERNEL_PART_SIZE, 0x83, buffer);
    fwu_gen_mbr(FWU_ADD_PART, FWU_EXTEND_PART, 0, 0x05, buffer);
    
    /* activate the first partition */
    mbr->partition[0].status = 0x80;
    
    /* write mbr block, the first block */
    if (fwu_block_write(dev, 0, 1, &buffer[0]))
    {
        printf("*** can't write to device %d ***\n", dev);
        return -1;
    }
    
    /* update internal structure */
    fwu.bootable[0] = 0x80;
    fwu.bootable[1] = 0x00;
    fwu.bootable[2] = 0x00;
    fwu.bootable[3] = 0x00;
    
    /* generate ebr */
    fwu_modify_partition(dev, FWU_ADD_PART, FWU_LOGIC_PART, FWU_ROOTFS_PART_SIZE, 0x00);
    fwu_modify_partition(dev, FWU_ADD_PART, FWU_LOGIC_PART, 64, 0x06);
    fwu_modify_partition(dev, FWU_ADD_PART, FWU_LOGIC_PART, 0, 0x83);
    
    /* show partition information */
    fwu_list_partition(dev);
    
#ifdef FWU_DEBUG
    for (i = 0; i < 8; i++)
    {
        PRINTD("part_lba[%d]=%lx, part_size[%d]=%lx, ebr_lba[%d]=%lx, bootable[%d]=%lx, type[%d]=%lx\n",
               i, fwu.part_lba[i],
               i, fwu.part_size[i],
               i, fwu.ebr_lba[i],
               i, fwu.bootable[i],
               i, fwu.type[i]);
    }
#endif
    
    PRINTD("pp=%d, ep=%d, lp=%d\n", 
           fwu.num_of_pp, fwu.num_of_ep, fwu.num_of_lp);
    
    /* change device init status */
    fwu.init = 1;
    
    return 0;
}


int fwu_check_partition(int dev)
{
    switch (dev)
    {
#ifdef CONFIG_WPCM450_SVB
        case FWU_DEV_EVB:
#endif
#ifdef CONFIG_WPCM450_WHOVILLE
        case FWU_DEV_MASER:
        case FWU_DEV_AMEA:
        case FWU_DEV_AMEA_NO_MUX:
#endif
            /* check if we have enought partitions */
            if ((fwu.num_of_pp != FWU_PRIMARY_PART_COUNT)
                || (fwu.num_of_ep != FWU_EXTENDED_PART_COUNT)
                || (fwu.num_of_lp != FWU_LOGICAL_PART_COUNT))
            { 
                printf("*** partition count was not correct ***\n");
                
                return -1;
            }
            
            /* check if each partition size is correct */
            if ((fwu.part_size[FWU_KERNEL_1] != FWU_KERNEL_PART_BLOCKS)
                || (fwu.part_size[FWU_ROOTFS_1] != FWU_ROOTFS_PART_BLOCKS)
                || (fwu.part_size[FWU_KERNEL_2] != FWU_KERNEL_PART_BLOCKS)
                || (fwu.part_size[FWU_ROOTFS_2] != FWU_ROOTFS_PART_BLOCKS)
                || (fwu.part_size[FWU_SCRATCHPAD] != FWU_SCRATCHPAD_PART_BLOCKS)
                || (fwu.part_size[FWU_EXTENDED] < (FWU_ROOTFS_PART_BLOCKS
                                                   + FWU_SCRATCHPAD_PART_BLOCKS
                                                   + FWU_OEM_PART_BLOCKS))
                || (fwu.part_size[FWU_OEM] < FWU_OEM_PART_BLOCKS))
            {
                printf("*** partition size changed ***\n");
                
                return -1;
            }
            
            break;
            
        default:
            break;
    }
    
    return 0;
}


/*
The first entry of an EBR partition table points to the logical partition 
belonging to that EBR:

Starting Sector = relative offset between this EBR sector and the first sector 
                  of the logical partition 
Note: This will be the same value for each EBR on the same hard disk; 
      usually 63. 

Number of Sectors = total count of sectors for this logical partition 
Note: The unused sectors in the same track as the EBR, are not considered 
      part of the logical partition for this count value. 

The second entry of an EBR partition table will contain zero-bytes if it's the 
last EBR in the extended partition; otherwise, it points to the next EBR in the 
EBR chain:

Starting Sector = relative address of next EBR within extended partition 
or
Starting Sector = LBA address of next EBR minus LBA address of extended 
                  partition's first EBR 

Number of Sectors = total count of sectors for next logical partition, 
                    but count starts from the next EBR sector 
Note: Unlike the first entry in an EBR's partition table, this Number of 
      Sectors count includes the next logical partition's EBR sector along 
      with the other sectors in its otherwise unused track.
*/


int fwu_check_ebr(int dev)
{
    UINT8 buffer[512] __attribute__ ((aligned(32)));
    fwu_ebr_type *ebr;
    UINT32 lp_start;
    UINT32 lp_size;
    UINT32 next_ebr_start;
    UINT32 next_ebr_size;
    
    /* reset parameter */
    fwu.num_of_lp = 0;
    
    while(1)
    {
        PRINTD("fwu.num_of_lp=%d\n", fwu.num_of_lp);
        
        /* read mbr block, the first block */
        if (fwu_block_read(dev, fwu.ebr_lba[fwu.num_of_lp], 1, &buffer[0]))
        {
            printf("*** can't read from device %d ***\n", dev);
            return -1;
        }
        
        ebr = (fwu_ebr_type *) buffer;
        
        PRINTD("fwu.ebr_lba[%d]=%lx\n", 
               fwu.num_of_lp, fwu.ebr_lba[fwu.num_of_lp]);
        
        PRINTD("ebr_signature[0]=%02x %02x\n",
                ebr->ebr_signature[0],
                ebr->ebr_signature[1]);
        
        if (ebr->ebr_signature[0] != 0x55 || ebr->ebr_signature[1] != 0xaa) 
        {
            /* no EBR signature found */
            printf("*** no EBR signature found ***\n");
            return 0;
        }
        
        /* retrieve logical partition offset */
        lp_start = ((ebr->partition[0].lba[3] << 24) |
                    (ebr->partition[0].lba[2] << 16) |
                    (ebr->partition[0].lba[1] << 8) |
                    (ebr->partition[0].lba[0]));
        
        /* retrieve logical partition size */
        lp_size = ((ebr->partition[0].len[3] << 24) |
                   (ebr->partition[0].len[2] << 16) |
                   (ebr->partition[0].len[1] << 8) |
                   (ebr->partition[0].len[0]));
        
        /* retrieve logical partition offset */
        next_ebr_start = ((ebr->partition[1].lba[3] << 24) |
                          (ebr->partition[1].lba[2] << 16) |
                          (ebr->partition[1].lba[1] << 8) |
                          (ebr->partition[1].lba[0]));
        
        /* retrieve logical partition size */
        next_ebr_size = ((ebr->partition[1].len[3] << 24) |
                         (ebr->partition[1].len[2] << 16) |
                         (ebr->partition[1].len[1] << 8) |
                         (ebr->partition[1].len[0]));
        
        PRINTD("lp_start=%lx, lp_size=%lx, next_ebr_start=%lx, next_ebr_size=%lx\n", 
               lp_start, lp_size, next_ebr_start, next_ebr_size);
        
        /* update internal structure */
        fwu.part_lba[fwu.num_of_pp + fwu.num_of_ep + fwu.num_of_lp] 
        = fwu.ebr_lba[fwu.num_of_lp] + lp_start;
        
        fwu.part_size[fwu.num_of_pp + fwu.num_of_ep + fwu.num_of_lp] = lp_size;
        
        fwu.type[fwu.num_of_pp + fwu.num_of_ep + fwu.num_of_lp] 
        = ebr->partition[0].type;
        
        fwu.bootable[fwu.num_of_pp + fwu.num_of_ep + fwu.num_of_lp] 
        = ebr->partition[0].status;
        
        /* increase the count of logical partition */
        fwu.num_of_lp++;
        
        /* check if it is the last logical partition */
        if (next_ebr_start == 0)
        {
            PRINTD("detect the last logical partition\n");
            break;
        }
        
        /* update internal structure */
        fwu.ebr_lba[fwu.num_of_lp] = fwu.ebr_lba[0] + next_ebr_start;
        
        /* limit the number of logical partition */
        if (fwu.num_of_lp >= 8)
        {
            printf("*** exceed the maximum capability of logical partition ***\n");
            break;
        }
    }
    
    return 0;   
}


int fwu_check_mbr(int dev)
{
    UINT8 buffer[512] __attribute__ ((aligned(32)));
    fwu_mbr_type *mbr;
    int i;
    
    /* read mbr block, the first block */
    if (fwu_block_read(dev, 0, 1, &buffer[0]))
    {
        printf("*** can't read from device %d ***\n", dev);
        return -1;
    }
    
    mbr = (fwu_mbr_type *) buffer;
    
    PRINTD("mbr_signature[0]=%02x %02x\n",
            mbr->mbr_signature[0],
            mbr->mbr_signature[1]);
    
    if (mbr->mbr_signature[0] != 0x55 || mbr->mbr_signature[1] != 0xaa) 
    {
        /* no MBR signature found */
        printf("*** no MBR signature found ***\n");
        return -1;
    }
    
    if (strncmp((char *) &mbr->disk_signature[0], "AVCT", 4)) 
    {
        /* no Avocent signature found */
        printf("*** no Avocent signature found ***\n");
    }
    
    /* update free space variable, minus size of mbr */
    /* fwu.free_space = fwu.free_space - 1; */
    
    /* reset variable */
    fwu.num_of_pp = 0;
    fwu.num_of_ep = 0;
    
    /* feed information to internal structure from MBR */ 
    for (i = 0; i < 4; i++) 
    {
        /* save partition information to internal structure */
        fwu.bootable[i] = mbr->partition[i].status;
        fwu.part_lba[i] = ((mbr->partition[i].lba[3] << 24) |
                           (mbr->partition[i].lba[2] << 16) |
                           (mbr->partition[i].lba[1] << 8) |
                           (mbr->partition[i].lba[0]));
        fwu.part_size[i] = ((mbr->partition[i].len[3] << 24) |
                           (mbr->partition[i].len[2] << 16) |
                           (mbr->partition[i].len[1] << 8) |
                           (mbr->partition[i].len[0]));
        fwu.type[i] = mbr->partition[i].type;
        
        PRINTD("fwu.bootable[%d]=%lx, fwu.part_lba[%d]=%lx, fwu.part_size[%d]=%lx\n", 
               i, fwu.bootable[i], 
               i, fwu.part_lba[i], 
               i, fwu.part_size[i]);
        
        /* if it is the first entry */
        if (i == 0)
        {
            if (fwu.part_size[0] != 0)
            {
                fwu.num_of_pp = 1;
            }
            else
            {
                fwu.num_of_pp = 0;
                break;
            }
        }
        else
        {
            if (fwu.part_size[i] != 0)
            {
                /* check if the value is reasonable */
                if ((fwu.part_lba[i - 1] + fwu.part_size[i - 1]) 
                    > fwu.part_lba[i])
                {
                    printf("*** MBR error ***\n");
                    break;
                }
                
                /* check if it is an extended partition */
                if (fwu.type[i] == 0x05)
                {
                    /* record the first ebr's lba */
                    fwu.ebr_lba[0] = fwu.part_lba[i];
                    
                    fwu.num_of_ep = 1;
                    break;
                }
                else
                {
                    fwu.num_of_pp++;
                }
            }
            else
            {
                break;
            }
        }
    }
    
    return 0;
}


int fwu_check_image(int dev, UINT8 *buffer)
{
    fwu_image_header_type *image_header;
    fwu_image_info_type *image_info;
    ps_info_type *ps;
    char parameter[32];
    UINT32 size;
    UINT8 img_type;
    
    PRINTD("fwu_check_image\n");
    
    image_header = (fwu_image_header_type *) buffer;
    
    /* shift to image information */
    image_info 
    = (fwu_image_info_type *) (buffer + sizeof(fwu_image_header_type));
    
    PRINTD("buffer=%p, image_header=%p, image_info=%p\n",
           buffer, image_header, image_info);
    
    PRINTD("crc32=%lx, header_ver=%x, num_of_image=%x, version=%lx, img_size=%lx\n", 
           image_header->crc32,
           image_header->header_ver, 
           image_header->num_of_image, 
           image_header->version, 
           image_header->img_size);
    
    if(image_header->header_ver != 1)
    {
        printf("*** unknown image version ***\n");
        
        return -1;
    }
    
    switch (dev)
    {
#ifdef CONFIG_WPCM450_SVB
        case FWU_DEV_EVB:
#endif
#ifdef CONFIG_WPCM450_WHOVILLE
        case FWU_DEV_MASER:
        case FWU_DEV_AMEA:
        case FWU_DEV_AMEA_NO_MUX:
#endif
            /* setting the expect image type */
            img_type = 1;
            break;
            
        case FWU_DEV_SPI:
            img_type = 3;
            
            break;
            
        default:
            img_type = 1;
            break;
    }
    
    if (image_header->img_type != img_type)
    {
        printf("*** wrong image type %d ***\n", image_header->img_type);
        printf("*** expected image type is %d ***\n", img_type);  
        return -1;
    }
    
    printf("Checking image header CRC ... ");
    
    /* check crc32 for the header, header size is fixed to 512 bytes */
    if (crc32 (0, (buffer + 4), 508) != image_header->crc32)
    {
        PRINTD("buffer+4=%p\n", buffer + 4);
        PRINTD("crc32=%lx\n", (crc32 (0, (buffer + 4), 508)));
        
        printf("Bad CRC\n");
        
        return -1;
    }
    
    printf("OK\n");
    
    printf("Checking platform ID ........ ");
    
    /* get the start address of persistent storage */
    ps = (ps_info_type *) CFG_PS_ADDR;
    
    /* check platform ID */
    if ((*((UINT32 *) &ps->platform_id[0])) != 0xFFFFFFFF)
    {
        if (((*((UINT32 *) &image_header->platform_id[0])) != 0x00000000)
            && ((*((UINT32 *) &image_header->platform_id[0])) != 0xFFFFFFFF))
        {
            if ((strncmp((char *) &image_header->platform_id[0], 
                         CONFIG_WPCM450_PLATFORM_ID, 
                         4)) 
                && (sizeof(CONFIG_WPCM450_PLATFORM_ID) > 1))
            {
                printf("Fail  (Image: %s != Platform: %c%c%c%c)\n", 
                        CONFIG_WPCM450_PLATFORM_ID, 
                        (char) image_header->platform_id[0],
                        (char) image_header->platform_id[1],
                        (char) image_header->platform_id[2],
                        (char) image_header->platform_id[3]);
                
                return -1;
            }
        }
    }
    
    printf("OK\n");
    
    /* reset kernel/rootfs/uboot size variable */
    setenv("fwu_kernel_size", "0");
    setenv("fwu_rootfs_size", "0");
    setenv("fwu_uboot_size", "0");
    
    /* retrieve kernel image information, the 1st image is always kernel */
    if (image_header->num_of_image >= 1)
    {
        printf("Checking kernel image CRC ... ");
        
        PRINTD("offset=%lx, size=%lx, crc32=%lx\n", 
               image_info->offset,
               image_info->size, 
               image_info->crc32);
        
        /* check crc32 for the kernel image */
        if (image_info->crc32 
            != crc32 (0, (buffer + image_info->offset), image_info->size))
        {
            PRINTD("crc32=%lx\n", 
                   crc32 (0, (buffer + image_info->offset), image_info->size));
            
            printf("Bad CRC\n");
            
            /* setenv("fwu_kernel_size", "0"); */
            
            return -1;
        }
        
        printf("OK\n");
        
        /* update kernel load address */
        sprintf(parameter, "%lx", ((UINT32) buffer + image_info->offset));
        setenv("fwu_kernel_offset", parameter);
        
        /* get image size from image information structure */
        size = image_info->size;
        
        switch (dev)
        {
#ifdef CONFIG_WPCM450_SVB
            case FWU_DEV_EVB:
#endif
#ifdef CONFIG_WPCM450_WHOVILLE
            case FWU_DEV_MASER:
            case FWU_DEV_AMEA:
            case FWU_DEV_AMEA_NO_MUX:
#endif
                /* change size to block unit */
                size = fwu_cal_blocks(size);
                break;
                
            case FWU_DEV_SPI:
                
                /* change size to sector unit */
                /* size = fwu_cal_sectors(size); */
                
                break;
                
            default:
                break;
        }
        
        PRINTD("size=%lx\n", size);
        
        sprintf(parameter, "%lx", size);
        
        setenv("fwu_kernel_size", parameter);
        
        /* retrieve rootfs image information, the 2nd image is always rootfs */
        if (image_header->num_of_image >= 2)
        {
            printf("Checking rootfs image CRC ... ");
            
            /* move to the next image information */
            image_info++;
            
            PRINTD("offset=%lx, size=%lx, crc32=%lx\n", 
                   image_info->offset,
                   image_info->size, 
                   image_info->crc32);
            
            /* check crc32 for the rootfs image */
            if (image_info->crc32 
                != crc32 (0, (buffer + image_info->offset), image_info->size))
            {
                PRINTD("crc32=%lx\n", 
                       crc32 (0, 
                              (buffer + image_info->offset), 
                              image_info->size));
                
                printf("Bad CRC\n");
                
                /* setenv("fwu_rootfs_size", "0"); */
                
                return -1;
            }
            
            printf("OK\n");
            
            /* update rootfs load address */
            sprintf(parameter, "%lx", ((UINT32) buffer + image_info->offset));
            setenv("fwu_rootfs_offset", parameter);
            
            /* get image size from image information structure */
            size = image_info->size;
            
            switch (dev)
            {
#ifdef CONFIG_WPCM450_SVB
                case FWU_DEV_EVB:
#endif
#ifdef CONFIG_WPCM450_WHOVILLE
                case FWU_DEV_MASER:
                case FWU_DEV_AMEA:
                case FWU_DEV_AMEA_NO_MUX:
#endif
                    /* change size to block unit */
                    size = fwu_cal_blocks(size);
                    break;
                    
                case FWU_DEV_SPI:
                    
                    /* change size to sector unit */
                    /* size = fwu_cal_sectors(size); */
                    
                    break;
                    
                default:
                    break;
            }
            
            PRINTD("size=%lx\n", size);
            
            sprintf(parameter, "%lx", size);
            
            setenv("fwu_rootfs_size", parameter);
        }
        
        /* retrieve u-boot image information, the 3rd image is always u-boot */
        if (image_header->num_of_image >= 3)
        {
            printf("Checking u-boot image CRC ... ");
            
            /* move to the next image information */
            image_info++;
            
            PRINTD("offset=%lx, size=%lx, crc32=%lx\n", 
                   image_info->offset,
                   image_info->size, 
                   image_info->crc32);
            
            /* check crc32 for the u-boot image */
            if (image_info->crc32 
                != crc32 (0, (buffer + image_info->offset), image_info->size))
            {
                PRINTD("crc32=%lx\n", 
                       crc32 (0, 
                              (buffer + image_info->offset), 
                              image_info->size));
                
                printf("Bad CRC\n");
                
                /* setenv("fwu_uboot_size", "0"); */
                
                return -1;
            }
            
            printf("OK\n");
            
            printf("Skipping u-boot update ...... ");
            
            /* check U-Boot version */
            if ((image_header->uboot_ver[0] 
                 == fwu_bin2bcd(simple_strtoul(CONFIG_UBOOT_VERSION, 
                                               NULL, 
                                               10)))
                && (image_header->uboot_ver[1] 
                    == fwu_bin2bcd(simple_strtoul(CONFIG_UBOOT_PATCHLEVEL, 
                                                  NULL, 
                                                  10)))
                && (image_header->uboot_ver[2] 
                    == fwu_bin2bcd(simple_strtoul(CONFIG_UBOOT_SUBLEVEL, 
                                                  NULL, 
                                                  10)))
                && (image_header->uboot_ver[4] 
                    == fwu_bin2bcd(simple_strtoul(CONFIG_AVCT_VERSION, 
                                                  NULL, 
                                                  10)))
                && (image_header->uboot_ver[5] 
                    == fwu_bin2bcd(simple_strtoul(CONFIG_AVCT_PATCHLEVEL, 
                                                  NULL, 
                                                  10)))
                && (image_header->uboot_ver[6] 
                    == fwu_bin2bcd(simple_strtoul(CONFIG_AVCT_SUBLEVEL, 
                                                  NULL, 
                                                  10))))
            {
                printf("YES\n");
                
                return 0;
            }
            
            printf("NO\n");
            
            /* update u-boot load address */
            sprintf(parameter, "%lx", ((UINT32) buffer + image_info->offset));
            setenv("fwu_uboot_offset", parameter);
            
            /* get image size from image information structure */
            size = image_info->size;
            
            PRINTD("size=%lx\n", size);
            
            sprintf(parameter, "%lx", size);
            
            setenv("fwu_uboot_size", parameter);
        }
    }
    
    return 0;
}


int fwu_update_image(int dev, UINT8 *buffer)
{
    int ret;
    UINT32 size;
    UINT32 offset;
    UINT32 start;
    
    PRINTD("fwu_update_image\n");
    
    /* check image first */
    ret = fwu_check_image(dev, buffer);
    
    if (ret)
    {
        return ret;
    }
    
    /* update kernel image */
    size = simple_strtoul(getenv("fwu_kernel_size"), NULL, 16);
    
    PRINTD("kernel size=%lx\n", size);
    
    if (size)
    {
        printf("Copying kernel image ........ ");
        
        offset = simple_strtoul(getenv("fwu_kernel_offset"), NULL, 16);
        start = simple_strtoul(getenv("fwu_kernel_start"), NULL, 16);
        
        PRINTD("kernel offset=%lx, start=%lx, size=%lx\n", offset, start, size);
        
        ret = fwu_block_write(dev, 
                              start,
                              size,
                              (UINT8 *) offset);
        
        if (ret) 
        {
            printf("Fail\n");
            printf("*** fail to update kernel image ***\n");
            return ret;
        }
        
        printf("OK\n");
    }
    
    /* update rootfs image */
    size = simple_strtoul(getenv("fwu_rootfs_size"), NULL, 16);
    
    PRINTD("rootfs size=%lx\n", size);
    
    if (size)
    {
        printf("Copying rootfs .............. ");
        
        offset = simple_strtoul(getenv("fwu_rootfs_offset"), NULL, 16);
        start = simple_strtoul(getenv("fwu_rootfs_start"), NULL, 16);
        
        PRINTD("rootfs offset=%lx, start=%lx, size=%lx\n", offset, start, size);
        
        ret = fwu_block_write(dev, 
                              start,
                              size,
                              (UINT8 *) offset);
        
        if (ret) 
        {
            printf("Fail\n");
            printf("*** fail to update rootfs image ***\n");
            return ret;
        }
        
        printf("OK\n");
    }
    
    /* update u-boot image */
    size = simple_strtoul(getenv("fwu_uboot_size"), NULL, 16);
    
    PRINTD("uboot size=%lx\n", size);
    
    if (size)
    {
        printf("Copying u-boot .............. ");
        
        offset = simple_strtoul(getenv("fwu_uboot_offset"), NULL, 16);
        start = simple_strtoul(getenv("uboot_start"), NULL, 16);
        
        PRINTD("rootfs offset=%lx, start=%lx, size=%lx\n", offset, start, size);
        
        ret = fwu_block_write(FWU_DEV_SPI, 
                              start,
                              size,
                              (UINT8 *) offset);
        
        if (ret) 
        {
            printf("Fail\n");
            printf("*** fail to update u-boot image ***\n");
            return ret;
        }
        
        printf("OK\n");
    }
    
    /* update environment */
    fwu_update_env(dev);
    
    /* saveenv(); */
    
    return 0;
}


#ifdef CONFIG_WPCM450_BOOT_STATUS
/* check the last boot status */
int fwu_check_boot_status(int dev)
{
    UINT8 buffer[512] __attribute__ ((aligned(32)));
    message_type *message;
    UINT32 kernel_start;
    UINT32 rootfs_start;
    UINT32 start;
    UINT32 size;
    UINT32 load;
    int ret = 0;
    
    message = (message_type *) FWU_BOOT_STATUS_ADDR;
    
    PRINTD("crc32=%lx ver=%x boot_image=%x boot_status=%x system_status=%x\n",
            message->crc32,
            message->ver,
            message->boot_image,
            message->boot_status,
            message->system_status);
    
    switch (dev)
    {
#ifdef CONFIG_WPCM450_SVB
        case FWU_DEV_EVB:
#endif
#ifdef CONFIG_WPCM450_WHOVILLE
        case FWU_DEV_MASER:
        case FWU_DEV_AMEA:
        case FWU_DEV_AMEA_NO_MUX:
#endif
            break;
            
        case FWU_DEV_SPI:
            /* spi device doesn't support boot status checking */
            return 0;
    }
    
    /* clena up boot status memory if crc32 is incorrect */
    if (crc32(0, ((UINT8 *) message + 4), (sizeof(message_type) - 4)) 
        != message->crc32)
    {
        printf("*** generated boot status checksum ***\n");
        
        /* clean up boot status memory */
        memset(message, 0, sizeof(message_type));
        message->ver = 1;
        message->boot_image = FWU_BOOT_N_IMG_1ST;
        message->crc32 
        = crc32(0, ((UINT8 *) message + 4), (sizeof(message_type) - 4));
        
        return 1;
    }
    
    /* reset U-Boot to kernel status */
    message->boot_status = FWU_BOOT_NO_STATUS;
    
    /* handle kernel to U-Boot status */
    switch (message->system_status)
    {
        case FWU_BOOT_NO_STATUS:
        case FWU_BOOT_KERNEL_SUPPORT:
        case FWU_BOOT_ROOT_MOUNTED:
            
            PRINTD("FWU_BOOT_NO_STATUS\n");
            PRINTD("FWU_BOOT_KERNEL_SUPPORT\n");
            PRINTD("FWU_BOOT_ROOT_MOUNTED\n");
            
            /* if it is a watchdog reset */
            if (WTCR == 0x00)
            {
                SET_BIT(message->boot_status, FWU_BOOT_WATCHDOG);
            }
            
            if (message->boot_image != FWU_BOOT_NO_STATUS)
            {
                printf("*** previous boot was failed, retry=%02x ***\n", 
                       message->boot_image & 0x0F);
            }
            
            switch (message->boot_image)
            {
                case FWU_BOOT_NO_STATUS:
                    
                    /* record retry count of current image */
                    message->boot_image = FWU_BOOT_N_IMG_1ST;
                    
                    break;
                
                case FWU_BOOT_N_IMG_1ST:
                    
                    /* record retry count of current image */
                    message->boot_image = FWU_BOOT_N_IMG_2ND;
                    
                    break;
                    
                case FWU_BOOT_N_IMG_2ND:
                    
                    /* record retry count of current image */
                    message->boot_image = FWU_BOOT_N_IMG_3RD;
                    
                    break;
                    
                case FWU_BOOT_N_IMG_3RD:
                    
                    /* record the active image is bad */
                    SET_BIT(message->boot_status, FWU_BOOT_N_IMG_BAD);
                    
                    /* record retry count of current image */
                    message->boot_image = FWU_BOOT_N_1_IMG_1ST;
                    
                    /* record N image kernel start address */
                    kernel_start 
                    = simple_strtoul(getenv("fwu_kernel_start"), NULL, 16);
                    
                    /* record N image rootfs start address */
                    rootfs_start 
                    = simple_strtoul(getenv("fwu_rootfs_start"), NULL, 16);
                    
                    printf("*** swap to N-1 image ***\n");
                    
                    /* swap active partition */
                    ret = fwu_swap_act_partition(dev);
                    
                    if (ret)
                    {
                        printf("*** fail to swap to N-1 image ***\n");
                        break;
                    }
                    
                    /* prepare to replenish N image with N-1 image */
                    start = simple_strtoul(getenv("fwu_kernel_start"), NULL, 16);
                    size = simple_strtoul(getenv("fwu_kernel_size"), NULL, 16);
                    load = simple_strtoul(getenv("offset"), NULL, 16);
                    
                    printf("*** replenish N image with N-1 image ***\n");
                    
                    PRINTD("kernel size=%lx\n", size);
                    
                    /* read N-1 kernel image to memory */
                    if (fwu_block_read(dev, 
                                       start, 
                                       size, 
                                       (UINT8 *) load))
                    {
                        printf("*** can't read from device %d ***\n", dev);
                    }
                    
                    /* write N kernel image from memory */
                    if (fwu_block_write(dev, 
                                        kernel_start,
                                        size,
                                        (UINT8 *) load)) 
                    {
                        printf("*** can't write to device %d ***\n", dev);
                    }
                    
                    start = simple_strtoul(getenv("fwu_rootfs_start"), NULL, 16);
                    size = simple_strtoul(getenv("fwu_rootfs_size"), NULL, 16);
                    
                    PRINTD("rootfs size=%lx\n", size);
                    
                    /* read N-1 rootfs image to memory */
                    if (fwu_block_read(dev, 
                                       start, 
                                       size, 
                                       (UINT8 *) load))
                    {
                        printf("*** can't read from device %d ***\n", dev);
                    }
                    
                    /* write N rootfs image from memory */
                    if (fwu_block_write(dev, 
                                        rootfs_start,
                                        size,
                                        (UINT8 *) load)) 
                    {
                        printf("*** can't write to device %d ***\n", dev);
                    }
                    
                    break;
                    
                case FWU_BOOT_N_1_IMG_1ST:
                    
                    /* record the active image is bad */
                    SET_BIT(message->boot_status, FWU_BOOT_N_IMG_BAD);
                    
                    /* record retry count of current image */
                    message->boot_image = FWU_BOOT_N_1_IMG_2ND;
                    
                    break;
                    
                case FWU_BOOT_N_1_IMG_2ND:
                    
                    /* record the active image is bad */
                    SET_BIT(message->boot_status, FWU_BOOT_N_IMG_BAD);
                    
                    /* record retry count of current image */
                    message->boot_image = FWU_BOOT_N_1_IMG_3RD;
                    
                    break;
                    
                case FWU_BOOT_N_1_IMG_3RD:
                    
                    /* record the active image is bad */
                    SET_BIT(message->boot_status, FWU_BOOT_N_IMG_BAD);
                    
                    /* reset boot image status */
                    message->boot_image = FWU_BOOT_NO_STATUS;
                    
                    /* prepare to destroy N and N-1 images */
                    printf("*** destroy N and N-1 images ***\n");
                    
                    /* reset buffer */
                    memset(buffer, 0, 512);
                    
                    buffer[0] = 0xba;
                    buffer[1] = 0xd0;
                    
                    /* destroy 1st kernel image */
                    if (fwu_block_write(dev, 
                                        fwu.part_lba[FWU_KERNEL_1],
                                        1,
                                        (UINT8 *) buffer)) 
                    {
                        printf("*** can't write to device %d ***\n", dev);
                    }
                    
                    /* destroy 1st rootfs image */
                    if (fwu_block_write(dev, 
                                        fwu.part_lba[FWU_ROOTFS_1],
                                        1,
                                        (UINT8 *) buffer)) 
                    {
                        printf("*** can't write to device %d ***\n", dev);
                    }
                    
                    /* destroy 2nd kernel image */
                    if (fwu_block_write(dev, 
                                        fwu.part_lba[FWU_KERNEL_2],
                                        1,
                                        (UINT8 *) buffer)) 
                    {
                        printf("*** can't write to device %d ***\n", dev);
                    }
                    
                    /* destroy 2nd rootfs image */
                    if (fwu_block_write(dev, 
                                        fwu.part_lba[FWU_ROOTFS_2],
                                        1,
                                        (UINT8 *) buffer)) 
                    {
                        printf("*** can't write to device %d ***\n", dev);
                    }
                    
                    ret = -1;
                    
                    break;
                    
                default:
                    printf("*** unsupport boot image status %x ***\n", 
                           message->system_status);
                    break;
            }
            
            break;
            
        case FWU_BOOT_USER_REBOOT:
            
            PRINTD("FWU_BOOT_USER_REBOOT\n");
            
            SET_BIT(message->boot_status, FWU_BOOT_RESET);
            
            /* reset boot image status */
            message->boot_image = FWU_BOOT_N_IMG_1ST;
            
            break;
            
        case FWU_BOOT_ALL_RUNNING:
            
            PRINTD("FWU_BOOT_ALL_RUNNING\n");
            
            /* if it is a watchdog reset */
            if (WTCR == 0x00)
            {
                printf("*** detected watchdog timeout reset %x ***\n", 
                       message->system_status);
                
                SET_BIT(message->boot_status, FWU_BOOT_WATCHDOG);
            }
            
            /* reset boot image status */
            message->boot_image = FWU_BOOT_N_IMG_1ST;
            
            break;
            
        default:
            printf("*** unsupport system status %x ***\n", 
                   message->system_status);
            break;
    }
    
    /* reset kernel to U-Boot status */
    message->system_status = FWU_BOOT_NO_STATUS;
    
    /* update checksum */
    message->crc32 
    = crc32(0, ((UINT8 *) message + 4), (sizeof(message_type) - 4));
    
    PRINTD("crc32=%lx ver=%x boot_image=%x boot_status=%x system_status=%x\n",
            message->crc32,
            message->ver,
            message->boot_image,
            message->boot_status,
            message->system_status);
    
    return ret;
}
#endif


int fwu_boot_image(int dev)
{
    UINT32 load;
    UINT32 start;
    UINT32 size;
    char *bootm_args[3];
    char arg0[16] = "bootm";
    char arg1[16] = "";
    char arg2[16] = "";
    int ret;
    
    /* handle the retry */
    while (1)
    {
        /* prepare parameter array */
        bootm_args[0] = &arg0[0];
        bootm_args[1] = &arg1[0];
        bootm_args[2] = &arg2[0];
        
        /* update environment */
        fwu_update_env(dev);
        
        /* check previous boot status */
        ret = fwu_check_boot_status(dev);
        
        if (ret < 0)
        {
            return ret;
        }
        
        load = simple_strtoul(getenv("fwu_kernel_load"), NULL, 16);
        start = simple_strtoul(getenv("fwu_kernel_start"), NULL, 16);
        size = simple_strtoul(getenv("fwu_kernel_size"), NULL, 16);
        
        PRINTD("load=%lx, start=%lx, size=%lx\n", load, start, size);
        
        /* copy kernel image from flash to ram */
        if (size != 0)
        {
            if (load != 0)
            {
                printf("Copying kernel image ... ");
                
                if (fwu_block_read(dev, start, size, (UINT8 *) load))
                {
                    printf("Fail\n");
                    printf("*** can't read from device %d ***\n", dev);
                    return -1;
                }
                
                /* give kernel image address to bootm */
                sprintf(bootm_args[1], "%lx", load);
                
                PRINTD("bootm_args[1]=%s, load=%lx\n", bootm_args[1], load);
                
                printf("OK\n");
            }
            else
            {
                /* give kernel image address to bootm */
                sprintf(bootm_args[1], "%lx", start);
                
                PRINTD("bootm_args[1]=%s, load=%lx\n", bootm_args[1], load);
            }
            
            load = simple_strtoul(getenv("fwu_rootfs_load"), NULL, 16);
            start = simple_strtoul(getenv("fwu_rootfs_start"), NULL, 16);
            size = simple_strtoul(getenv("fwu_rootfs_size"), NULL, 16);
            
            PRINTD("load=%lx, start=%lx, size=%lx\n", load, start, size);
            
            /* copy rootfs image from flash to ram */
            if (size != 0)
            {
                if (load != 0)
                {
                    printf("Copying rootfs image ... ");
                    
                    if (fwu_block_read(dev, start, size, (UINT8 *) load))
                    {
                        printf("Fail\n");
                        printf("*** can't read from device %d ***\n", dev);
                        return -1;
                    }
                    
                    /* give rootfs image address to bootm */
                    sprintf(bootm_args[2], "%lx", load);
                    
                    PRINTD("bootm_args[2]=%s, load=%lx\n", bootm_args[2], load);
                    
                    printf("OK\n");
                }
                else
                {
                    PRINTD("rootfs is not uimage\n");
                    
                    if (dev == FWU_DEV_SPI)
                    {
                        printf("Copying rootfs image ... ");
                        
                        load = simple_strtoul(getenv("rootfs_load"), NULL, 16);
                        
                        if (fwu_block_read(dev, start, size, (UINT8 *) load))
                        {
                            printf("Fail\n");
                            printf("*** can't read from device %d ***\n", dev);
                            return -1;
                        }
                        
                        printf("OK\n");
                        
                        /* bootm does not need to handle rootfs */
                        load = 0;
                    }
                    else
                    {
                        /* printf("Skip rootfs image\n"); */
                    }
                }
            }
            else
            {
                printf("*** rootfs image is invalid ***\n");
            }
        }
        else
        {
            printf("*** kernel image is invalid ***\n");
        }
        
        PRINTD("bootm_args=%p, bootm_args[1]=%s, bootm_args[2]=%s\n", 
               bootm_args, bootm_args[1], bootm_args[2]);
        
        /* check if the image size is valid */
        if (size)
        {
            /* start to boot from the kernel image */
            if (load)
            {
                /* it won't return if succeed */
                ret = do_bootm(NULL, 0, 3, bootm_args);
            }
            else
            {
                /* it won't return if succeed */
                ret = do_bootm(NULL, 0, 2, bootm_args);
            }
        }
        
        /* ignore non-partition device */
        if (dev == FWU_DEV_SPI)
        {
            PRINTD("do not try the 2nd image for non-partition device\n");
            return -1;
        }
    }
}


/* init the firmware upgrade sub-function */
int fwu_init(void)
{
    int ret;
    
    PRINTD("fwu_init\n");
    
    /* reset device init status */
    fwu.init = 0;
    
    /* get device type */
    if (fwu_get_dev_type(&fwu.dev))
    {
        return -1;
    }
    
    PRINTD("fwu.dev=%d\n", fwu.dev);
    
    /* initiate device */
    if (fwu_dev_init(fwu.dev))
    {
        return -1;
    }
    
    /* reset partition variable */
    fwu.num_of_pp = 0;
    fwu.num_of_ep = 0;
    fwu.num_of_lp = 0;
    
    /* get device size */
    fwu_get_dev_size(fwu.dev, &fwu.size);
    
    /* update free space variable */
    fwu.free_space = fwu.size;
    
    switch (fwu.dev)
    {
#ifdef CONFIG_WPCM450_SVB
        case FWU_DEV_EVB:
#endif
#ifdef CONFIG_WPCM450_WHOVILLE
        case FWU_DEV_MASER:
        case FWU_DEV_AMEA:
        case FWU_DEV_AMEA_NO_MUX:
#endif
            /* check mbr */
            ret = fwu_check_mbr(fwu.dev);
            
            if (ret)
            {
                /* force to stay in U-boot if there is no active partition */
                /* setenv("bootdelay", "-1"); */
                
                /* force to create default partitions */
                if (fwu.dev == FWU_DEV_MASER)
                {
                    printf("*** create default partitions ***\n");
                    
                    /* create default partitions if mbr is not existent */
                    ret = fwu_default_partition(fwu.dev);
                    
                    if (ret)
                    {
                        return ret;
                    }
                }
                else
                {
                    return ret;
                }
            }
            
            /* check logical partition if there is a extended partition */
            if (fwu.num_of_ep)
            {
                PRINTD("found extended partition\n");
                
                /* check ebr */
                ret = fwu_check_ebr(fwu.dev);
                
                if (ret)
                {
                    /* force to stay in U-boot if there is no active partition */
                    /* setenv("bootdelay", "-1"); */
                    
                    return ret;
                }
            }
            
            /* check the partition size and count */
            if (fwu.dev == FWU_DEV_MASER)
            {
                ret = fwu_check_partition(fwu.dev);
                
                if (ret)
                {
                    printf("*** create default partitions ***\n");
                    
                    /* create default partitions if size is not correct */
                    ret = fwu_default_partition(fwu.dev);
                    
                    if (ret)
                    {
                        return ret;
                    }
                }
            }
            
            break;
            
        case FWU_DEV_SPI:
            break;
            
        default:
            break;
    }
    
    /* update environment */
    fwu_update_env(fwu.dev);
    
    /* change device init status */
    fwu.init = 1;
    
    return 0;   
}


int do_fwu(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
    int ret;
    UINT32 offset;
    UINT32 start;
    UINT32 blocks;
    UINT32 bytes;
    UINT8 partition;
    UINT8 size;
    UINT8 type;
    char parameter[32];
    
    switch (argc) 
    {
        case 0:
        case 1:
            break;
            
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
            if (strcmp (argv[1], "init") == 0) 
            {
                ret = fwu_init();
                
                if (ret) 
                {
                    printf("*** fail to initiate firmware upgrade function ***\n");
                    return ret;
                }
                
                printf("Done!\n");
                
                return 0;
            }
            
            else if (strcmp (argv[1], "dev") == 0) 
            {
                if (argc < 3)
                {
                    fwu_print_dev_list();
                    
                    return 0;
                }
                
                type = (UINT8) simple_strtoul(argv[2], NULL, 16);
                
                if (type <= FWU_DEV_LAST)
                {
                    /* update environment variable */
                    fwu_set_dev_type(type);
                }
                else
                {
                    printf("*** unsupported device type ***\n");
                    return -1;
                }
                
                /* re-init device */
                ret = fwu_init();
                
                if (ret) 
                {
                    printf("*** fail to set device ***\n");
                    return ret;
                }
                
                printf("\n");
                
                fwu_print_dev_list();
                
                return 0;
            }
            
            else if (strcmp (argv[1], "info") == 0) 
            {
                /* ignore non-partition device */
                if (fwu.dev == FWU_DEV_SPI)
                {
                    printf("*** not support in this device ***\n");
                    return 0;
                }
                
                ret = fwu_list_partition(fwu.dev);
                
                if (ret) 
                {
                    printf("*** fail to list partition information ***\n");
                    return ret;
                }
                
                return 0;
            }
            
            else if (strcmp (argv[1], "fdisk") == 0) 
            {
                /* ignore non-partition device */
                if (fwu.dev == FWU_DEV_SPI)
                {
                    printf("*** not support in this device ***\n");
                    return 0;
                }
                
                if (argc < 3)
                {
                    PRINTD("*** argc < 3 ***\n");
                    break;
                }
                
                if (strcmp (argv[2], "default") == 0) 
                {
                    ret = fwu_default_partition(fwu.dev);
                    
                    if (ret)
                    {
                        printf("*** fail to create default partition ***\n");
                        return ret;
                    }
                }
                else if (strcmp (argv[2], "list") == 0) 
                {
                    printf(" Partition Type\n");
                    printf("-----------------\n");
                    printf(" 0x00  Empty\n");
                    printf(" 0x05  Extended\n");
                    printf(" 0x06  FAT16\n");
                    printf(" 0x0B  W95 FAT32\n");
                    printf(" 0x83  Linux\n\n");
                    
                    return 0;
                }
                else if ((strcmp (argv[2], "add") == 0) 
                         || (strcmp (argv[2], "del") == 0))
                {
                    if (argc < 4)
                    {
                        PRINTD("*** argc < 4 ***\n");
                        break;
                    }
                    
                    if (strcmp (argv[3], "p") == 0)
                    {
                        if (strcmp (argv[2], "add") == 0)
                        {
                            if ((fwu.num_of_pp < 4) && (fwu.num_of_ep == 0))
                            {
                                partition = 0;
                            }
                            else
                            {
                                printf("*** can't add primary partition ***\n");
                                return -1;
                            }
                        }
                        else
                        {
                            if ((fwu.num_of_pp == 0) 
                                || (fwu.num_of_ep != 0) 
                                || (fwu.num_of_lp != 0))
                            {
                                printf("*** can't delete primary partition ***\n");
                                return -1;
                            }
                            
                            partition = 0;
                        }
                    }
                    else if (strcmp (argv[3], "e") == 0)
                    {
                        if (strcmp (argv[2], "add") == 0)
                        {
                            if (fwu.num_of_ep == 0)
                            {
                                partition = 1;
                                
                                if (argc < 5)
                                {
                                    PRINTD("*** argc < 5 ***\n");
                                    break;
                                }
                                
                                size = (UINT8) simple_strtoul(argv[4], NULL, 16);
                                
                                ret = fwu_modify_partition(fwu.dev, 
                                                           FWU_ADD_PART, /* add */
                                                           partition,
                                                           size,
                                                           0);    /* don't care */
                                
                                if (ret)
                                {
                                    printf("*** fail to create extended partition ***\n");
                                    return ret;
                                }
                            }
                            else
                            {
                                printf("*** can't add extended partition ***\n");
                                return -1;
                            }
                        }
                        else
                        {
                            if ((fwu.num_of_ep == 0)
                                || (fwu.num_of_lp != 0))
                            {
                                printf("*** can't delete extended partition ***\n");
                                return -1;
                            }
                            
                            partition = 1;
                        }
                    }
                    else if (strcmp (argv[3], "l") == 0)
                    {
                        if (strcmp (argv[2], "add") == 0)
                        {
                            if ((fwu.num_of_ep == 1) && (fwu.num_of_lp < 8))
                            {
                                partition = 2;
                            }
                            else
                            {
                                printf("*** can't add logical partition ***\n");
                                return -1;
                            }
                        }
                        else
                        {
                            if (fwu.num_of_lp == 0)
                            {
                                printf("*** can't delete logical partition ***\n");
                                return -1;
                            }
                            
                            partition = 2;
                        }
                    }
                    else
                    {
                        PRINTD("*** argv[3] doesn't match ***\n");
                        break;
                    }
                    
                    if (strcmp (argv[2], "add") == 0)
                    {
                        if (argc < 6)
                        {
                            PRINTD("*** argc < 6 ***\n");
                            break;
                        }
                        
                        size = (UINT8) simple_strtoul(argv[4], NULL, 16);
                        type = (UINT8) simple_strtoul(argv[5], NULL, 16);
                        
                        ret = fwu_modify_partition(fwu.dev, 
                                                   FWU_ADD_PART, /* add */
                                                   partition,
                                                   size,
                                                   type);
                        
                        if (ret)
                        {
                            printf("*** fail to add partition ***\n");
                            return ret;
                        }
                    }
                    else
                    {
                        ret = fwu_modify_partition(fwu.dev, 
                                                   FWU_DEL_PART,   /* delete */
                                                   partition,
                                                   0,          /* don't care */
                                                   0);         /* don't care */
                        
                        if (ret)
                        {
                            printf("*** fail to delete partition ***\n");
                            return ret;
                        }
                    }
                    
                    return 0;
                }
                else
                {
                    PRINTD("*** argv[2] doesn't match ***\n");
                    break;
                }
                
                return 0;
            }
            
            else if (strcmp (argv[1], "erase") == 0) 
            {
                /* ignore non-partition device */
                if (fwu.dev == FWU_DEV_SPI)
                {
                    printf("*** not support in this device ***\n");
                    return 0;
                }
                
                if (argc < 3)
                {
                    break;
                }
                
                partition = (UINT8) simple_strtoul(argv[2], NULL, 16);
                
                /* check partition number */
                if ((partition == 0) 
                    || (partition > (fwu.num_of_pp 
                                     + fwu.num_of_ep 
                                     + fwu.num_of_lp)))
                {
                    printf("*** partition number is not correct ***\n");
                    return -1;
                }
                
                ret = fwu_block_erase(fwu.dev, 
                                      fwu.part_lba[partition - 1], 
                                      fwu.part_size[partition - 1]);
                
                if (ret) 
                {
                    printf("*** fail to activate partition ***\n");
                    return ret;
                }
                
                printf("Done!\n");
                
                return 0;
            }
            
            else if (strcmp (argv[1], "act") == 0) 
            {
                /* ignore non-partition device */
                if (fwu.dev == FWU_DEV_SPI)
                {
                    printf("*** not support in this device ***\n");
                    return 0;
                }
                
                if (argc < 3)
                {
                    break;
                }
                
                partition = (UINT8) simple_strtoul(argv[2], NULL, 16);
                
                /* check partition number */
                if ((partition == 0) 
                    || (partition > (fwu.num_of_pp 
                                     + fwu.num_of_ep 
                                     + fwu.num_of_lp)))
                {
                    printf("*** partition number is not correct ***\n");
                    return -1;
                }
                
                ret = fwu_activate_partition(fwu.dev, partition, 1);
                
                if (ret) 
                {
                    printf("*** fail to activate partition ***\n");
                    return ret;
                }
                
                printf("Done!\n");
                
                return 0;
            }
            
            else if (strcmp (argv[1], "deact") == 0) 
            {
                /* ignore non-partition device */
                if (fwu.dev == FWU_DEV_SPI)
                {
                    printf("*** not support in this device ***\n");
                    return 0;
                }
                
                if (argc < 3)
                {
                    break;
                }
                
                partition = (UINT8) simple_strtoul(argv[2], NULL, 16);
                
                /* check partition number */
                if ((partition == 0) 
                    || (partition > (fwu.num_of_pp 
                                     + fwu.num_of_ep 
                                     + fwu.num_of_lp)))
                {
                    printf("*** partition number is not correct ***\n");
                    return -1;
                }
                
                ret = fwu_activate_partition(fwu.dev, partition, 0);
                
                if (ret) 
                {
                    printf("*** fail to deactivate partition ***\n");
                    return ret;
                }
                
                printf("Done!\n");
                
                return 0;
            }
            
            else if (strcmp (argv[1], "dump") == 0) 
            {
                /* ignore non-partition device */
                if (fwu.dev == FWU_DEV_SPI)
                {
                    printf("*** not support in this device ***\n");
                    return 0;
                }
                
                if (argc < 3)
                {
                    break;
                }
                
                if (argc == 3)
                {
                    /* default count is 1 */
                    blocks = 1;
                }
                else
                {
                    blocks = simple_strtoul(argv[3], NULL, 16);
                    
                }
                
                /* retrieve start block variable */
                start = simple_strtoul(argv[2], NULL, 16);
                
                ret = fwu_dump_block(fwu.dev, start, blocks);
                
                if (ret) 
                {
                    printf("*** fail to dump data ***\n");
                    return ret;
                }
                
                return 0;
            }
            
            else if (strcmp (argv[1], "read") == 0) 
            {
                /* ignore non-partition device */
                if (fwu.dev == FWU_DEV_SPI)
                {
                    printf("*** not support in this device ***\n");
                    return 0;
                }
                
                if (argc < 4)
                {
                    break;
                }
                
                /* retrieve start block variable */
                offset = simple_strtoul(argv[2], NULL, 16);
                start = simple_strtoul(argv[3], NULL, 16);
                
                if (argc == 4)
                {
                    /* default count is 1 */
                    blocks = 1;
                }
                else
                {
                    blocks = simple_strtoul(argv[4], NULL, 16);
                    
                }
                
                if (offset & 0x1FF)
                {
                    printf("Memory address is not at block boundary!\n");
                    return -1;
                }
                
                PRINTD("addr=%lx, start=%lx, blocks=%lx\n", 
                       offset, start, blocks);
                
                ret = fwu_block_read(fwu.dev, 
                                     start,
                                     blocks,
                                     (UINT8 *) offset);
                
                if (ret) 
                {
                    printf("*** fail to read block(s) ***\n");
                    return ret;
                }
                
                printf("Done!\n");
                
                return 0;
            }
            
            else if (strcmp (argv[1], "write") == 0) 
            {
                /* ignore non-partition device */
                if (fwu.dev == FWU_DEV_SPI)
                {
                    printf("*** not support in this device ***\n");
                    return 0;
                }
                
                if (argc < 4)
                {
                    break;
                }
                
                /* retrieve start block variable */
                offset = simple_strtoul(argv[2], NULL, 16);
                start = simple_strtoul(argv[3], NULL, 16);
                
                if (argc == 4)
                {
                    /* default count is 1 */
                    blocks = 1;
                }
                else
                {
                    blocks = simple_strtoul(argv[4], NULL, 16);
                    
                }
                
                if (offset & 0x1FF)
                {
                    printf("Memory address is not at block boundary!\n");
                    return -1;
                }
                
                PRINTD("addr=%lx, start=%lx, blocks=%lx\n", 
                       offset, start, blocks);
                
                ret = fwu_block_write(fwu.dev, 
                                      start,
                                      blocks,
                                      (UINT8 *) offset);
                
                if (ret) 
                {
                    printf("*** fail to write block(s) ***\n");
                    return ret;
                }
                
                printf("Done!\n");
                
                return 0;
            }
            
            else if (strcmp (argv[1], "check") == 0) 
            {
                /* retrieve start block variable */
                if (argc == 3)
                {
                    offset = simple_strtoul(argv[2], NULL, 16);
                }
                else
                {
                    /* get default offset from 'offset' variable */
                    sprintf(parameter, "%s", getenv("offset"));
                    
                    if (parameter != NULL) 
                    {
                        offset = simple_strtoul (parameter, NULL, 16);
                    }
                    else 
                    {
                        offset = CFG_LOAD_ADDR;
                    }
                }
                
                if (offset & 0x1FF)
                {
                    printf("Memory address is not at block boundary!\n");
                    return -1;
                }
                
                ret = fwu_check_image(fwu.dev, (UINT8 *) offset);
                
                if (ret) 
                {
                    printf("*** fail to check image ***\n");
                    return ret;
                }
                
                printf("Done!\n");
                
                return 0;
            }
            
            else if (strcmp (argv[1], "update") == 0) 
            {
                /* check device init status */
                if (fwu.init == 0)
                {
                    printf("*** device is not ready ***\n");
                    return 0;
                }
                
                /* retrieve start block variable */
                if (argc == 3)
                {
                    offset = simple_strtoul(argv[2], NULL, 16);
                }
                else
                {
                    /* get default offset from 'offset' variable */
                    sprintf(parameter, "%s", getenv("offset"));
                    
                    if (parameter != NULL) 
                    {
                        offset = simple_strtoul (parameter, NULL, 16);
                    }
                    else 
                    {
                        offset = CFG_LOAD_ADDR;
                    }
                }
                
                if (offset & 0x1FF)
                {
                    printf("Memory address is not at block boundary!\n");
                    return -1;
                }
                
                ret = fwu_update_image(fwu.dev, (UINT8 *) offset);
                
                if (ret) 
                {
                    printf("*** fail to update image ***\n");
                    return ret;
                }
                
                printf("Done!\n");
                
                return 0;
            }
            
            else if (strcmp (argv[1], "boot") == 0) 
            {
                /* check device init status */
                if (fwu.init == 0)
                {
                    printf("*** device is not ready ***\n");
                    return 0;
                }
                
                ret = fwu_boot_image(fwu.dev);
                
                if (ret) 
                {
                    printf("*** fail to boot image ***\n");
                    return ret;
                }
                
                printf("Done!\n");
                
                return 0;
            }
            
            else if (strcmp (argv[1], "cal") == 0) 
            {
                if (argc < 3)
                {
                    break;
                }
                
                bytes = simple_strtoul(argv[2], NULL, 16);
                
                printf("0x%lx bytes are equal to 0x%lx blocks = 0x%lx blocks + 0x%lx bytes\n\n", 
                       bytes,
                       (bytes & 0x1FF) ? (bytes >> 9) + 1 : bytes >> 9 ,
                       bytes >> 9,
                       bytes & 0x1FF);
                
                return 0;
            }
            
            break;
            
        default:
            break;
    }
    
    /* the command is not correct */
    printf("Usage:\n%s\n", cmdtp->usage);
    
    return 1;
}


U_BOOT_CMD(
    fwu, 6, 1, do_fwu,
    "fwu     - firmware upgrade sub-system, type 'help fwu' for details\n",
    "info   - show available partitions and images\n"
    "fwu dev [no]\n"
    "           - list available devices or set device 'no' \n"
    "fwu init   - parse MBR/EBR to retrieve partition and image information\n"
    "fwu fdisk default|add|del|list [p|e|l] [size] [type]\n"
    "           - create default partitions or add/delete manually,\n"
    "             'size' in MB (size = 0 = the rest of spaces of card),\n"
    "             'fwu fdisk list' to get 'type' list\n"
    "fwu erase partition (not implement)\n"
    "           - erase entire data of selected 'partition'\n"
    "fwu dump start [count]\n"
    "           - dump 'count' block(s) starting at 'start' block\n"
    "fwu read addr start [count]\n"
    "fwu write addr start [count]\n"
    "           - read/write `count' block(s) starting at 'start' block\n"
    "             to/from memory address `addr'\n"
    "fwu cal size\n"
    "           - calculate how many blocks are equal to 'size' bytes\n"
    "fwu act partition\n"
    "           - activate the 'partition' and deactivate others in MBR\n"
    "fwu deact partition\n"
    "           - deactivate the 'partition'\n"
    "fwu check [addr]\n"
    "           - check image at memory address 'addr'\n"
    "fwu update [addr]\n"
    "           - update flash by the images at memory address 'addr'\n"
    "fwu boot   - load image(s) and then boot\n"
);
