/*
 * $RCSfile$
 * $Revision$
 * $Date$
 * $Author$
 *
 * WPCM450 persistent storage driver.
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

#include "wpcm450_ps.h"
#include "wpcm450_platform.h"

#include "malloc.h"
#include "version.h"


#if 0
#define PRINTD(fmt,args...) printf ("PS: " fmt ,##args)
#else
#define PRINTD(fmt,args...)
#endif

#if 0
#define PS_DEBUG
#endif


#ifndef CONFIG_WPCM450_PLATFORM_ID
#define CONFIG_WPCM450_PLATFORM_ID  ""
#endif


#define PS_SECTOR_START_ADDR    (CFG_PS_ADDR & ~(CFG_PS_SECT_SIZE - 1))
#define PS_SECTOR_END_ADDR      (PS_SECTOR_START_ADDR + CFG_PS_SECT_SIZE -1)
#define PS_OFFSET_IN_SECTOR     (CFG_PS_OFFSET & (CFG_PS_SECT_SIZE - 1))


/* platform ID */
extern UINT8 platform_id;

extern int flash_write (char *src, ulong addr, ulong cnt);
extern int flash_sect_erase (ulong addr_first, ulong addr_last);
extern int flash_sect_protect (int p, ulong addr_first, ulong addr_last);


static unsigned ps_bcd2bin(uchar n)
{
    return ((((n >> 4) & 0x0F) * 10) + (n & 0x0F));
}


static unsigned char ps_bin2bcd(unsigned int n)
{
    return (((n / 10) << 4) | (n % 10));
}


int ps_create(UINT8 type)
{
    char *ram_start;
    ps_info_type *ram_ps;
    ps_info_type *ps;
    
    /* mac comparison */
    char *start;
    char *end;
    UINT8 i;
    char parameter[32];
    
    PRINTD("ps_create\n");
    
    /* get the start address of persistent storage */
    ps = (ps_info_type *) CFG_PS_ADDR;
    
    if ((ram_start = malloc(CFG_PS_SECT_SIZE)) == NULL) 
    {
        printf("*** unable to allocate memory ***\n");
        return -1;
    }
    
    /* copy data from SPI flash to RAM */
    memcpy((void *) ram_start, 
           (const void *) PS_SECTOR_START_ADDR, 
           CFG_PS_SECT_SIZE);
    
    /* pointer to RAM space */
    ram_ps = (ps_info_type *) (ram_start + PS_OFFSET_IN_SECTOR);
    
    PRINTD("ps=%p\n", ps);
    PRINTD("ram_start=%p\n", ram_start);
    PRINTD("ram_ps=%p\n", ram_ps);
    
    /* clear memory */
    memset(ram_ps, 0xFF, CFG_PS_SIZE);
    
    /* set magic number */
    ram_ps->magic_number[0] = 0x41;
    ram_ps->magic_number[1] = 0x56;
    ram_ps->magic_number[2] = 0x43;
    ram_ps->magic_number[3] = 0x54;
    
    /* set version */
    ram_ps->ps_ver = PS_CURRENT_VERSION;
    
    /* set MAC0 */
    if (type == PS_CREATE_UPDATE)
    {
        ram_ps->mac0[0] = ps->mac0[0];
        ram_ps->mac0[1] = ps->mac0[1];
        ram_ps->mac0[2] = ps->mac0[2];
        ram_ps->mac0[3] = ps->mac0[3];
        ram_ps->mac0[4] = ps->mac0[4];
        ram_ps->mac0[5] = ps->mac0[5];
    }
    
    /* get default MAC address from environment variable */
    sprintf(parameter, "%s", getenv("ethaddr"));
    
    start = &parameter[0];
    
    /* set MAC1 */
    for (i = 0; i < 6; ++i) 
    {
        ram_ps->mac1[i] = start ? simple_strtoul(start, &end, 16) : 0;
        
        if (start) start = (*end) ? end+1 : end;
    }
    
    /* set U-Boot version */
    ram_ps->uboot_ver[0] 
    = ps_bin2bcd(simple_strtoul(CONFIG_UBOOT_VERSION, NULL, 10));
    ram_ps->uboot_ver[1] 
    = ps_bin2bcd(simple_strtoul(CONFIG_UBOOT_PATCHLEVEL, NULL, 10));
    ram_ps->uboot_ver[2] 
    = ps_bin2bcd(simple_strtoul(CONFIG_UBOOT_SUBLEVEL, NULL, 10));
    
    ram_ps->uboot_ver[4] 
    = ps_bin2bcd(simple_strtoul(CONFIG_AVCT_VERSION, NULL, 10));
    ram_ps->uboot_ver[5] 
    = ps_bin2bcd(simple_strtoul(CONFIG_AVCT_PATCHLEVEL, NULL, 10));
    ram_ps->uboot_ver[6] 
    = ps_bin2bcd(simple_strtoul(CONFIG_AVCT_SUBLEVEL, NULL, 10));
    
    /* set platform ID */
    if (sizeof(CONFIG_WPCM450_PLATFORM_ID) > 1)
    {
        strncpy((char *) &ram_ps->platform_id[0], 
                CONFIG_WPCM450_PLATFORM_ID, 4);
    }
    
    /* un-protect sector */
    flash_sect_protect(0, PS_SECTOR_START_ADDR, PS_SECTOR_END_ADDR);
    
    PRINTD("flash_sect_erase\n");
    
    PRINTD("PS_SECTOR_START_ADDR=%lx, PS_SECTOR_END_ADDR=%lx\n", 
           PS_SECTOR_START_ADDR, PS_SECTOR_END_ADDR);
    
    /* erase sector */
    flash_sect_erase(PS_SECTOR_START_ADDR, PS_SECTOR_END_ADDR);
    
    PRINTD("flash_write\n");
    
    PRINTD("s=%p, d=%lx, size=%lx\n", 
           ram_start, PS_SECTOR_START_ADDR, CFG_PS_SECT_SIZE);
    
    /* write back data from RAM */
    flash_write(ram_start, PS_SECTOR_START_ADDR, CFG_PS_SECT_SIZE);
    
    /* re-protect sector */
    flash_sect_protect(1, PS_SECTOR_START_ADDR, PS_SECTOR_END_ADDR);
    
    /* free allocated memory */
    free(ram_start);
    
    return 0;
}


