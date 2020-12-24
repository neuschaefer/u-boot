/*
 * $RCSfile$
 * $Revision$
 * $Date$
 * $Author$
 *
 * WPCM450 SPI flash driver.
 *
 * Copyright (C) 2007 Avocent Corp.
 *
 * This file is subject to the terms and conditions of the GNU 
 * General Public License. This program is distributed in the hope 
 * that it will be useful, but WITHOUT ANY WARRANTY; without even 
 * the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
 * PURPOSE.  See the GNU General Public License for more details.
 */


#include "cdefs.h"
#include "com_defs.h"
#include "common.h"
#include "config.h"
#include "spi.h"
#include "flash.h"
#include "asm/sizes.h"
#include "wpcm450_gctrl_regs.h"
#include "wpcm450_fiu_regs.h"


#if defined(CONFIG_WPCM450_SPI_DRIVER)


#if 0
#define PRINTD(fmt,args...) printf ("SPI: " fmt ,##args)
#else
#define PRINTD(fmt,args...)
#endif

#if 0
#define DEBUG_SPI
#endif



/************************************************************************/
/*                                                                      */
/************************************************************************/
#define SPI_READ_JEDEC_ID_CMD       0x9F
#define SPI_WRITE_ENABLE_CMD        0x06
#define SPI_WRITE_DISABLE_CMD       0x04
#define SPI_READ_STATUS_REG_CMD     0x05
#define SPI_WRITE_STATUS_REG_CMD    0x01
#define SPI_READ_DATA_CMD           0x03
#define SPI_PAGE_PRGM_CMD           0x02
#define SPI_SECTOR_ERASE_CMD        0xD8
#define SPI_READ_PID_CMD            0x90
#define SPI_SEQU_PRGM_CMD           0xAD


/************************************************************************/
/*                                                                      */
/************************************************************************/
#define DEFAULT_MF_ID               0xFF
#define DEFAULT_CAPACITY_ID         0xFF


/************************************************************************/
/* Winbond                                                              */
/************************************************************************/
#define WINBOND_MF_ID               0xEF
#define W25P80_CAPACITY_ID          0x14
#define W25P16_CAPACITY_ID          0x15
#define W25P32_CAPACITY_ID          0x16
#define W25Q128_CAPACITY_ID         0x18


/************************************************************************/
/* STMicro - STMicroelectronics                                         */
/************************************************************************/
#define STMICRO_MF_ID               0x20
#define M25P80_CAPACITY_ID          0x14
#define M25P16_CAPACITY_ID          0x15
#define M25P32_CAPACITY_ID          0x16
#define M25P64_CAPACITY_ID          0x17


/************************************************************************/
/* Atmel                                                                */
/************************************************************************/
#define ATMEL_MF_ID                 0x1F
#define AT26DF_DEV_ID               0x40
#define AT26DF08_CAPACITY_ID        0x05
#define AT26DF16_CAPACITY_ID        0x06
#define AT26DF32_CAPACITY_ID        0x07


/************************************************************************/
/* SST - Silicon Storage Technology                                     */
/************************************************************************/
#define SST_MF_ID                   0xBF
#define SST25VF080B_CAPACITY_ID     0x8E
#define SST25VF016B_CAPACITY_ID     0x41
#define SST25VF032B_CAPACITY_ID     0x4A


/************************************************************************/
/* MXIC - Macronix International Co., Ltd.                              */
/************************************************************************/
#define MXIC_MF_ID                  0xC2
#define MX25L8005_CAPACITY_ID       0x14
#define MX25L1605_CAPACITY_ID       0x15
#define MX25L3205_CAPACITY_ID       0x16
#define MX25L6405_CAPACITY_ID       0x17
#define MX25L12805_CAPACITY_ID      0x18


/************************************************************************/
/* Spansion                                                             */
/************************************************************************/
#define SPANSION_MF_ID              0x01
#define S25FL008A_CAPACITY_ID       0x13
#define S25FL016A_CAPACITY_ID       0x14
#define S25FL032A_CAPACITY_ID       0x15
#define S25FL064A_CAPACITY_ID       0x16
#define S25FL128P_CAPACITY_ID       0x18

/************************************************************************/
/*                                                                      */
/************************************************************************/
#define SHIFT_256                   8
#define SHIFT_512                   9
#define SHIFT_1024                  10
#define WIN_LIMIT_4K_SHIFT          12


/************************************************************************/
/*                                                                      */
/************************************************************************/
typedef struct flash_t
{
    UINT32 mf_id;
    UINT32 capacity;
    char*  name;
    UINT32 chip_size;
    UINT32 block_size;
}flash_t;


/*******************************************************************
* Typedef Definitions
*******************************************************************/
typedef enum _spi_w_burst_t {
  SPI_ONE_BYTE_W_BURST = 0,
  SPI_FOUR_BYTE_W_BURST = 2
} SPI_w_burst_t;

typedef enum _spi_r_burst_t {
  SPI_ONE_BYTE_R_BURST = 0,
  SPI_SIXTEEN_BYTE_R_BURST = 3
} SPI_r_burst_t;

typedef enum _spi_w_protect_int_t {
  SPI_W_PROTECT_INT_DISABLE = 0,
  SPI_W_PROTECT_INT_ENABLE = 1
} SPI_w_protect_int_t;

typedef enum _spi_incorect_access_int_t {
  SPI_INCORECT_ACCESS_INT_DISABLE = 0,
  SPI_INCORECT_ACCESS_INT_ENABLE = 1
} SPI_incorect_access_int_t;

typedef enum _spi_fast_read_t {
  SPI_FAST_READ_DISABLE = 0,
  SPI_FAST_READ_ENABLE = 1
} SPI_fast_read_t;


typedef enum {
  DEFAULT_FLASH,
  W25P80,
  W25P16,
  W25P32,
  W25Q128,
  M25P80,
  M25P16,
  M25P32,
  M25P64,
  AT26DF08,
  AT26DF16,
  AT26DF32,
  SST25VF080B,
  SST25VF016B,
  SST25VF032B,
  MX25L8005,
  MX25L1605,
  MX25L3205,
  MX25L6405,
  MX25L12805,
  S25FL008A,
  S25FL016A,
  S25FL032A,
  S25FL064A,
  S25FL128P
}FIU_FlashType_t;


/*******************************************************************
* Global variables
*******************************************************************/

