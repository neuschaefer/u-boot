#ifndef _FIU_REGS_WPCS410_H
#define _FIU_REGS_WPCS410_H

/************************************************************************/
/* WPCS410 registers definition                                         */
/************************************************************************/

/************************************************************************/
/*  FIU registers                                                       */
/************************************************************************/

#define FIU_BA      0xC8000000
#define FIU_CFG     VPchar(FIU_BA+0x00)   /* FIU configuration */
#define BURST_CFG   VPchar(FIU_BA+0x01)   /* Burst configuration */
#define RESP_CFG    VPchar(FIU_BA+0x02)   /* FIU response configuration */
#define CFBB_PROT   VPchar(FIU_BA+0x03)   /* Core firmware boot block protection */
#define FWIN1_LOW   VPshort(FIU_BA+0x04)  /* Flash Access Windows 1, low limit */
#define FWIN1_HIGH  VPshort(FIU_BA+0x06)  /* Flash Access Windows 1, high limit */
#define FWIN2_LOW   VPshort(FIU_BA+0x08)  /* Flash Access Windows 2, low limit */
#define FWIN2_HIGH  VPshort(FIU_BA+0x0A)  /* Flash Access Windows 2, high limit */
#define FWIN3_LOW   VPshort(FIU_BA+0x0C)  /* Flash Access Windows 3, low limit */
#define FWIN3_HIGH  VPshort(FIU_BA+0x0E)  /* Flash Access Windows 3, high limit */
#define PROT_LOCK   VPchar(FIU_BA+0x10)   /* Protection lock */
#define PROT_CLEAR  VPchar(FIU_BA+0x11)   /* Protection and lock clear */
#define FIU_TEST    VPchar(FIU_BA+0x13)   /* FIU test */
#define SPI_FL_CFG  VPchar(FIU_BA+0x14)   /* SPI flash configuration */
#define SPI_DFT     VPchar(FIU_BA+0x15)   /* SPI DFT */
#define UMA_CODE    VPchar(FIU_BA+0x16)   /* UMA code byte */
#define UMA_AB0     VPchar(FIU_BA+0x17)   /* UMA Address Byte 0 */
#define UMA_AB1     VPchar(FIU_BA+0x18)   /* UMA Address Byte 1 */
#define UMA_AB2     VPchar(FIU_BA+0x19)   /* UMA Address Byte 2 */
#define UMA_DB0     VPchar(FIU_BA+0x1A)   /* UMA Data Byte 0 */
#define UMA_DB1     VPchar(FIU_BA+0x1B)   /* UMA Data Byte 1 */
#define UMA_DB2     VPchar(FIU_BA+0x1C)   /* UMA Data Byte 2 */
#define UMA_DB3     VPchar(FIU_BA+0x1D)   /* UMA Data Byte 3 */
#define UMA_CTS     VPchar(FIU_BA+0x1E)   /* UMA control and status */
#define UMA_ECTS    VPchar(FIU_BA+0x1F)   /* UMA extended control and status */

/* FIU & SPI registers fields */
/* FIU_CFG fields */
#define FIU_FL_SIZE_S       8
#define FIU_FL_SIZE_P       0

/* BURST_CFG fields */
#define FIU_W_BURST_S       2
#define FIU_W_BURST_P       4
#define FIU_R_BURST_S       2
#define FIU_R_BURST_P       0


/* RESP_CFG fields */
#define FIU_INT_EN_BIT      1
#define FIU_IAD_EN_BIT      0

#define FIU_WIN_LOW_S       13
#define FIU_WIN_LOW_P       0

#define FIU_WIN_HIGH_S      15
#define FIU_WIN_HIGH_P      0

/* PROT_LOCK register fields */
#define FIU_PRM_LK_BIT      4
#define FIU_FWIN3_LK_BIT    2
#define FIU_FWIN2_LK_BIT    1
#define FIU_FWIN1_LK_BIT    0


/* SPI_FL_CFG fields */
#define SPI_F_READ_BIT      6
#define SPI_DEV_SIZE_S      6
#define SPI_DEV_SIZE_P      0

/* UMA_CTS fields */
#define UMA_EXEC_DONE_BIT   7
#define UMA_DEV_NUM_S       2
#define UMA_DEV_NUM_P       5
#define UMA_A_SIZE_BIT      3
#define UMA_D_SIZE_S        3
#define UMA_D_SIZE_P        0
#define UMA_RD_WR_BIT       4

/* FIU Flash definitions */
#if (FLASH_IF == FLASH_FIU_IF)
#define FLASH_BASE          0x40000000
#elif (FLASH_IF == FLASH_XBUS_IF) 
#define FLASH_BASE          0x42000000
#error "COMPILATION ERROR : platform not supported"
#endif

#define FLASH_BLOCK_SIZE    0x10000

#endif /* _FIU_REGS_WPCS410_H */