int ps_update(void)
{
    ps_info_type *ps;
    char parameter[32];
    int ret;
    
    PRINTD("ps_update\n");
    
    /* get the start address of persistent storage */
    ps = (ps_info_type *) CFG_PS_ADDR;
    
    PRINTD("update ethaddr\n");
    
    /* update environment variable */
    sprintf(parameter, "%02x:%02x:%02x:%02x:%02x:%02x",
            ps->mac1[0],
            ps->mac1[1],
            ps->mac1[2],
            ps->mac1[3],
            ps->mac1[4],
            ps->mac1[5]);
    
    /* update internal variable */
    setenv("ethaddr", parameter);
    
    /* check U-Boot version and platform ID */
    if ((ps->ps_ver != PS_CURRENT_VERSION)
        || (ps->uboot_ver[0] 
            != ps_bin2bcd(simple_strtoul(CONFIG_UBOOT_VERSION, NULL, 10)))
        || (ps->uboot_ver[1] 
            != ps_bin2bcd(simple_strtoul(CONFIG_UBOOT_PATCHLEVEL, NULL, 10)))
        || (ps->uboot_ver[2] 
            != ps_bin2bcd(simple_strtoul(CONFIG_UBOOT_SUBLEVEL, NULL, 10)))
        || (ps->uboot_ver[4] 
            != ps_bin2bcd(simple_strtoul(CONFIG_AVCT_VERSION, NULL, 10)))
        || (ps->uboot_ver[5] 
            != ps_bin2bcd(simple_strtoul(CONFIG_AVCT_PATCHLEVEL, NULL, 10)))
        || (ps->uboot_ver[6] 
            != ps_bin2bcd(simple_strtoul(CONFIG_AVCT_SUBLEVEL, NULL, 10)))
        || ((strncmp((char *) &ps->platform_id[0], 
                    CONFIG_WPCM450_PLATFORM_ID, 4)) 
            && (sizeof(CONFIG_WPCM450_PLATFORM_ID) > 1)))
    {
        printf("Updating persistent storage...\n");
        
        ret = ps_create(PS_CREATE_UPDATE);
        
        if (ret)
        {
            printf("*** fail to update U-Boot version ***\n");
            return ret;
        }
    }
    
    return 0;
}