static flash_t flash_spi[]={
  {DEFAULT_MF_ID,   DEFAULT_CAPACITY_ID,     "DEFAULT",     SZ_2M,  SZ_64K},
  {WINBOND_MF_ID,   W25P80_CAPACITY_ID,      "W25P80",      SZ_1M,  SZ_64K},
  {WINBOND_MF_ID,   W25P16_CAPACITY_ID,      "W25P16",      SZ_2M,  SZ_64K},
  {WINBOND_MF_ID,   W25P32_CAPACITY_ID,      "W25P32",      SZ_4M,  SZ_64K},
  {WINBOND_MF_ID,   W25Q128_CAPACITY_ID,     "W25Q128",     SZ_16M, SZ_64K},
  {STMICRO_MF_ID,   M25P80_CAPACITY_ID,      "M25P80",      SZ_1M,  SZ_64K},
  {STMICRO_MF_ID,   M25P16_CAPACITY_ID,      "M25P16",      SZ_2M,  SZ_64K},
  {STMICRO_MF_ID,   M25P32_CAPACITY_ID,      "M25P32",      SZ_4M,  SZ_64K},
  {STMICRO_MF_ID,   M25P64_CAPACITY_ID,      "M25P64",      SZ_8M,  SZ_64K},
  {ATMEL_MF_ID,     AT26DF08_CAPACITY_ID,    "AT26DF08",    SZ_1M,  SZ_64K},
  {ATMEL_MF_ID,     AT26DF16_CAPACITY_ID,    "AT26DF16",    SZ_2M,  SZ_64K},
  {ATMEL_MF_ID,     AT26DF32_CAPACITY_ID,    "AT26DF32",    SZ_4M,  SZ_64K},
  {SST_MF_ID,       SST25VF080B_CAPACITY_ID, "SST25VF080B", SZ_1M,  SZ_64K},
  {SST_MF_ID,       SST25VF016B_CAPACITY_ID, "SST25VF016B", SZ_2M,  SZ_64K},
  {SST_MF_ID,       SST25VF032B_CAPACITY_ID, "SST25VF032B", SZ_4M,  SZ_64K},
  {MXIC_MF_ID,      MX25L8005_CAPACITY_ID,   "MX25L8005",   SZ_1M,  SZ_64K},
  {MXIC_MF_ID,      MX25L1605_CAPACITY_ID,   "MX25L1605",   SZ_2M,  SZ_64K},
  {MXIC_MF_ID,      MX25L3205_CAPACITY_ID,   "MX25L3205",   SZ_4M,  SZ_64K},
  {MXIC_MF_ID,      MX25L6405_CAPACITY_ID,   "MX25L6405",   SZ_8M,  SZ_64K},
  {MXIC_MF_ID,      MX25L12805_CAPACITY_ID,  "MX25L12805",  SZ_16M, SZ_64K},
  {SPANSION_MF_ID,  S25FL008A_CAPACITY_ID,   "S25FL008A",   SZ_1M,  SZ_64K},
  {SPANSION_MF_ID,  S25FL016A_CAPACITY_ID,   "S25FL016A",   SZ_2M,  SZ_64K},
  {SPANSION_MF_ID,  S25FL032A_CAPACITY_ID,   "S25FL032A",   SZ_4M,  SZ_64K},
  {SPANSION_MF_ID,  S25FL064A_CAPACITY_ID,   "S25FL064A",   SZ_8M,  SZ_64K},
  {SPANSION_MF_ID,  S25FL128P_CAPACITY_ID,   "S25FL128P",   SZ_16M, SZ_64K},
  {0}
};


UINT32 fiu_flash_type_idx = (UINT32) DEFAULT_FLASH;


static UINT32 max_dev_size;
static UINT32 total_size;

flash_info_t flash_info[CFG_MAX_FLASH_BANKS];
static ulong bank_base[CFG_MAX_FLASH_BANKS] = CFG_FLASH_BANKS_LIST;

/************************************************************************/
/* Definitions                                                          */
/************************************************************************/
void fiu_config(int num_device,
                int max_dev_size,
                int total_flash_size);

BOOL fiu_set_write_window(int win_num,
                          UINT32 low_limit,
                          UINT32 high_limit,
                          BOOL lock);

INT32 fiu_read_pid(UINT32 dev_num,
                   UCHAR *pid0,
                   UCHAR *pid1);

void fiu_uma_read(int device,
                  UINT8 transaction_code,
                  UINT32 address,
                  BOOL address_size,
                  UINT8 * data,
                  int data_size);

void fiu_uma_write(int device,
                   UINT8 transaction_code,
                   UINT32 address,
                   BOOL address_size,
                   UINT8 * data,
                   int data_size);

void fiu_uma_sequ_write(int device,
                        UINT32 address,
                        UINT8 *data,
                        UINT32 data_size,
                        UINT8 bytes_per_cycle);

void fiu_uma_page_write(int device,
                        UINT32 address,
                        UINT8 * data,
                        int data_size);

void fiu_wait_for_ready(UINT8 device);

void fiu_write_protect(int dev, UINT8 enable);


/*=====================================================================*/
/*                         Public Functions                            */
/*=====================================================================*/


unsigned long flash_init (void)
{
    spi_init();
    
    return total_size;
}


void flash_print_info (flash_info_t *flash_info_ptr)
{
    int sect_idx;
    
    /* PRINTD("flash_print_info\n"); */
    
    if (flash_info_ptr->size >= (1 << 20)) 
    {
        sect_idx = 20;
    }
    else 
    {
        sect_idx = 10;
    }
    
    printf ("  Size: %ld %cB in %d Sectors  (",
            flash_info_ptr->size >> sect_idx,
            (sect_idx == 20) ? 'M' : 'k',
            flash_info_ptr->sector_count);
    
    switch(flash_spi[fiu_flash_type_idx].mf_id)
    {
        case WINBOND_MF_ID:
            printf("Winbond ");
            break;
        case STMICRO_MF_ID:
            printf("STMicro ");
            break;
        case ATMEL_MF_ID:
            printf("ATMEL ");
            break;
        case SST_MF_ID:
            printf("SST ");
            break;
        case MXIC_MF_ID:
            printf("MXIC ");
            break;
        case SPANSION_MF_ID:
            printf("Spansion ");
            break;
        default:
            printf("Unknown Vendor ");
            break;
    }
    
    printf("%s)\n", flash_spi[fiu_flash_type_idx].name);
    
    printf ("  Sector Start Addresses:");
    
    for (sect_idx = 0; sect_idx < flash_info_ptr->sector_count; ++sect_idx) 
    {
        if ((sect_idx % 5) == 0)
            printf ("\n   ");
        
        printf (" %08lX%s",
                flash_info_ptr->start[sect_idx],
                flash_info_ptr->protect[sect_idx] ? " (RO)" : "     ");
    }
    
    printf ("\n");
    
    return;
}


int flash_erase (flash_info_t * flash_info_ptr, int s_first, int s_last)
{
    int sect_idx;
    int dev;
    int divisor;
    
    /* setup divisor for printing erasing progress */
    divisor = s_last - s_first + 1;
    
    dev = (flash_info_ptr->start[s_first] - CFG_FLASH_BASE)/flash_info_ptr->size;
    
    printf("\nErasing Bank %d: ", dev);
    
    PRINTD("s_first=%x, s_last=%x\n", s_first, s_last);
    
    printf("  0\%");
    
    /* check if flash is busy */
    fiu_wait_for_ready(dev);
    
    PRINTD("device is not busy\n");
    
    for (sect_idx = s_first; sect_idx <= s_last; sect_idx++)
    {
        fiu_uma_write(dev,
                      SPI_WRITE_ENABLE_CMD,
                      0,
                      FALSE,
                      NULL,
                      0);
        
        fiu_wait_for_ready(dev);
        
        fiu_uma_write(dev,
                      SPI_SECTOR_ERASE_CMD,
                      flash_info_ptr->start[sect_idx],
                      TRUE,
                      NULL,
                      0);
        
        printf("\b\b\b\b%3d\%", ((sect_idx - s_first + 1) * 100 / divisor));
        
        fiu_wait_for_ready(dev);
    }
    
    printf("\b\b\b\b100\%\n");
    
    return 0;
}