int ps_check(void)
{
    ps_info_type *ps;
    int ret;
    
    PRINTD("ps_check\n");
    
    /* get the start address of persistent storage */
    ps = (ps_info_type *) CFG_PS_ADDR;
    
    PRINTD("ps_check, ps=%p\n", ps);
    
    PRINTD("magic_number[0-3]=%x %x %x %x, ps_ver=%d\n",
           ps->magic_number[0],
           ps->magic_number[1],
           ps->magic_number[2],
           ps->magic_number[3],
           ps->ps_ver);
    
    if (strncmp((char *) &ps->magic_number[0], "AVCT", 4))
    {
        PRINTD("create default persistent storage\n");
        ret = ps_create(PS_CREATE_NEW);
    }
    else
    {
        PRINTD("update environment variables\n");
        ret = ps_update();
    }
    
    /* Check Failsafe Lockout byte.
       Mask off the upper 7 bits and look for the flag value indicating power-up should be blocked.
       This will allow the iDRAC FW time to initialize and restore error messages to the LCD.
       This is currently only supported on McCave. */
    if (PS_FAILSAFE_LOCKOUT == (ps->u8FailsafeFlag & PS_FAILSAFE_LOCKOUT_MASK))
    {
#ifdef CONFIG_WPCM450_WHOVILLE
        if (PF_ID_MCCAVE == platform_id)
        {
            /* Set bit 1 at offset byte 27 to disable the system power on
               The base address of the CPLD memory map IO is 0xC4000000 */
            PRINTD("Failsafe Lockout detected - setting bit 1 at CPLD offset 0x27 to prevent power-up\n");
            *((UINT8 *) 0xC4000027) |= 0x02;
        }
#endif
    }
    
    return ret;
}


/* only support persistent storage in the first bank of flash */
int ps_set(UINT8 number, char *arg[])
{
    char *ram_start;
    ps_info_type *ram_ps;
    ps_info_type *ps;
    
    /* mac comparison */
    char *start;
    char *end;
    UINT8 i;
    
    PRINTD("ps_set\n");
    
    /* get the start address of persistent storage */
    ps = (ps_info_type *) CFG_PS_ADDR;
    
    if (strncmp((char *) &ps->magic_number[0], "AVCT", 4))
    {
        /* no Avocent signature found */
        printf("*** no Avocent signature found ***\n");
        
        return -1;
    }
    
    if (ps->ps_ver > PS_CURRENT_VERSION)
    {
        printf("*** unknown persistent storage version ***\n");
        return -1;
    }
    
    if ((ram_start = malloc(CFG_PS_SECT_SIZE)) == NULL) 
    {
        printf("*** unable to allocate memory ***\n");
        return -1;
    }
    
    /* copy data from SPI flash to RAM */
    memcpy((void *) ram_start, 
           (const void *) PS_SECTOR_START_ADDR, 
           CFG_PS_SECT_SIZE);
    
    /* pointer to RAM space */
    ram_ps = (ps_info_type *) (ram_start + PS_OFFSET_IN_SECTOR);
    
    PRINTD("ram_start=%p\n", ram_start);
    PRINTD("ram_ps=%p\n", ram_ps);
    
    switch (number)
    {
        case PS_REC_MAC0:
            
            start = arg[0];
            
            PRINTD("start=%p\n", start);
            
            for (i = 0; i < 6; ++i) 
            {
                ram_ps->mac0[i] = start ? simple_strtoul(start, &end, 16) : 0;
                
                if (start) start = (*end) ? end+1 : end;
            }
            
            break;
            
        case PS_REC_MAC1:
            
            start = arg[0];
            
            PRINTD("start=%p\n", start);
            
            for (i = 0; i < 6; ++i) 
            {
                ram_ps->mac1[i] = start ? simple_strtoul(start, &end, 16) : 0;
                
                if (start) start = (*end) ? end+1 : end;
            }
            
            /* update environment variable */
            setenv("ethaddr", *arg);
            
            break;
            
        case PS_REC_VERSION:
            
            ram_ps->uboot_ver[0] 
            = ps_bin2bcd(simple_strtoul(CONFIG_UBOOT_VERSION, NULL, 10));
            ram_ps->uboot_ver[1] 
            = ps_bin2bcd(simple_strtoul(CONFIG_UBOOT_PATCHLEVEL, NULL, 10));
            ram_ps->uboot_ver[2] 
            = ps_bin2bcd(simple_strtoul(CONFIG_UBOOT_SUBLEVEL, NULL, 10));
            
            ram_ps->uboot_ver[4] 
            = ps_bin2bcd(simple_strtoul(CONFIG_AVCT_VERSION, NULL, 10));
            ram_ps->uboot_ver[5] 
            = ps_bin2bcd(simple_strtoul(CONFIG_AVCT_PATCHLEVEL, NULL, 10));
            ram_ps->uboot_ver[6] 
            = ps_bin2bcd(simple_strtoul(CONFIG_AVCT_SUBLEVEL, NULL, 10));
            
            printf("%2d. U-Boot Version: Maj. %d.%d.%d  Min. %d.%d.%d\n",
                   PS_REC_VERSION,
                   ps_bcd2bin(ps->uboot_ver[0]), 
                   ps_bcd2bin(ps->uboot_ver[1]), 
                   ps_bcd2bin(ps->uboot_ver[2]), 
                   ps_bcd2bin(ps->uboot_ver[4]), 
                   ps_bcd2bin(ps->uboot_ver[5]), 
                   ps_bcd2bin(ps->uboot_ver[6]));
            
            break;
            
        /* failsafe lockout */
        case PS_REC_FAILSAFE:
            
            start = arg[0];
            
            PRINTD("start=%p\n", start);
            
            ram_ps->u8FailsafeFlag = start ? simple_strtoul(start, &end, 16) : 0;
            
            break;
            
        default:
            break;
    }
    
    /* un-protect sector */
    flash_sect_protect(0, PS_SECTOR_START_ADDR, PS_SECTOR_END_ADDR);
    
    PRINTD("erase start addr=PS_SECTOR_START_ADDR=%lx\n", PS_SECTOR_START_ADDR);
    
    PRINTD("erase end addr=PS_SECTOR_END_ADDR=%lx\n", PS_SECTOR_END_ADDR);
    
    /* erase sector */
    flash_sect_erase(PS_SECTOR_START_ADDR, PS_SECTOR_END_ADDR);
    
    /* write back data from RAM */
    flash_write(ram_start, PS_SECTOR_START_ADDR, CFG_PS_SECT_SIZE);
    
    /* re-protect sector */
    flash_sect_protect(1, PS_SECTOR_START_ADDR, PS_SECTOR_END_ADDR);
    
    /* free allocated memory */
    free(ram_start);
    
    return 0;
}


int ps_erase(UINT8 type)
{
    char *ram_start;
    ps_info_type *ps;
    
    PRINTD("ps_erase\n");
    
    /* get the start address of persistent storage */
    ps = (ps_info_type *) CFG_PS_ADDR;
    
    if ((ram_start = malloc(CFG_PS_SECT_SIZE)) == NULL) 
    {
        printf("*** unable to allocate memory ***\n");
        return -1;
    }
    
    /* copy data from SPI flash to RAM */
    memcpy((void *) ram_start, 
           (const void *) PS_SECTOR_START_ADDR, 
           CFG_PS_SECT_SIZE);
    
    /* clear memory */
    switch (type)
    {
        case PS_ERASE_ENV:
            
            PRINTD("PS_ERASE_ENV\n");
            
            if (CFG_ENV_ADDR == PS_SECTOR_START_ADDR)
            {
                memset(ram_start, 0xFF, CFG_ENV_SIZE);
            }
            else
            {
                printf("*** env is not in the same sector ***\n");
                
                /* free allocated memory */
                free(ram_start);
                
                return -1;
            }
            
            break;
            
        case PS_ERASE_REC:
            
            PRINTD("PS_ERASE_REC\n");
            
            memset(ram_start + PS_OFFSET_IN_SECTOR, 0xFF, CFG_PS_SIZE);
            
            break;
    }
    
    /* un-protect sector */
    flash_sect_protect(0, PS_SECTOR_START_ADDR, PS_SECTOR_END_ADDR);
    
    /* erase sector */
    flash_sect_erase(PS_SECTOR_START_ADDR, PS_SECTOR_END_ADDR);
    
    /* write back data from RAM */
    flash_write(ram_start, PS_SECTOR_START_ADDR, CFG_PS_SECT_SIZE);
    
    /* re-protect sector */
    flash_sect_protect(1, PS_SECTOR_START_ADDR, PS_SECTOR_END_ADDR);
    
    /* free allocated memory */
    free(ram_start);
    
    return 0;
}