int write_buff (flash_info_t *flash_info_ptr, uchar *src, ulong addr, ulong cnt)
{
    int dev;
    UINT32 align_size;
    UINT32 rest_size;
    UINT32 w_addr;
    UINT32 w_bytes = 0;
    int divisor;
    
    /* setup divisor for printing erasing progress */
    divisor = cnt;
    
    /* get flash bank information */
    dev = (addr - CFG_FLASH_BASE)/flash_info_ptr->size;
    
    printf("\nWriting Bank %d: ", dev);
    
    PRINTD("s=%p, d=%lx, size=%lx\n", src, addr, cnt);
    
    printf("  0\%");
    
    /* check if flash is busy */
    fiu_wait_for_ready(dev);
    
    PRINTD("device is not busy\n");
    
    /* align to even address */
    if (addr & 0x01)
    {
        fiu_uma_write(dev,
                      SPI_WRITE_ENABLE_CMD,
                      0,
                      FALSE,
                      NULL,
                      0);
        
        fiu_wait_for_ready(dev);
        
        *((volatile UINT8*)(addr))=*src;
        
        src++;
        addr++;
        cnt--;  
        
        fiu_wait_for_ready(dev);
    }
    
    /* set up destination of writing address */
    w_addr = addr;
    
    align_size = cnt & (~0x03); /* amount whole 32-bit words */
    rest_size  = cnt & 0x03;    /* amount non 32-bit rest bytes */
    
    if (align_size != 0)
    {
        if ((addr & 0x000000FF) != 0)
        {
            fiu_uma_write(dev,
                          SPI_WRITE_ENABLE_CMD,
                          0,
                          FALSE,
                          NULL,
                          0);
            
            w_bytes = 0x00000100 - (addr & 0x000000FF);
            
            if (w_bytes > align_size)
            {
                w_bytes = align_size;
            }
            
            fiu_wait_for_ready(dev);
            
            /* check to decide to use page program or sequential program */
            /* if((fiu_flash_type_idx == AT26DF08) || (fiu_flash_type_idx == SST25VF080B)) */
            if ((fiu_flash_type_idx == SST25VF080B) 
                || (fiu_flash_type_idx == SST25VF016B)
                || (fiu_flash_type_idx == SST25VF032B))
            {
                PRINTD("s\n");
                
                /* ATMEL */
                /* fiu_uma_sequ_write(dev, w_addr, (UINT8*) src, (int) w_bytes, 1); */
                
                /* SST */
                fiu_uma_sequ_write(dev, 
                                   w_addr - flash_info_ptr->start[0], 
                                   (UINT8*) src, 
                                   (int) w_bytes, 
                                   2);
            }
            else
            {
                PRINTD("p\n");
                fiu_uma_page_write(dev, 
                                   w_addr - flash_info_ptr->start[0], 
                                   (UINT8*) src, 
                                   (int) w_bytes);
            }
            
            src += w_bytes;
            w_addr += w_bytes;
            align_size -= w_bytes;
            
            printf("\b\b\b\b%3d\%", ((divisor - align_size) * 100 / divisor));
            
            fiu_wait_for_ready(dev);
        }
        
        while (align_size > 0)
        {
            fiu_uma_write(dev,
                          SPI_WRITE_ENABLE_CMD,
                          0,
                          FALSE,
                          NULL,
                          0);
            
            /* page size is 256 bytes */
            if (align_size >= 0x100)
            {
                w_bytes = 0x100;
            }
            else
            {
                w_bytes = align_size;
            }
            
            fiu_wait_for_ready(dev);
            
            /* check to deside to use page program or sequential program */
            /* if((fiu_flash_type_idx == AT26DF08) || (fiu_flash_type_idx == SST25VF080B)) */
            if ((fiu_flash_type_idx == SST25VF080B)
                || (fiu_flash_type_idx == SST25VF016B)
                || (fiu_flash_type_idx == SST25VF032B))
            {
                PRINTD("s");
                
                /* ATMEL */
                /* fiu_uma_sequ_write(dev, w_addr, (UINT8*) src, (int) w_bytes, 1); */
                
                /* SST */
                fiu_uma_sequ_write(dev, w_addr - flash_info_ptr->start[0], 
                                   (UINT8*) src, (int) w_bytes, 2);
            }
            else
            {
                PRINTD("p");
                fiu_uma_page_write(dev, w_addr - flash_info_ptr->start[0], 
                                   (UINT8*)src, (int) w_bytes);
            }
            
            src += w_bytes;
            w_addr += w_bytes;
            align_size -= w_bytes;
            
            printf("\b\b\b\b%3d\%", ((divisor - align_size) * 100 / divisor));
            
            fiu_wait_for_ready(dev);
        }
    }
    
    if (rest_size != 0)
    {
        for (; w_addr < (addr+cnt); w_addr++)
        {
            fiu_uma_write(dev,
                          SPI_WRITE_ENABLE_CMD,
                          0,
                          FALSE,
                          NULL,
                          0);
            
            fiu_wait_for_ready(dev);
            
            *((volatile UINT8*) w_addr) = *src;
            
            src++;
            
            fiu_wait_for_ready(dev);
        }
    }
    
    printf("\b\b\b\b100\%\n");
    
    return 0;
}


/*-----------------------------------------------------------------------
 * Initialization
 */