int ps_list(void)
{
    ps_info_type *ps;
    
    PRINTD("ps_list\n");
    
    /* get the start address of persistent storage */
    ps = (ps_info_type *) CFG_PS_ADDR;
    
    /* check the consistence of persistent storage area */
    if (ps_check())
    {
        PRINTD("ps_check failed\n");
        return -1;
    }
    
    PRINTD("ps_list ps=%p\n", ps);
    
    printf("No.  Name                      Value\n");
    printf("------------------------------------------------------\n");
    
    printf("%2d   MAC0                      %02x:%02x:%02x:%02x:%02x:%02x\n",
           PS_REC_MAC0,
           ps->mac0[0], ps->mac0[1], ps->mac0[2], 
           ps->mac0[3], ps->mac0[4], ps->mac0[5]);
    
    printf("%2d   MAC1                      %02x:%02x:%02x:%02x:%02x:%02x\n",
           PS_REC_MAC1,
           ps->mac1[0], ps->mac1[1], ps->mac1[2], 
           ps->mac1[3], ps->mac1[4], ps->mac1[5]);
    
    printf("%2d   U-Boot Ver                Maj %d.%d.%d  Min %d.%d.%d\n",
           PS_REC_VERSION,
           ps_bcd2bin(ps->uboot_ver[0]), 
           ps_bcd2bin(ps->uboot_ver[1]), 
           ps_bcd2bin(ps->uboot_ver[2]), 
           ps_bcd2bin(ps->uboot_ver[4]), 
           ps_bcd2bin(ps->uboot_ver[5]), 
           ps_bcd2bin(ps->uboot_ver[6]));
    
    printf("%2d   Platform ID               %c%c%c%c\n",
           PS_REC_PLATFORM,
           (char) ps->platform_id[0],
           (char) ps->platform_id[1],
           (char) ps->platform_id[2],
           (char) ps->platform_id[3]);
    
    /* failsafe lockout */
    printf("%2d   Failsafe Lockout Flag     %02x\n",
           PS_REC_FAILSAFE,
           ps->u8FailsafeFlag);
    printf("\n");
    
    return 0;
}


int do_ps(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
    int ret;
    UINT8 number;
    
    PRINTD("argc=%d\n", argc);
    
    switch (argc) 
    {
        case 0:
        case 1:
            break;
            
        case 2:
            if (strcmp(argv[1], "list") == 0) 
            {
                ps_list();
                
                return 0;
            }
            
            break;
            
        case 3:
            if (strcmp(argv[1], "erase") == 0) 
            {
                if (strcmp(argv[2], "env") == 0) 
                {
                    number = PS_ERASE_ENV;
                }
                else if (strcmp(argv[2], "rec") == 0) 
                {
                    number = PS_ERASE_REC;
                }
                else
                {
                    break;
                }
                
                ret = ps_erase(number);
                
                if (ret) 
                {
                    printf ("*** fail to erase ***\n");
                    return ret;
                }
                
                return 0;
            }
            
            break;
            
        case 4:
            if (strcmp(argv[1], "set") == 0) 
            {
                number = (UINT8) simple_strtoul(argv[2], NULL, 10);
                
                if (!((number == PS_REC_MAC0) || (number == PS_REC_MAC1) || (number == PS_REC_FAILSAFE)) )
                {
                    printf("*** can not modify No %d ***\n", number);
                    return -1;
                }
                
                ret = ps_set(number, &argv[3]);
                
                if (ret) 
                {
                    printf ("*** fail to set data ***\n");
                    return ret;
                }
                
                /* list data after setting */
                ps_list();
                
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
    ps, 4, 1, do_ps,
    "ps      - persistent storage, type 'help ps' for details\n",
    "list   - show all records storaged in persistent storage\n"
    "ps set number value\n"
    "           - set record 'number' to 'value'\n"
    "ps erase env|rec\n"
    "           - erase U-Boot environment 'env' or records 'rec'\n"
);