void spi_init (void)
{
    UCHAR pid0; 
    UCHAR pid1;
    int fl_idx;
    int idx;
    int dev_cnt;
    int sect_idx;
    UINT8 status_reg_val;
    
#ifdef CONFIG_WPCM450_SVB
    /* enable flashes 1-3 */
    CLEAR_BIT(MFSEL1, MF_SCS3SEL_BIT);
    SET_BIT(MFSEL1, MF_SCS2SEL_BIT);
    SET_BIT(MFSEL1, MF_SCS1SEL_BIT);
#else
#ifdef CONFIG_WPCM450_WHOVILLE
    /* disable flashes 1-3 */
    SET_BIT(MFSEL1, MF_SCS3SEL_BIT);
    CLEAR_BIT(MFSEL1, MF_SCS2SEL_BIT);
    CLEAR_BIT(MFSEL1, MF_SCS1SEL_BIT); 
#endif
#endif
    
    max_dev_size = 0;
    total_size = 0;
    dev_cnt = 0;
    
    PRINTD("Flash Init\n");
    
    for (fl_idx = 0; fl_idx < CFG_MAX_FLASH_BANKS; fl_idx++)
    {
        fiu_read_pid(fl_idx, &pid0, &pid1);
        
        idx = 0;
        
        while (flash_spi[idx].mf_id != 0)
        {
            if ((flash_spi[idx].mf_id == pid0) 
                && (flash_spi[idx].capacity == pid1))
            {
                dev_cnt++;
                
                if(max_dev_size < flash_spi[idx].chip_size)
                {
                    max_dev_size = flash_spi[idx].chip_size;
                }
                
                total_size  += flash_spi[idx].chip_size;
                flash_info[fl_idx].size = flash_spi[idx].chip_size;
                flash_info[fl_idx].flash_id = (pid1 << 16) | pid0;
                flash_info[fl_idx].sector_count 
                = flash_spi[idx].chip_size / flash_spi[idx].block_size;
                
                PRINTD("Found Bank#%d ID[0x%x] Size [%d]Sect Count=[%d]\n",
                       fl_idx,
                       flash_info[fl_idx].flash_id,
                       flash_info[fl_idx].size,
                       flash_info[fl_idx].sector_count);
                
                for (sect_idx = 0; 
                     sect_idx < flash_info[fl_idx].sector_count; 
                     sect_idx++)
                {
                    flash_info[fl_idx].start[sect_idx] 
                    = bank_base[fl_idx] + sect_idx * flash_spi[idx].block_size;
                    flash_info[fl_idx].protect[sect_idx] = 0;
                }
            }
            
            idx++;
        }
    }
    
    fiu_config(dev_cnt,
               max_dev_size >> SHIFT_1024,
               total_size >> SHIFT_1024);
    
    PRINTD("fiu_flash_type_idx=%d\n", fiu_flash_type_idx);
    
    /* disable the default write protection */
    for (fl_idx = 0; fl_idx < CFG_MAX_FLASH_BANKS; fl_idx++)
    {
        /* disable write protect signal, WP# */
        fiu_write_protect(fl_idx, 0);
        
#ifdef DEBUG_SPI
        fiu_uma_read(fl_idx, 
                     SPI_READ_STATUS_REG_CMD, 
                     0, 
                     FALSE, 
                     &status_reg_val, 
                     sizeof(UINT8));
        
        PRINTD("00 SPI_READ_STATUS_REG_CMD=%lx\n", status_reg_val);
#endif
        
        /* set to 0 bits 2,3,4,5 that move all chip to unprotected mode */
        /* status_reg_val &= 0xC3; */
        
        /* clear bit 7, 5, 4, 3 and 2 to disable write protections \
           bit 6, 1 and 0 are don't care */
        status_reg_val = 0;
        
        fiu_uma_write(fl_idx,
                      SPI_WRITE_ENABLE_CMD,
                      0,
                      FALSE,
                      NULL,
                      0);
        
        fiu_wait_for_ready(fl_idx);
        
        fiu_uma_write(fl_idx, 
                      SPI_WRITE_STATUS_REG_CMD, 
                      0, 
                      FALSE, 
                      &status_reg_val, 
                      sizeof(UINT8));
        
        PRINTD("01 SPI_WRITE_STATUS_REG_CMD=%lx\n", status_reg_val);
        
        fiu_wait_for_ready(fl_idx);
        
#ifdef DEBUG_SPI
        fiu_uma_read(fl_idx, 
                     SPI_READ_STATUS_REG_CMD, 
                     0, 
                     FALSE, 
                     &status_reg_val, 
                     sizeof(UINT8));
        
        PRINTD("02 SPI_READ_STATUS_REG_CMD=%lx\n", status_reg_val);
#endif
        /* enable write protect signal, WP# */
        fiu_write_protect(fl_idx, 1);
    }
    
    PRINTD("total_size=%lx\n", total_size);
    
    if (fiu_set_write_window(1,
                             0x0,
                             total_size,
                             FALSE) != TRUE)
    {
        PRINTD("fail to set write window\n"); 
        
        /* ERROR */
        return;
    }
    
    if (fiu_set_write_window(2,
                             0x0,
                             0x0,
                             FALSE) != TRUE)
    {
        PRINTD("fail to set write window\n");
        
        /* ERROR */
        return;
    }
    
    if (fiu_set_write_window(3,
                             0x0,
                             0x0,
                             FALSE) != TRUE)
    {
        PRINTD("fail to set write window\n");
        
        /* ERROR */
        return;
    }

}


/*-----------------------------------------------------------------------
 * SPI transfer
 *
 * This writes "bitlen" bits out the SPI MOSI port and simultaneously clocks
 * "bitlen" bits in the SPI MISO port.  That's just the way SPI works.
 *
 * The source of the outgoing bits is the "dout" parameter and the
 * destination of the input bits is the "din" parameter.  Note that "dout"
 * and "din" can point to the same memory location, in which case the
 * input data overwrites the output data (since both are buffered by
 * temporary variables, this is OK).
 *
 * If the chipsel() function is not NULL, it is called with a parameter
 * of '1' (chip select active) at the start of the transfer and again with
 * a parameter of '0' at the end of the transfer.
 *
 * If the chipsel() function _is_ NULL, it the responsibility of the
 * caller to make the appropriate chip select active before calling
 * spi_xfer() and making it inactive after spi_xfer() returns.
 */
int  spi_xfer(spi_chipsel_type chipsel, int bitlen, uchar *dout, uchar *din)
{

  PRINTD("spi_xfer: chipsel %08X dout %08X din %08X bitlen %d\n",
        (int)chipsel, *(uint *)dout, *(uint *)din, bitlen);

    return(0);
}


/* control the signal of WP# */
void fiu_write_protect(int dev, UINT8 enable)
{
    if (enable)
    {
#ifdef CONFIG_WPCM450_WHOVILLE
        /* initiate SPI flash WP# pin, GPIO40 */
        /* select GPI 113-108, GPIO 127-120, GPIO 40-37 */
        SET_FIELD(MFSEL1, MF_DVOSEL, 0x00);
        
        /* pull low GPIO40, external resister is pulling low */
        CLEAR_BIT(GP2DOUT, GPIO40);
        CLEAR_BIT(GP2CFG0, GPIO40);
#endif
    }
    else
    {
#ifdef CONFIG_WPCM450_WHOVILLE
        /* initiate SPI flash WP# pin, GPIO40 */
        /* select GPI 113-108, GPIO 127-120, GPIO 40-37 */
        SET_FIELD(MFSEL1, MF_DVOSEL, 0x00);
        
        /* pull high GPIO40 */
        SET_BIT(GP2DOUT, GPIO40);
        SET_BIT(GP2CFG0, GPIO40);
#endif
    }
    
    return;
}


/*------------------------------------------------------------------------------
* Function:    
*    FIU_config
*
* Descrition:
*    Configure the FIU according to the below parameters 
*
* Parameters:  
*    num_device        - number of flash devices connected to the chip. Can only 
*                        be 1 or 2 
*    total_flash_size  - The total flash size in Kbytes 
*    
*  
* Return Value:
*    None.
*----------------------------------------------------------------------------*/
void fiu_config(int num_device,
                int max_dev_size,
                int total_flash_size)
{
    UINT8 fl_blk_cnt = 0x00;
    /* SPI_fast_read_t fast_read = SPI_FAST_READ_DISABLE; */
    SPI_fast_read_t fast_read = SPI_FAST_READ_ENABLE;
    SPI_w_protect_int_t wpa_trap = SPI_W_PROTECT_INT_DISABLE;
    SPI_incorect_access_int_t iad_trap = SPI_INCORECT_ACCESS_INT_DISABLE; 
    SPI_w_burst_t write_burst_size = SPI_ONE_BYTE_W_BURST;
    /* SPI_r_burst_t read_burst_size = SPI_ONE_BYTE_R_BURST; */
    /* SPI_r_burst_t read_burst_size = SPI_SIXTEEN_BYTE_R_BURST; */
    SPI_r_burst_t read_burst_size = 2;
    
    
    /* open lock sequence - 0x87, 0x61, 0x63 to clear CFBB_PROT and PROT_LOCK */
    PROT_CLEAR = 0x87;
    PROT_CLEAR = 0x61;
    PROT_CLEAR = 0x63;
    
    
    /* config total flash size - FIU_CFG.FL_SIZE_P1 \
       if not 512 (KB) align, add 512 (KB) */
    if (total_flash_size & (SZ_512K - 1))
        total_flash_size = total_flash_size + SZ_512K; 

    /* convert from KBs to 512KBs */
    fl_blk_cnt = total_flash_size >> SHIFT_512;

    SET_FIELD(FIU_CFG,FIU_FL_SIZE, fl_blk_cnt);
    
   
    /* config larger flash size */
    SET_FIELD(SPI_FL_CFG, SPI_DEV_SIZE, max_dev_size >> SHIFT_512); 
    
    /* config fast read */
    if (fast_read)
    {
        SET_BIT(SPI_FL_CFG, SPI_F_READ_BIT);
        PRINTD("fast read\n");
    }
    else
    {
        CLEAR_BIT(SPI_FL_CFG, SPI_F_READ_BIT);
        PRINTD("normal read\n");
    }

    /* config IAD trap */
    if (iad_trap)
        SET_BIT(RESP_CFG, FIU_IAD_EN_BIT);
    else
        CLEAR_BIT(RESP_CFG, FIU_IAD_EN_BIT);

    /* config IAD trap */
    if (wpa_trap)
      SET_BIT(RESP_CFG, FIU_INT_EN_BIT);
    else
      CLEAR_BIT(RESP_CFG, FIU_INT_EN_BIT);
   
    /* config write burst size */
    SET_FIELD(BURST_CFG, FIU_W_BURST, write_burst_size);

    /* config read burst size */
    SET_FIELD(BURST_CFG, FIU_R_BURST, read_burst_size);

}


/*------------------------------------------------------------------------------
* Function:
*    fiu_set_write_window
*
* Description:
*    All access to the flash is write protected by default. Write access is only 
*    allowed inside a "write window". Use this function to open a "write window"
*    The FIU support up to 3 programmable write windows. Each window can be located 
*    on any address boundary of 4Kbytes and it size can vary from 4Kbytes to 4Mbytes
*
* Parameters:  
*    win_num    - "write window" number. Legal values are 1, 2 or 3
*    low_limit  - Address of the window's low limit in the flash address space.
*                 Writing 0 to this parameter will disable the window.
*    high_limit - Address of the window's high limit in the flash address space.
*                 Writing 0 to this parameter will disable the window.
*    lock       - Set to TRUE to lock the window position & size, otherwise set to
*                 FALSE. Once the window is locked it can be unlocked either by
*                 reset, or by calling FIU_unlock_write_protection().
*
* Return Value:    
*    TRUE  - write window was set as requested.
*    FALSE - Error: The selected window is locked and can not be changed.
*----------------------------------------------------------------------------*/
BOOL fiu_set_write_window(int win_num,
                          UINT32 low_limit,
                          UINT32 high_limit,
                          BOOL lock) 
{
    PRINTD("win_num=%d, high_limit=%lx, low_limit=%lx, lock=%d\n",
           win_num, high_limit, low_limit, lock);
    
    /* divide the window limits by 4K */
    low_limit = low_limit >> WIN_LIMIT_4K_SHIFT;
    high_limit = high_limit >> WIN_LIMIT_4K_SHIFT;
    
    /* check that high_limit is indeed higher than low_limit */
    if (!(high_limit >= low_limit))
    {
        PRINTD("high_limit > low_limit, high_limit=%lx, low_limit=%lx\n",
               high_limit, low_limit);
        return FALSE;
    }

    switch (win_num)
    {
        case 1:
            /* check that WIN1 is not locked */
            if (IS_BIT_SET(PROT_LOCK, FIU_FWIN1_LK_BIT))
                return FALSE;
            
            /* set WIN1 */
            SET_FIELD(FWIN1_LOW, FIU_WIN_LOW, low_limit);
            SET_FIELD(FWIN1_HIGH, FIU_WIN_HIGH, high_limit);
            
            /* if lock - lock WIN1 */
            if (lock)
                SET_BIT(PROT_LOCK, FIU_FWIN1_LK_BIT);
            
            break;
            
        case 2:
            /* check that WIN2 is not locked */
            if (IS_BIT_SET(PROT_LOCK, FIU_FWIN2_LK_BIT))
                return FALSE;
            
            /* set WIN2 */
            SET_FIELD(FWIN2_LOW, FIU_WIN_LOW, low_limit);
            SET_FIELD(FWIN2_HIGH, FIU_WIN_HIGH, high_limit);
            
            /* if lock - lock WIN2 */
            if (lock)
                SET_BIT(PROT_LOCK, FIU_FWIN2_LK_BIT);
            
            break;
            
        case 3:
            /* check that WIN3 is not locked */
            if (IS_BIT_SET(PROT_LOCK, FIU_FWIN3_LK_BIT))
                return FALSE;
            
            /* set WIN3 */
            SET_FIELD(FWIN3_LOW, FIU_WIN_LOW, low_limit);
            SET_FIELD(FWIN3_HIGH, FIU_WIN_HIGH, high_limit);
            
            /* if lock - lock WIN3 */
            if (lock)
                SET_BIT(PROT_LOCK, FIU_FWIN3_LK_BIT);
            
            break;
            
        default:
            PRINTD("win_num is not correct\n");
            /* illegal device */
            return FALSE;
    }
    
    return TRUE;
}


/********************************************************************
name : 

description :

parameters :

return :

*********************************************************************/
INT32 fiu_read_pid(UINT32 dev_num, 
                   UCHAR *pid0, 
                   UCHAR *pid1)
{
    UINT32 data;
    
    fiu_uma_read(dev_num,
                 SPI_READ_JEDEC_ID_CMD, /* read JEDEC ID transaction code */
                 0x0,
                 FALSE,
                 (UINT8*)&data,
                 3);
    
    *pid0 = (data & 0x0000FF);
    
    if((*pid0)== WINBOND_MF_ID)
    {
        *pid1 = (data & 0xFF0000) >> 16;
        
        switch((*pid1))
        {
            case W25P80_CAPACITY_ID:
                fiu_flash_type_idx = W25P80;
                break;
            case W25P16_CAPACITY_ID:
                fiu_flash_type_idx = W25P16;
                break;
            case W25P32_CAPACITY_ID:
                fiu_flash_type_idx = W25P32;
                break;
            case W25Q128_CAPACITY_ID:
                fiu_flash_type_idx = W25Q128;
                break;
        }
    }
    
    else if((*pid0)== STMICRO_MF_ID)
    {
        *pid1 = (data & 0xFF0000) >> 16;
    
        switch((*pid1))
        {
            case M25P80_CAPACITY_ID:
                fiu_flash_type_idx = M25P80;
                break;
            case M25P16_CAPACITY_ID:
                fiu_flash_type_idx = M25P16;
                break;
            case M25P32_CAPACITY_ID:
                fiu_flash_type_idx = M25P32;
                break;
            case M25P64_CAPACITY_ID:
                fiu_flash_type_idx = M25P64;
                break;
        }
    }
    
    else if((*pid0)== ATMEL_MF_ID)
    {
        *pid1 = ((data & 0x1F00) >> 8);
        
        switch((*pid1))
        {
            case AT26DF08_CAPACITY_ID:
                fiu_flash_type_idx = AT26DF08;
                break;
            case AT26DF16_CAPACITY_ID:
                fiu_flash_type_idx = AT26DF16;
                break;
            case AT26DF32_CAPACITY_ID:
                fiu_flash_type_idx = AT26DF32;
                break;
        }
    }
    
    else if((*pid0)== SST_MF_ID)
    {
        *pid1 = (data & 0xFF0000) >> 16;
     
        switch((*pid1))
        {
            case SST25VF080B_CAPACITY_ID:
                fiu_flash_type_idx = SST25VF080B;
                break;
            case SST25VF016B_CAPACITY_ID:
                fiu_flash_type_idx = SST25VF016B;
                break;
            case SST25VF032B_CAPACITY_ID:
                fiu_flash_type_idx = SST25VF032B;
                break;
        }
    }
    
    else if((*pid0)== MXIC_MF_ID)
    {
        *pid1 = (data & 0xFF0000) >> 16;
     
        switch((*pid1))
        {
            case MX25L8005_CAPACITY_ID:
                fiu_flash_type_idx = MX25L8005;
                break;
            case MX25L1605_CAPACITY_ID:
                fiu_flash_type_idx = MX25L1605;
                break;
            case MX25L3205_CAPACITY_ID:
                fiu_flash_type_idx = MX25L3205;
                break;
            case MX25L6405_CAPACITY_ID:
                fiu_flash_type_idx = MX25L6405;
                break;
            case MX25L12805_CAPACITY_ID:
                fiu_flash_type_idx = MX25L12805;
                break;
        }
    }
    
    else if((*pid0)== SPANSION_MF_ID)
    {
        *pid1 = (data & 0xFF0000) >> 16;
     
        switch((*pid1))
        {
            case S25FL008A_CAPACITY_ID:
                fiu_flash_type_idx = S25FL008A;
                break;
            case S25FL016A_CAPACITY_ID:
                fiu_flash_type_idx = S25FL016A;
                break;
            case S25FL032A_CAPACITY_ID:
                fiu_flash_type_idx = S25FL032A;
                break;
            case S25FL064A_CAPACITY_ID:
                fiu_flash_type_idx = S25FL064A;
                break;
            case S25FL128P_CAPACITY_ID:
                fiu_flash_type_idx = S25FL128P;
                break;
        }
    }
    
    /* setup default flash size as 2 MB */
    if (fiu_flash_type_idx == DEFAULT_FLASH)
    {
        *pid0 = DEFAULT_MF_ID;
        *pid1 = DEFAULT_CAPACITY_ID;
    }
    
    PRINTD("pid0=%x, pid1=%x\n", *pid0, *pid1);
    
#ifdef DEBUG_SPI
    switch(flash_spi[fiu_flash_type_idx].mf_id)
    {
        case WINBOND_MF_ID:
            printf("Winbond ");
            break;
        case STMICRO_MF_ID:
            printf("STMicro ");
            break;
        case ATMEL_MF_ID:
            printf("ATMEL ");
            break;
        case SST_MF_ID:
            printf("SST ");
            break;
        case MXIC_MF_ID:
            printf("MXIC ");
            break;
        case SPANSION_MF_ID:
            printf("Spansion ");
            break;
        default:
            printf("Unknown Vendor ");
            break;
    }
    
    printf("%s\n", flash_spi[fiu_flash_type_idx].name);
#endif
    
    return 0;
}


/*------------------------------------------------------------------------------
* Function:
*    fiu_uma_read
*                
* Description:
*    Read up to 4 bytes from the flash. using the FIU User Mode Access (UMA).
*
* Parameters:
*    device           - Select the flash device (0 or 1) to be accessed 
*    transaction_code - Specify the SPI UMA transaction code
*    address          - Location on the flash , in the flash address space
*    address_size     - if TRUE, 3 bytes address, to be placed in UMA_AB0-2
*                       else (FALSE), no address for this SPI UMA transaction 
*    data             - a pointer to a data buffer to hold the read data.
*    data_size        - buffer size. Legal sizes are 1,2,3,4 
*                       
*
* Return Value:
*    None
*----------------------------------------------------------------------------*/
void fiu_uma_read(int device,
                  UINT8 transaction_code,        
                  UINT32 address,
                  BOOL address_size,
                  UINT8 * data,
                  int data_size) 
{
    UINT8 uma_ab;
    UINT8 uma_cts = 0x0;
    
    /* set device number - DEV_NUM in UMA_CTS \
       legal device numbers are 0,1,2,3 */
    switch(device)
    {
        case 0:
        case 1:
        case 2:
        case 3:
            SET_FIELD(uma_cts, UMA_DEV_NUM, device);
            break;
        default:
            return;
    }
    
    /* set transaction code in UMA_CODE */
    UMA_CODE = transaction_code;
    
    if (address_size)
    {
        /* set address size bit */
        SET_BIT(uma_cts, UMA_A_SIZE_BIT);
        
        /* set the UMA address registers - UMA_AB0-2 */
        uma_ab = address & 0xFF;
        UMA_AB0 = uma_ab;
        
        uma_ab = (address >> 8) & 0xFF;
        UMA_AB1 = uma_ab;
        
        uma_ab = (address >> 16) & 0xFF;
        UMA_AB2 = uma_ab;
    }
    else
    {
        /* clear address size bit */
        CLEAR_BIT(uma_cts, UMA_A_SIZE_BIT);
    }
    
    /* check legal data size */
    switch (data_size)
    {
        case 4:
            UMA_DB3 = 0x0;
        case 3:
            UMA_DB2 = 0x0;
        case 2:
            UMA_DB1 = 0x0;
        case 1:
            UMA_DB0 = 0x0;
            break;       
        default:
            /* illegal size */
        return;
    }
    
    /* set data size - D_SIZE in UMA_CTS */
    SET_FIELD(uma_cts, UMA_D_SIZE, data_size);
    
    /* read select in UMA_CTS */
    CLEAR_BIT(uma_cts, UMA_RD_WR_BIT);
    
    UMA_CTS = uma_cts;
    
    /* initiate the read */
    SET_BIT(UMA_CTS, UMA_EXEC_DONE_BIT);
    
    /* wait for indication that transaction has terminated */
    while (IS_BIT_SET(UMA_CTS, UMA_EXEC_DONE_BIT));
    
    /* copy read data from UMA_DB0-3 regs to data buffer */
    switch (data_size)
    {
        case 4:
            *(data + 3) = UMA_DB3;
        case 3:
            *(data + 2) = UMA_DB2;
        case 2:
            *(data + 1) = UMA_DB1;
        case 1:
            *data = UMA_DB0;
        break;  
    }
}





/*------------------------------------------------------------------------------
* Function:    
*    FIU_uma_write
*
* Description:
*    Write up to 4 bytes to the flash using the FIU User Mode Access (UMA) which 
*    allows the core an indirect access to the flash, bypassing FIU flash write
*    protection.
*                  
* Parameters:  
*    device           - Select the flash device (0 or 1) to be accessed
*    transaction_code - Specify the SPI UMA transaction code
*    address          - Location on the flash, in the flash address space
*    address_size     - if TRUE, 3 bytes address, to be placed in UMA_AB0-2
*                       else (FALSE), no address for this SPI UMA transaction 
*    data             - a pointer to a data buffer (buffer of bytes)
*    data_size        - data buffer size in bytes. Legal sizes are 0,1,2,3,4 
*
* Return Value:
*    None
*----------------------------------------------------------------------------*/
void fiu_uma_write(int device,
                   UINT8 transaction_code,
                   UINT32 address,
                   BOOL address_size,
                   UINT8 * data,
                   int data_size)
{
    UINT8 uma_ab = 0x0;
    UINT8 uma_cts = 0x0;
    
    /* set device number - DEV_NUM in UMA_CTS \
       legal device numbers are 0,1,2,3 */
    switch(device)
    {
        case 0 :
        case 1 :
        case 2 :
        case 3 :
            SET_FIELD(uma_cts, UMA_DEV_NUM, device);
            break;
        default:
            return;
    }
    
    /* set transaction code in UMA_CODE */
    UMA_CODE = transaction_code;
    
    if (address_size)
    {
        /* set address size bit */
        SET_BIT(uma_cts, UMA_A_SIZE_BIT);
        
        /* set the UMA address registers - UMA_AB0-2 */
        uma_ab = address & BITS_7_0;
        UMA_AB0 = uma_ab;
        
        uma_ab = (address & BITS_15_8) >> 8;
        UMA_AB1 = uma_ab;
        
        uma_ab = (address & BITS_23_16) >> 16;
        UMA_AB2 = uma_ab;
    } 
    else
    {
        /* clear address size bit */
        CLEAR_BIT(uma_cts, UMA_A_SIZE_BIT);
    }
    
    /* set the UMA data registers - UMA_DB0-3 */
    switch (data_size)
    {
        case 4:
            UMA_DB3 = *(data+3);
        case 3:
            UMA_DB2 = *(data+2);
        case 2:
            UMA_DB1 = *(data+1);
        case 1:
            UMA_DB0 = *data;
            break;
        case 0:
            /* no data to write (a control transaction) */
            break;
        default:
            /* illegal data_size */
            return;
    }
    
    /* set data size */
    SET_FIELD(uma_cts,UMA_D_SIZE, data_size);
    
    /* write select */
    SET_BIT(uma_cts, UMA_RD_WR_BIT);
    
    UMA_CTS = uma_cts;  
    
    /* initiate the write */
    SET_BIT(UMA_CTS, UMA_EXEC_DONE_BIT);
    
    /* wait for indication that transaction has terminated */
    while (IS_BIT_SET(UMA_CTS, UMA_EXEC_DONE_BIT));
}


/* sequential write 
   - data_size should be even and greater than 2 for SST flash
   - bit 0 of address should be zero, 
   - (address + data_size) can't exceed one page*/
void fiu_uma_sequ_write(int device,
                        UINT32 address,
                        UINT8 *data,
                        UINT32 data_size,
                        UINT8 bytes_per_cycle)
{
    UINT8 uma_ab = 0x00;
    UINT8 uma_cts = 0x00;
    UINT32 cur_offset = 0;
    
    PRINTD("device=%d, address=%lx, data=%p, data_size=%lx, bytes_per_cycle=%d\n",
           device, address, data, data_size, bytes_per_cycle);
    
    /* set transaction code in UMA_CODE */
    UMA_CODE = SPI_SEQU_PRGM_CMD;
    
    /* set the UMA address registers - UMA_AB0-2 */
    uma_ab = address & BITS_7_0;
    UMA_AB0 = uma_ab;

    uma_ab = (address & BITS_15_8) >> 8;
    UMA_AB1 = uma_ab;

    uma_ab = (address & BITS_23_16) >> 16;
    UMA_AB2 = uma_ab;
    
    /* set the UMA data registers - UMA_DB0-3 */
    switch (bytes_per_cycle)
    {
        case 1:
            UMA_DB0 = *(data + cur_offset++);
            /* PRINTD("UMA_DB0=%lx\n", UMA_DB0); */
            break;
            
        case 2:
            UMA_DB0 = *(data + cur_offset++);
            UMA_DB1 = *(data + cur_offset++);
            /* PRINTD("UMA_DB0=%lx\n", UMA_DB0); */
            /* PRINTD("UMA_DB1=%lx\n", UMA_DB1); */
            break;
            
        case 3:
            UMA_DB0 = *(data + cur_offset++);
            UMA_DB1 = *(data + cur_offset++);
            UMA_DB2 = *(data + cur_offset++);
            /* PRINTD("UMA_DB0=%lx\n", UMA_DB0); */
            /* PRINTD("UMA_DB1=%lx\n", UMA_DB1); */
            /* PRINTD("UMA_DB2=%lx\n", UMA_DB2); */
            break;
            
        case 4:
            UMA_DB0 = *(data + cur_offset++);
            UMA_DB1 = *(data + cur_offset++);
            UMA_DB2 = *(data + cur_offset++);
            UMA_DB3 = *(data + cur_offset++);
            /* RINTD("UMA_DB0=%lx\n", UMA_DB0); */
            /* RINTD("UMA_DB1=%lx\n", UMA_DB1); */
            /* RINTD("UMA_DB2=%lx\n", UMA_DB2); */
            /* RINTD("UMA_DB3=%lx\n", UMA_DB3); */
            break;
            
        default:
            return;
    }
    
    /* set device number - DEV_NUM in UMA_CTS
       legal device numbers are 0,1,2,3 */
    switch (device)
    {
        case 0:
        case 1:
        case 2:
        case 3:
            SET_FIELD(uma_cts, UMA_DEV_NUM, device);
            break;
            
        default:
            PRINTD("ERROR device number\n");
            return;
    }

    /* set address size bit */
    SET_BIT(uma_cts, UMA_A_SIZE_BIT);
    
    /* set data size */
    SET_FIELD(uma_cts, UMA_D_SIZE, bytes_per_cycle);
    
    /* write select */
    SET_BIT(uma_cts, UMA_RD_WR_BIT);
    
    /* set to hardware register */
    UMA_CTS = uma_cts;
    
    /* initiate the write */
    SET_BIT(UMA_CTS, UMA_EXEC_DONE_BIT);
    
    /* wait for indication that transaction has terminated */
    while (IS_BIT_SET(UMA_CTS, UMA_EXEC_DONE_BIT));
    
    /* check if flash is busy */
    fiu_wait_for_ready(device);
    
    if (data_size <= cur_offset)
    {
        /* issue write disable command to terminate the sequential program */
        fiu_uma_write(device,
                      SPI_WRITE_DISABLE_CMD,
                      0,
                      FALSE,
                      NULL,
                      0);
        
        /* check if flash is busy */
        fiu_wait_for_ready(device);
        
        return;
    }
    
    /* clear address size bit */
    CLEAR_BIT(uma_cts, UMA_A_SIZE_BIT);
    
    while (cur_offset < data_size)
    {
        /* set transaction code in UMA_CODE */
        UMA_CODE = SPI_SEQU_PRGM_CMD;
        
        /* set the UMA data registers - UMA_DB0-3 */
        switch (bytes_per_cycle)
        {
            case 1:
                UMA_DB0 = *(data + cur_offset++);
                /* PRINTD("UMA_DB0=%lx\n", UMA_DB0); */
                break;
                
            case 2:
                UMA_DB0 = *(data + cur_offset++);
                UMA_DB1 = *(data + cur_offset++);
                /* PRINTD("UMA_DB0=%lx\n", UMA_DB0); */
                /* PRINTD("UMA_DB1=%lx\n", UMA_DB1); */
                break;
                
            case 3:
                UMA_DB0 = *(data + cur_offset++);
                UMA_DB1 = *(data + cur_offset++);
                UMA_DB2 = *(data + cur_offset++);
                /* PRINTD("UMA_DB0=%lx\n", UMA_DB0); */
                /* PRINTD("UMA_DB1=%lx\n", UMA_DB1); */
                /* PRINTD("UMA_DB2=%lx\n", UMA_DB2); */
                break;
                
            case 4:
                UMA_DB0 = *(data + cur_offset++);
                UMA_DB1 = *(data + cur_offset++);
                UMA_DB2 = *(data + cur_offset++);
                UMA_DB3 = *(data + cur_offset++);
                /* PRINTD("UMA_DB0=%lx\n", UMA_DB0); */
                /* PRINTD("UMA_DB1=%lx\n", UMA_DB1); */
                /* PRINTD("UMA_DB2=%lx\n", UMA_DB2); */
                /* PRINTD("UMA_DB3=%lx\n", UMA_DB3); */
                break;
                
            default:
                return;
        }
        
        /* set to hardware register */
        UMA_CTS = uma_cts;
        
        /* initiate the write */
        SET_BIT(UMA_CTS, UMA_EXEC_DONE_BIT);
        
        /* wait for indication that transaction has terminated */
        while (IS_BIT_SET(UMA_CTS, UMA_EXEC_DONE_BIT));
        
        /* check if flash is busy */
        fiu_wait_for_ready(device);
    }
    
    /* issue write disable command to terminate the sequential program */
    fiu_uma_write(device,
                  SPI_WRITE_DISABLE_CMD,
                  0,
                  FALSE,
                  NULL,
                  0);
    
    /* check if flash is busy */
    fiu_wait_for_ready(device);
    
    return;
}


/* page write 
   - data_size should be even and between 4 to 256
   - bit 0 of address should be zero, 
   - (address + data_size) can't exceed one page*/
void fiu_uma_page_write(int device,
                        UINT32 address,
                        UINT8 *data,
                        int data_size)
{
    UINT8 uma_ab = 0x00;
    UINT8 uma_cts = 0x00;
    int cur_offset = 0;
    int rest_data = 0;
    
    /* set transaction code in UMA_CODE */
    UMA_CODE = SPI_PAGE_PRGM_CMD;
    
    /* set the UMA address registers - UMA_AB0-2 */
    uma_ab = address & BITS_7_0;
    UMA_AB0 = uma_ab;

    uma_ab = (address & BITS_15_8) >> 8;
    UMA_AB1 = uma_ab;

    uma_ab = (address & BITS_23_16) >> 16;
    UMA_AB2 = uma_ab;
    
    /* set the UMA data registers - UMA_DB0-3 */
    UMA_DB0 = *(data + cur_offset++);
    UMA_DB1 = *(data + cur_offset++);
    UMA_DB2 = *(data + cur_offset++);
    UMA_DB3 = *(data + cur_offset++);
    
    /* set device number - DEV_NUM in UMA_CTS
       legal device numbers are 0,1,2,3 */
    switch(device)
    {
        case 0 :
        case 1 :
        case 2 :
        case 3 :
            SET_FIELD(uma_cts, UMA_DEV_NUM, device);
            break;
        default:
            return;
    }

    /* set address size bit */
    SET_BIT(uma_cts, UMA_A_SIZE_BIT);
    
    /* set data size */
    SET_FIELD(uma_cts,UMA_D_SIZE, 4);

    /* write select */
    SET_BIT(uma_cts, UMA_RD_WR_BIT);

    /* software control chip select */
    UMA_ECTS = 0x0F & ~(0x01 << device);

    UMA_CTS = uma_cts;

    /* initiate the write */
    SET_BIT(UMA_CTS, UMA_EXEC_DONE_BIT);

    /* wait for indication that transaction has terminated */
    while (IS_BIT_SET(UMA_CTS, UMA_EXEC_DONE_BIT));
    
    cur_offset = 4;
    
    if (data_size <= cur_offset)
    {
        /* software release chip select */
        UMA_ECTS = 0x0F;
        
        return;
    }
    
    for(;cur_offset < data_size;)
    {
        rest_data = data_size - cur_offset;
        
        if (rest_data >= 8)
        {
            /* 8 bytes */
            UMA_CODE = *(data + cur_offset++);
            UMA_AB2 = *(data + cur_offset++);
            
            UMA_AB1 = *(data + cur_offset++);
            UMA_AB0 = *(data + cur_offset++);
            
            UMA_DB0 = *(data + cur_offset++);
            UMA_DB1 = *(data + cur_offset++);
            
            UMA_DB2 = *(data + cur_offset++);
            UMA_DB3 = *(data + cur_offset++);
        }
        else if (rest_data == 6)
        {
            /* 6 bytes */
            UMA_CODE = *(data + cur_offset++);
            UMA_AB2 = *(data + cur_offset++);
            
            UMA_AB1 = *(data + cur_offset++);
            UMA_AB0 = *(data + cur_offset++);
            
            UMA_DB0 = *(data + cur_offset++);
            UMA_DB1 = *(data + cur_offset++);
            
            /* set data field as 2 */
            uma_cts = ((uma_cts & 0xF8) | 0x02);
        }
        else if (rest_data == 4)
        {
            /* 4 bytes */
            UMA_CODE = *(data + cur_offset++);
            UMA_AB2 = *(data + cur_offset++);
            
            UMA_AB1 = *(data + cur_offset++);
            UMA_AB0 = *(data + cur_offset++);
            
            /* set data field as 0 */
            uma_cts = (uma_cts & 0xF8);
        }
        /* if (rest_data == 2) */
        else if (rest_data == 2)
        {
            /* 2 bytes */
            UMA_CODE = *(data + cur_offset++);
            UMA_DB0 = *(data + cur_offset++);
            
            /* set data field as 1 */
            uma_cts = ((uma_cts & 0xF0) | 0x01);
        }
        else
        {
            PRINTD("data_size error\n");
            break;
        }
        
        UMA_CTS = uma_cts;
        
        /* initiate the write */
        SET_BIT(UMA_CTS, UMA_EXEC_DONE_BIT);
        
        /* wait for indication that transaction has terminated */
        while (IS_BIT_SET(UMA_CTS, UMA_EXEC_DONE_BIT));
    }
    
    /* software release chip select */
    UMA_ECTS = 0x0F;
    
    return;
}


/********************************************************************
name : 

description : Waits until the flash is not busy

parameters :

return :

*********************************************************************/
void fiu_wait_for_ready(UINT8 device) 
{
    UINT8 busy = 1;
    
#ifdef DEBUG_SPI
    UINT32 retry = 0;
#endif
    
    while (busy)
    {
        fiu_uma_read(device,
                     SPI_READ_STATUS_REG_CMD,
                     0,
                     0,
                     &busy,
                     1);
        
        /* get the status register, only "busy" bit 0 */
        busy &= 0x01;
        
#ifdef DEBUG_SPI
        retry++;
        /* printf("retry=%ld, UMA_DB0=%lx\n", retry, UMA_DB0); */
#endif
        
#if 0
        if (busy)
        {
            /* delay 1 ms */
            udelay(1000);
        }
#endif
    }
}


#endif  /* CONFIG_WPCM450_SPI_DRIVER */
