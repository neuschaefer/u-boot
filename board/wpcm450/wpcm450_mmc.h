/*
 * $RCSfile$
 * $Revision$
 * $Date$
 * $Author$
 *
 * WPCM450 SDHC driver.
 *  
 * Copyright (C) 2007 Avocent Corp.
 *
 * This file is subject to the terms and conditions of the GNU 
 * General Public License. This program is distributed in the hope 
 * that it will be useful, but WITHOUT ANY WARRANTY; without even 
 * the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
 * PURPOSE.  See the GNU General Public License for more details.
 */


#define ENABLE      1
#define DISABLE     0

/* Activate the Flash Readers (unless TASK_FLASH is DISABLE) */
/* Define each reader as ENABLE/DISABLE */
#define  LUN_SD				ENABLE		/* Secure Digital */
#define  LUN_MMC			ENABLE		/* MultiMedia Card */

/* Auxiliary macro do determine a task/module inclusion */
#define IS_ENABLED(TASK)	(TASK == ENABLE)


#define SUPPORT_SD_2_0      1

#define SDHC_SECTOR_SIZE    512


/* Flash Card Types */
typedef enum CARD_TYPE {
    UNKNOWN_CARD = 0,
    SD_CARD,             /* Secure Digital Card */
    MINI_SD_CARD,        /* Mini Secure Digital Card */
    MMC_CARD,            /* MultiMediaCard (ver. 3.31) */
    RS_MMC_CARD,         /* Reduced Size MultiMediaCard */
    MMCPLUS_CARD,        /* MultiMediaCard (ver. 4.1) */
    SECURE_MMC_CARD,     /* Secure MultiMediaCard (ver. 4.1) */
    MS_CARD,             /* Memory Stick Card */
    MS_RO_CARD,          /* Memory Stick Read-Only Card */
    MS_ROM_CARD,         /* MS-ROM Card */
    MS_PRO_CARD,         /* MS-PRO Card */
    MS_PRO_RO_CARD,      /* MS-PRO Read-Only Card */
    MS_DUO_CARD,         /* MS-Duo Card */
    MS_PRO_DUO_CARD,     /* MS-PRO-Duo Card */
    SM_CARD,             /* Smart Media Card */
    XD_CARD,             /* Extreme Digital Picture Card */
    CF_CARD              /* Compact Flash Card */
} CARD_TYPE_TYPE;

/* Check to which family belongs 'card' (assuming cards of each family are consecutive) */
#define CARD_IS_SD(card)    ((card >= SD_CARD)  && (card <= MINI_SD_CARD))
#define CARD_IS_MMC(card)   ((card >= MMC_CARD) && (card <= SECURE_MMC_CARD))
#define CARD_IS_MS(card)    ((card >= MS_CARD)  && (card <= MS_PRO_DUO_CARD))
#define CARD_IS_SM(card)    (card == SM_CARD)
#define CARD_IS_XD(card)    (card == XD_CARD)
#define CARD_IS_CF(card)    (card == CF_CARD)


/* Flash Cards Error Codes */
typedef enum FLASH_ERR_CODE {
    CARD_SUCCESS          = 0x0,
    CARD_SUCCESS_WITH_ECC,
    CARD_ERROR_PARAMETER,
    CARD_ERROR_NO_MEMORY,
    CARD_ERROR_OUT_OF_SPACE,
    CARD_ERROR_NO_MEDIA,
    CARD_ERROR_MEDIA_FAILED,
    CARD_ERROR_WRITE_PROTECT,
    CARD_ERROR_PROTOCOL,
    CARD_ERROR_TIMEOUT,
    CARD_ERROR_READ_DATA,
    CARD_ERROR_MISCOMPARE
} FLASH_ERR_CODE_TYPE;



/*-----------------------------------------------------------------------
 * SD Host Controller Register
 *-----------------------------------------------------------------------*/

/* SD DMA ADDRESS Register */
#define SDHC_DMA_ADD_ADDR           (SDHC_BASE_ADDR + 0x0)
#define SDHC_DMA_ADD                VPlong(SDHC_DMA_ADD_ADDR)
//#define SDHC_DMA_ADD_SIZE          SDHC_BLK_SIZE_ADDR, SDHC_ACCESS, 32


/* Block Size Register */
#define SDHC_BLK_SIZE_ADDR          (SDHC_BASE_ADDR + 0x4)
#define SDHC_BLK_SIZE               VPshort(SDHC_BLK_SIZE_ADDR)
//#define SDHC_BLK_SIZE               SDHC_BLK_SIZE_ADDR, SDHC_ACCESS, 16
#define XFR_BLK_SIZE_P              0   /* Transfer Block Size */
#define XFR_BLK_SIZE_S              12
#define HOST_SDMA_BUF_BOUNDARY_P    12  /* Host SDMA Buffer Boundary */
#define HOST_SDMA_BUF_BOUNDARY_S    3

/* Block Count Register */
#define SDHC_BLK_CNT_ADDR           (SDHC_BASE_ADDR + 0x6)
#define SDHC_BLK_CNT                VPshort(SDHC_BLK_CNT_ADDR)
//#define SDHC_BLK_CNT                SDHC_BLK_CNT_ADDR, SDHC_ACCESS, 16
#define MAX_BLK_CNT                 (((UINT32)1 << 16) - 1)

/* Argument Register 1+2*/
#define SDHC_ARG_ADDR               (SDHC_BASE_ADDR + 0x8)
#define SDHC_ARG                    VPlong(SDHC_ARG_ADDR)
//#define SDHC_ARG                    SDHC_ARG_ADDR, SDHC_ACCESS, 32

/* Transfer Mode Register */
#define SDHC_XFR_MODE_ADDR          (SDHC_BASE_ADDR + 0xC)
#define SDHC_XFR_MODE               VPshort(SDHC_XFR_MODE_ADDR)
//#define SDHC_XFR_MODE               SDHC_XFR_MODE_ADDR, SDHC_ACCESS, 16
#define BLK_DMA_EN_P                0    /* DMA Enable */
#define BLK_DMA_EN_S                1
#define BLK_CNT_EN_P                1    /* Block Count Enable */
#define BLK_CNT_EN_S                1
#define AUTO_CMD12_EN_P             2    /* Auto CMD12 Enable */
#define AUTO_CMD12_EN_S             1
#define DATA_XFR_DIR_SEL_P          4    /* Data Transfer Direction Select */
#define DATA_XFR_DIR_SEL_S          1
#define SDHC_DATA_WRITE             0    /* Write (Host to Card) */
#define SDHC_DATA_READ              1    /* Read (Card to Host) */
#define MULTI_BLK_SEL_P             5    /* Multi/Single Block Select */
#define MULTI_BLK_SEL_S             1

/* Command Register */
#define SDHC_CMD_ADDR               (SDHC_BASE_ADDR + 0xE)
#define SDHC_CMD                    VPshort(SDHC_CMD_ADDR)
//#define SDHC_CMD                    SDHC_CMD_ADDR, SDHC_ACCESS, 16
#define RSP_TYPE_SEL_P              0    /* Response Type Select */
#define RSP_TYPE_SEL_S              2
    #define RSP_NONE                    0    /* No Response */
    #define RSP_136                     1    /* Response Length 136 */
    #define RSP_48                      2    /* Response Length 48 */
    #define RSP_48_BUSY                 3    /* Response Length 48 check Busy after response */
#define CMD_CRC_CHK_EN_P            3    /* Command CRC Check Enable */
#define CMD_CRC_CHK_EN_S            1
#define CMD_IDX_CHK_EN_P            4    /* Command Index Check Enable */
#define CMD_IDX_CHK_EN_S            1
#define DATA_PRSNT_SEL_P            5    /* Data Present Select */
#define DATA_PRSNT_SEL_S            1
#define CMD_TYPE_P                  6    /* Command Type */
#define CMD_TYPE_S                  2
    #define CMD_NORMAL                  0    /* Other commands */
    #define CMD_SUSPEND                 1    /* CMD52 for writing “Bus Suspend?in CCCR */
    #define CMD_RESUME                  2    /* CMD52 for writing “Function Select?in CCCR */
    #define CMD_ABORT                   3    /* CMD12, CMD52 for writing “I/O Abort?in CCCR */
#define CMD_IDX_P                   8    /* Command Index */
#define CMD_IDX_S                   6

/* Response Registers */
#define SDHC_RSP0_ADDR              (SDHC_BASE_ADDR + 0x10)
#define SDHC_RSP0                   VPlong(SDHC_RSP0_ADDR)
//#define SDHC_RSP0                   SDHC_RSP0_ADDR, SDHC_ACCESS, 32

#define SDHC_RSP1_ADDR              (SDHC_BASE_ADDR + 0x14)
#define SDHC_RSP1                   VPlong(SDHC_RSP1_ADDR)
//#define SDHC_RSP1                   SDHC_RSP1_ADDR, SDHC_ACCESS, 32

#define SDHC_RSP2_ADDR              (SDHC_BASE_ADDR + 0x18)
#define SDHC_RSP2                   VPlong(SDHC_RSP2_ADDR)
//#define SDHC_RSP2                   SDHC_RSP2_ADDR, SDHC_ACCESS, 32

#define SDHC_RSP3_ADDR              (SDHC_BASE_ADDR + 0x1C)
#define SDHC_RSP3                   VPlong(SDHC_RSP3_ADDR)
//#define SDHC_RSP3                   SDHC_RSP3_ADDR, SDHC_ACCESS, 32

/* Buffer Data Port Register */
#define SDHC_BUF_DATA_ADDR          (SDHC_BASE_ADDR + 0x20)
#define SDHC_BUF_DATA               VPlong(SDHC_BUF_DATA_ADDR)
//#define SDHC_BUF_DATA               SDHC_BUF_DATA_ADDR, SDHC_ACCESS, 32

/* Present State Register */
#define SDHC_PRSNT_STATE_ADDR       (SDHC_BASE_ADDR + 0x24)
#define SDHC_PRSNT_STATE            VPlong(SDHC_PRSNT_STATE_ADDR)
//#define SDHC_PRSNT_STATE            SDHC_PRSNT_STATE_ADDR, SDHC_ACCESS, 32
#define CMD_INHIBIT_P               0    /* Command Inhibit (CMD) */
#define CMD_INHIBIT_S               1
#define DAT_INHIBIT_P               1    /* Command Inhibit (DAT) */
#define DAT_INHIBIT_S               1
#define DAT_ACTIVE_P                2    /* DAT Line Active */
#define DAT_ACTIVE_S                1
#define WRITE_XFR_ACTIVE_P          8    /* Write Transfer Active */
#define WRITE_XFR_ACTIVE_S          1
#define READ_XFR_ACTIVE_P           9    /* Read Transfer Active */
#define READ_XFR_ACTIVE_S           1
#define BUF_WRITE_EN_P              10   /* Buffer Write Enable */
#define BUF_WRITE_EN_S              1
#define BUF_READ_EN_P               11   /* Buffer Read Enable */
#define BUF_READ_EN_S               1
#define CARD_INSERT_P               16   /* Card Insert */
#define CARD_INSERT_S               1
#define CARD_STABLE_P               17   /* Card State Stable */
#define CARD_STABLE_S               1
#define CARD_PIN_LVL_P              18   /* Card Detect Pin Level */
#define CARD_PIN_LVL_S              1
#define WP_PIN_LVL_P                19   /* Write Protect Switch Pin Level */
#define WP_PIN_LVL_S                1
#define DAT_SIG_LVL_P               20   /* DAT[3:0] Line Signal Level */
#define DAT_SIG_LVL_S               4
#define CMD_SIG_LVL_P               24   /* CMD Line Signal Level */
#define CMD_SIG_LVL_S               1

/* Host Control Register */
#define SDHC_HOST_CTRL_ADDR         (SDHC_BASE_ADDR + 0x28)
#define SDHC_HOST_CTRL              VPchar(SDHC_HOST_CTRL_ADDR)
//#define SDHC_HOST_CTRL              SDHC_HOST_CTRL_ADDR, SDHC_ACCESS, 8
#define LED_CTRL_P                  0    /* LED Controller */
#define LED_CTRL_S                  1
#define DATA_XFR_WIDTH_P            1    /* Data Transfer Width */
#define DATA_XFR_WIDTH_S            1
    #define _1_BIT_MODE                 0    /* 1-bit mode */
    #define _4_BIT_MODE                 1    /* 4-bit mode */
#define HIGH_SPEED_EN_P             2    /* High Speed Enable */
#define HIGH_SPEED_EN_S             1

/* Power Controller Register */
#define SDHC_PWR_CTRL_ADDR          (SDHC_BASE_ADDR + 0x29)
#define SDHC_PWR_CTRL               VPchar(SDHC_PWR_CTRL_ADDR)
//#define SDHC_PWR_CTRL               SDHC_PWR_CTRL_ADDR, SDHC_ACCESS, 8
#define BUS_PWR_P                   0    /* Bus Power */
#define BUS_PWR_S                   1
#define VOLAGE_SEL_P                1    /* Voltage select*/
#define VOLAGE_SEL_S                3

/* Block Gap Control Register */
#define SDHC_BLOCK_GAP_CONTROL_ADDR (SDHC_BASE_ADDR + 0x2A)
#define SDHC_BLOCK_GAP_CONTROL      VPchar(SDHC_BLOCK_GAP_CONTROL_ADDR)

/* Wakeup Register */
#define SDHC_WAKEUP_ADDR            (SDHC_BASE_ADDR + 0x2B)
#define SDHC_WAKEUP                 VPchar(SDHC_WAKEUP_ADDR)
//#define SDHC_WAKEUP                 SDHC_WAKEUP_ADDR, SDHC_ACCESS, 8
#define INSERT_WU_EN_P              1    /* Card insertion wakeup enable */
#define INSERT_WU_EN_S              1
#define REMOVE_WU_EN_P              2    /* Card removal wakeup enable */
#define REMOVE_WU_EN_S              1

/* Clock Control Register */
#define SDHC_CLK_CTRL_ADDR          (SDHC_BASE_ADDR + 0x2C)
#define SDHC_CLK_CTRL               VPshort(SDHC_CLK_CTRL_ADDR)
//#define SDHC_CLK_CTRL               SDHC_CLK_CTRL_ADDR, SDHC_ACCESS, 16
#define INTR_CLK_EN_P               0    /* Internal Clock Enable */
#define INTR_CLK_EN_S               1
#define INTR_CLK_STABLE_P           1    /* Internal Clock Stable */
#define INTR_CLK_STABLE_S           1
#define SD_CLK_EN_P                 2    /* SD Clock Enable */
#define SD_CLK_EN_S                 1
#define SDCLK_FREQ_SEL_P            8    /* SDCLK Frequency Select */
#define SDCLK_FREQ_SEL_S            8

/* Timeout Control Register */
#define SDHC_TO_CTRL_ADDR           (SDHC_BASE_ADDR + 0x2E)
#define SDHC_TO_CTRL                VPchar(SDHC_TO_CTRL_ADDR)
//#define SDHC_TO_CTRL                SDHC_TO_CTRL_ADDR, SDHC_ACCESS, 8
#define DATA_TO_CNT_P               0    /* Data Timeout Counter Value */
#define DATA_TO_CNT_S               4

/* Software Reset Register */
#define SDHC_SW_RST_ADDR            (SDHC_BASE_ADDR + 0x2F)
#define SDHC_SW_RST                 VPchar(SDHC_SW_RST_ADDR)
//#define SDHC_SW_RST                 SDHC_SW_RST_ADDR, SDHC_ACCESS, 8
#define SW_RST_ALL_P                0    /* Software Reset for All */
#define SW_RST_ALL_S                1
#define SW_RST_CMD_P                1    /* Software Reset for CMD Line */
#define SW_RST_CMD_S                1
#define SW_RST_DAT_P                2    /* Software Reset for DAT Line */
#define SW_RST_DAT_S                1

/* Normal Interrupt Status Register */
#define SDHC_NRML_INT_STS_ADDR      (SDHC_BASE_ADDR + 0x30)
#define SDHC_NRML_INT_STS           VPshort(SDHC_NRML_INT_STS_ADDR)
//#define SDHC_NRML_INT_STS           SDHC_NRML_INT_STS_ADDR, SDHC_ACCESS, 16
#define CMD_COMPLETE_P              0    /* Command Complete */
#define CMD_COMPLETE_S              1
#define XFR_COMPLETE_P              1    /* Transfer Complete */
#define XFR_COMPLETE_S              1
#define DMA_INT_P                   3    /* DMA Interrupt */
#define DMA_INT_S                   1
#define BUF_WR_READY_P              4    /* Buffer Write Ready */
#define BUF_WR_READY_S              1
#define BUF_RD_READY_P              5    /* Buffer Read Ready */
#define BUF_RD_READY_S              1
#define CARD_INS_P                  6    /* Card Insertion */
#define CARD_INS_S                  1
#define CARD_RMV_P                  7    /* Card Removal */
#define CARD_RMV_S                  1
#define CARD_INT_P                  8    /* Card Interrupt */
#define CARD_INT_S                  1
#define ERR_INT_P                   15   /* Error Interrupt */
#define ERR_INT_S                   1

/* Error Interrupt Status Register */
#define SDHC_ERR_INT_STS_ADDR       (SDHC_BASE_ADDR + 0x32)
#define SDHC_ERR_INT_STS            VPshort(SDHC_ERR_INT_STS_ADDR)
//#define SDHC_ERR_INT_STS            SDHC_ERR_INT_STS_ADDR, SDHC_ACCESS, 16
#define CMD_TO_ERR_P                0    /* Command Timeout Error */
#define CMD_TO_ERR_S                1
#define CMD_CRC_ERR_P               1    /* Command CRC Error */
#define CMD_CRC_ERR_S               1
#define CMD_ENDBIT_ERR_P            2    /* Command End Bit Error */
#define CMD_ENDBIT_ERR_S            1
#define CMD_IDX_ERR_P               3    /* Command Index Error */
#define CMD_IDX_ERR_S               1
    #define ALL_CMD_ERROR_P             0   /* All Command errors */
    #define ALL_CMD_ERROR_S             4
#define DATA_TO_ERR_P               4    /* Data Timeout Error */
#define DATA_TO_ERR_S               1
#define DATA_CRC_ERR_P              5    /* Data CRC Error */
#define DATA_CRC_ERR_S              1
#define DATA_ENDBIT_ERR_P           6    /* Data End Bit Error */
#define DATA_ENDBIT_ERR_S           1
    #define ALL_DATA_ERROR_P            4   /* All Data errors */
    #define ALL_DATA_ERROR_S            7
#define CUR_LIM_ERR_P               7    /* Current Limit Error */
#define CUR_LIM_ERR_S               1
#define AUTO_CMD12_ERR_P            8    /* Auto CMD12 Error */
#define AUTO_CMD12_ERR_S            1
#define VNDR_ERR_P                  12   /* Vendor Specific Error Status */
#define VNDR_ERR_S                  4

/* Normal Interrupt Status Enable Register */
#define SDHC_NRML_INT_STS_EN_ADDR   (SDHC_BASE_ADDR + 0x34)
#define SDHC_NRML_INT_STS_EN        VPshort(SDHC_NRML_INT_STS_EN_ADDR)
//#define SDHC_NRML_INT_STS_EN        SDHC_NRML_INT_STS_EN_ADDR, SDHC_ACCESS, 16
#define CMD_COMPLETE_STS_EN_P       0    /* Command Complete Status Enable */
#define CMD_COMPLETE_STS_EN_S       1
#define XFR_COMPLETE_STS_EN_P       1    /* Transfer Complete Status Enable */
#define XFR_COMPLETE_STS_EN_S       1
#define DMA_ITR_STS_EN_P            3    /* DMA intr */
#define DMA_ITR_STS_EN_S            1
#define BUF_WR_READY_STS_EN_P       4    /* Buffer Write Ready Status Enable */
#define BUF_WR_READY_STS_EN_S       1
#define BUF_RD_READY_STS_EN_P       5    /* Buffer Read Ready Status Enable */
#define BUF_RD_READY_STS_EN_S       1
#define CARD_INSERT_STS_EN_P        6    /* Card Insertion Status Enable */
#define CARD_INSERT_STS_EN_S        1
#define CARD_REMOVE_STS_EN_P        7    /* Card Removal Status Enable */
#define CARD_REMOVE_STS_EN_S        1
#define CARD_INT_STS_EN_P           8    /* Card Interrupt Status Enable */
#define CARD_INT_STS_EN_S           1

#define SDHC_NRML_INT_MASK          \
        ( 1 << CMD_COMPLETE_STS_EN_P    \
        | 1 << XFR_COMPLETE_STS_EN_P    \
        | 1 << BUF_WR_READY_STS_EN_P    \
        | 1 << BUF_RD_READY_STS_EN_P    \
        | 1 << CARD_INSERT_STS_EN_P     \
        | 1 << CARD_REMOVE_STS_EN_P)

/* Error Interrupt Status Enable Register */
#define SDHC_ERR_INT_STS_EN_ADDR    (SDHC_BASE_ADDR + 0x36)
#define SDHC_ERR_INT_STS_EN         VPshort(SDHC_ERR_INT_STS_EN_ADDR)
//#define SDHC_ERR_INT_STS_EN         SDHC_ERR_INT_STS_EN_ADDR, SDHC_ACCESS, 16
#define CMD_TO_ERR_STS_EN_P         0    /* Command Timeout Error Status Enable */
#define CMD_TO_ERR_STS_EN_S         1
#define CMD_CRC_ERR_STS_EN_P        1    /* Command CRC Error Status Enable */
#define CMD_CRC_ERR_STS_EN_S        1
#define CMD_ENDBIT_ERR_STS_EN_P     2    /* Command End Bit Error Status Enable */
#define CMD_ENDBIT_ERR_STS_EN_S     1
#define CMD_IDX_ERR_STS_EN_P        3    /* Command Index Error Status Enable */
#define CMD_IDX_ERR_STS_EN_S        1
#define DATA_TO_ERR_STS_EN_P        4    /* Data Timeout Error Status Enable */
#define DATA_TO_ERR_STS_EN_S        1
#define DATA_CRC_ERR_STS_EN_P       5    /* Data CRC Error Status Enable */
#define DATA_CRC_ERR_STS_EN_S       1
#define DATA_ENDBIT_ERR_STS_EN_P    6    /* Data End Bit Error Status Enable */
#define DATA_ENDBIT_ERR_STS_EN_S    1
#define CUR_LIM_ERR_STS_EN_P        7    /* Current Limit Error Status Enable */
#define CUR_LIM_ERR_STS_EN_S        1
#define AUTO_CMD12_ERR_STS_EN_P     8    /* Auto CMD12 Error Status Enable */
#define AUTO_CMD12_ERR_STS_EN_S     1
#define ADMA_ERR_STS_EN_P           9    /* ADMA Error Status Enable */
#define ADMA_ERR_STS_EN_S           1
#define VNDR_ERR_STS_EN_P           12   /* Vendor Specific Error Status Enable */
#define VNDR_ERR_STS_EN_S           4

#define SDHC_ERR_INT_MASK           \
        ( 1 << CMD_TO_ERR_STS_EN_P      \
        | 1 << CMD_CRC_ERR_STS_EN_P     \
        | 1 << CMD_ENDBIT_ERR_STS_EN_P  \
        | 1 << CMD_IDX_ERR_STS_EN_P     \
        | 1 << DATA_TO_ERR_STS_EN_P     \
        | 1 << DATA_CRC_ERR_STS_EN_P    \
        | 1 << DATA_ENDBIT_ERR_STS_EN_P \
        | 1 << CUR_LIM_ERR_STS_EN_P     \
        | 1 << AUTO_CMD12_ERR_STS_EN_P)

/* Normal Interrupt Signal Enable Register */
#define SDHC_NRML_INT_SIG_EN_ADDR   (SDHC_BASE_ADDR + 0x38)
#define SDHC_NRML_INT_SIG_EN        VPshort(SDHC_NRML_INT_SIG_EN_ADDR)
//#define SDHC_NRML_INT_SIG_EN        SDHC_NRML_INT_SIG_EN_ADDR, SDHC_ACCESS, 16
#define CMD_COMPLETE_SIG_EN_P       0    /* Command Complete Signal Enable */
#define CMD_COMPLETE_SIG_EN_S       1
#define XFR_COMPLETE_SIG_EN_P       1    /* Transfer Complete Signal Enable */
#define XFR_COMPLETE_SIG_EN_S       1
#define DMA_ITR_SIG_EN_P            3    /* DMA intr */
#define DMA_ITR_SIG_EN_S            1
#define BUF_WR_READY_SIG_EN_P       4    /* Buffer Write Ready Signal Enable */
#define BUF_WR_READY_SIG_EN_S       1
#define BUF_RD_READY_SIG_EN_P       5    /* Buffer Read Ready Signal Enable */
#define BUF_RD_READY_SIG_EN_S       1
#define CARD_INSERT_SIG_EN_P        6    /* Card Insertion Signal Enable */
#define CARD_INSERT_SIG_EN_S        1
#define CARD_REMOVE_SIG_EN_P        7    /* Card Removal Signal Enable */
#define CARD_REMOVE_SIG_EN_S        1
#define ERR_INT_SIG_EN_P            15   /* Error Interrupt Signal Enable */
#define ERR_INT_SIG_EN_S            1

/* Error Interrupt Signal Enable Register */
#define SDHC_ERR_INT_SIG_EN_ADDR    (SDHC_BASE_ADDR + 0x3A)
#define SDHC_ERR_INT_SIG_EN         VPshort(SDHC_ERR_INT_SIG_EN_ADDR)
//#define SDHC_ERR_INT_SIG_EN         SDHC_ERR_INT_SIG_EN_ADDR, SDHC_ACCESS, 16
#define CMD_TO_ERR_SIG_EN_P         0    /* Command Timeout Error Signal Enable */
#define CMD_TO_ERR_SIG_EN_S         1
#define CMD_CRC_ERR_SIG_EN_P        1    /* Command CRC Error Signal Enable */
#define CMD_CRC_ERR_SIG_EN_S        1
#define CMD_ENDBIT_ERR_SIG_EN_P     2    /* Command End Bit Error Signal Enable */
#define CMD_ENDBIT_ERR_SIG_EN_S     1
#define CMD_IDX_ERR_SIG_EN_P        3    /* Command Index Error Signal Enable */
#define CMD_IDX_ERR_SIG_EN_S        1
#define DATA_TO_ERR_SIG_EN_P        4    /* Data Timeout Error Signal Enable */
#define DATA_TO_ERR_SIG_EN_S        1
#define DATA_CRC_ERR_SIG_EN_P       5    /* Data CRC Error Signal Enable */
#define DATA_CRC_ERR_SIG_EN_S       1
#define DATA_ENDBIT_ERR_SIG_EN_P    6    /* Data End Bit Error Signal Enable */
#define DATA_ENDBIT_ERR_SIG_EN_S    1
#define CUR_LIM_ERR_SIG_EN_P        7    /* Current Limit Error Signal Enable */
#define CUR_LIM_ERR_SIG_EN_S        1
#define AUTO_CMD12_ERR_SIG_EN_P     8    /* Auto CMD12 Error Signal Enable */
#define AUTO_CMD12_ERR_SIG_EN_S     1
#define VNDR_ERR_SIG_EN_P           12   /* Vendor Specific Error Signal Enable */
#define VNDR_ERR_SIG_EN_S           4

/* Auto CMD12 Error Status Register */
#define SDHC_CMD12_STS_ADDR         (SDHC_BASE_ADDR + 0x3C)
#define SDHC_CMD12_STS              VPshort(SDHC_CMD12_STS_ADDR)
//#define SDHC_CMD12_STS              SDHC_CMD12_STS_ADDR, SDHC_ACCESS, 16
#define AUTO_CMD12_NO_EXEC_P        0    /* Auto CMD12 not executed */
#define AUTO_CMD12_NO_EXEC_S        1
#define AUTO_CMD12_TO_ERR_P         1    /* Auto CMD12 Timeout Error */
#define AUTO_CMD12_TO_ERR_S         1
#define AUTO_CMD12_CRC_ERR_P        2    /* Auto CMD12 CRC Error */
#define AUTO_CMD12_CRC_ERR_S        1
#define AUTO_CMD12_ENDBIT_ERR_P     3    /* Auto CMD12 End Bit Error */
#define AUTO_CMD12_ENDBIT_ERR_S     1
#define AUTO_CMD12_IDX_ERR_P        4    /* Auto CMD12 Index Error */
#define AUTO_CMD12_IDX_ERR_S        1
#define AUTO_CMD12_CMD_NO_EXEC_P    7    /* Command not issued by Auto CMD12 Error */
#define AUTO_CMD12_CMD_NO_EXEC_S    1
    #define ALL_AUTO_CMD12_ERROR_P      0   /* All Auto CMD12 errors */
    #define ALL_AUTO_CMD12_ERROR_S      8

/* Capability Register */
#define SDHC_CAPABILITIES_ADDR      (SDHC_BASE_ADDR + 0x40)
#define SDHC_CAPABILITIES           VPlong(SDHC_CAPABILITIES_ADDR)
#define SDHC_CLOCK_BASE_P           8
#define SDHC_CLOCK_BASE_S           6
#define SDHC_MAX_BLOCK_LEN_P        16
#define SDHC_MAX_BLOCK_LEN_S        2
#define SDHC_CAN_DO_DMA_P           22
#define SDHC_CAN_DO_DMA_S           1
//#define SDHC_CAN_DO_DMA             0x00400000
//#define SDHC_CLOCK_BASE_MASK        0x00003F00
//#define SDHC_CLOCK_BASE_SHIFT       8

/* Max Current Capabilities Register */
#define SDHC_MAX_CUR_CAP_ADDR       (SDHC_BASE_ADDR + 0x48)
#define SDHC_MAX_CUR_CAP            VPlong(SDHC_MAX_CUR_CAP_ADDR)
//#define SDHC_MAX_CUR_CAP            SDHC_MAX_CUR_CAP_ADDR, SDHC_ACCESS, 32
#define _3_3V_MAX_CUR_P             0    /* Maximum Current for 3.3V */
#define _3_3V_MAX_CUR_S             8
#define _3_0V_MAX_CUR_P             8    /* Maximum Current for 3.0V */
#define _3_0V_MAX_CUR_S             8
#define _1_8V_MAX_CUR_P             16   /* Maximum Current for 1.8V */
#define _1_8V_MAX_CUR_S             8

/* MMC Enable Register */
#define SDHC_MMC_EN_ADDR            (SDHC_BASE_ADDR + 0xA0)
#define SDHC_MMC_EN                 VPshort(SDHC_MMC_EN_ADDR)
//#define SDHC_MMC_EN                 SDHC_MMC_EN_ADDR, SDHC_ACCESS, 16
#define MMCEN_P                     0    /* MMC Enable */
#define MMCEN_S                     1

/* Debug Clock Select Register */
#define SDHC_DBG_CLK_SEL_ADDR       (SDHC_BASE_ADDR + 0xA4)
#define SDHC_DBG_CLK_SEL            VPshort(SDHC_DBG_CLK_SEL_ADDR)
//#define SDHC_DBG_CLK_SEL            SDHC_DBG_CLK_SEL_ADDR, SDHC_ACCESS, 16
#define CLKSRC_P                    0    /* Card Detection Debouncer clock source */
#define CLKSRC_S                    1

/* Slot Interrupt Status Register */
#define SDHC_SLOT_INT_STATUS_ADDR   (SDHC_BASE_ADDR + 0xFC)
#define SDHC_SLOT_INT_STATUS        VPshort(SDHC_SLOT_INT_STATUS_ADDR)

/* Host Controller Version Register */
#define SDHC_HOST_VERSION_ADDR      (SDHC_BASE_ADDR + 0xFE)
#define SDHC_HOST_VERSION           VPshort(SDHC_HOST_VERSION_ADDR)


/*-----------------------------------------------------------------------
 * SD/MMC Commands
 *-----------------------------------------------------------------------*/
/* SDHC commands are encoded using 6 bits */
#define CMD_MASK                0x3F
/* In order to differ a CMD and an ACMD having the same command number, 
   we use bit 8 to mark an ACMD */
#define ACMD(cmd)               (0x80 + cmd)
#define COMMAND_IS_ACMD(cmd)    ((UINT8)cmd & 0x80)
/* In order to differ a MMC command from an SD command having the 
   same CMD number, we use bit 7 to mark an SD CMD (Ver. 2.00) */
#define SD_CMD(cmd)             (0x40 + cmd)
#define COMMAND_IS_SD(cmd)      ((UINT8)cmd & 0x40)


/* SD/MMC Commands */
typedef enum SDHC_CMDS {
    /* CMD0   */  SDHC_CMD_GO_IDLE_STATE            = 0,
    /* CMD1   */  SDHC_CMD_SEND_OP_COND             = 1,    /* MMC Only */
    /* CMD2   */  SDHC_CMD_ALL_SEND_CID             = 2,
    /* CMD3   */  SDHC_CMD_SEND_RELATIVE_ADDR       = 3,
    /* CMD4   */  SDHC_CMD_SET_DSR                  = 4,
    /* CMD6   */  SDHC_CMD_SWITCH_FUNC              = 6,
    /* CMD7   */  SDHC_CMD_SELECT_DESELECT_CARD     = 7,
    /* CMD8   */  SDHC_CMD_SEND_EXT_CSD             = 8,    /* MMC Only */
    /* CMD9   */  SDHC_CMD_SEND_CSD                 = 9,
    /* CMD10  */  SDHC_CMD_SEND_CID                 = 10,
    /* CMD11  */  SDHC_CMD_READ_DAT_UNTIL_STOP      = 11,  /* MMC Only */
    /* CMD12  */  SDHC_CMD_STOP_TRANSMISSION        = 12,
    /* CMD13  */  SDHC_CMD_SEND_STATUS              = 13,
    /* CMD14  */  SDHC_CMD_BUSTEST_R                = 14,  /* MMC Only */
    /* CMD15  */  SDHC_CMD_GO_INACTIVE_STATE        = 15,
    /* CMD16  */  SDHC_CMD_SET_BLOCKLEN             = 16,
    /* CMD17  */  SDHC_CMD_READ_SINGLE_BLOCK        = 17,
    /* CMD18  */  SDHC_CMD_READ_MULTIPLE_BLOCK      = 18,
    /* CMD19  */  SDHC_CMD_BUSTEST_W                = 19,   /* MMC Only */
    /* CMD20  */  SDHC_CMD_WRITE_DAT_UNTIL_STOP     = 20,   /* MMC Only */
    /* CMD23  */  SDHC_CMD_SET_BLOCK_COUNT          = 23,   /* MMC Only */
    /* CMD24  */  SDHC_CMD_WRITE_SINGLE_BLOCK       = 24,
    /* CMD25  */  SDHC_CMD_WRITE_MULTIPLE_BLOCK     = 25,
    /* CMD26  */  SDHC_CMD_PROGRAM_CID              = 26,   /* MMC Only */
    /* CMD27  */  SDHC_CMD_PROGRAM_CSD              = 27,
    /* CMD28  */  SDHC_CMD_SET_WRITE_PROT           = 28,
    /* CMD29  */  SDHC_CMD_CLR_WRITE_PROT           = 29,
    /* CMD30  */  SDHC_CMD_SEND_WRITE_PROT          = 30,
    /* CMD32  */  SDHC_CMD_ERASE_WR_BLK_START       = 32,   /* SD Only */
    /* CMD33  */  SDHC_CMD_ERASE_WR_BLK_END         = 33,   /* SD Only */
    /* CMD35  */  SDHC_CMD_ERASE_GROUP_START        = 35,   /* MMC Only */
    /* CMD36  */  SDHC_CMD_ERASE_GROUP_END          = 36,   /* MMC Only */
    /* CMD38  */  SDHC_CMD_ERASE                    = 38,
    /* CMD39  */  SDHC_CMD_FAST_IO                  = 39,   /* MMC Only */
    /* CMD40  */  SDHC_CMD_GO_IRQ_STATE             = 40,   /* MMC Only */
    /* CMD42  */  SDHC_CMD_LOCK_UNLOCK              = 42,
    /* CMD55  */  SDHC_CMD_APP_CMD                  = 55,
    /* CMD56  */  SDHC_CMD_GEN_CMD                  = 56,
    /* The following are SD Only (Ver. 2.00 or later) */
    /* CMD8  */   SDHC_CMD_SEND_IF_COND             = SD_CMD(8),
    /* The following ACMD are SD Only */
    /* ACMD6  */  SDHC_ACMD_SET_BUS_WIDTH           = ACMD(6),
    /* ACMD13 */  SDHC_ACMD_SD_STATUS               = ACMD(13),
    /* ACMD22 */  SDHC_ACMD_SEND_NUM_WR_BLOCKS      = ACMD(22),
    /* ACMD23 */  SDHC_ACMD_SET_WR_BLK_ERASE_COUNT  = ACMD(23),
    /* ACMD41 */  SDHC_ACMD_SD_SEND_OP_COND         = ACMD(41),
    /* ACMD42 */  SDHC_ACMD_SET_CLR_CARD_DETECT     = ACMD(42),
    /* ACMD51 */  SDHC_ACMD_SEND_SCR                = ACMD(51)
} SDHC_CMDS_TYPE;



/*************** CMD8 ****************/

/* SD CMD8 argument fields */
#define SD_CMD8_PATTERN_P       0    /* Check pattern */
#define SD_CMD8_PATTERN_S       8
#define SD_CMD8_VHS_P           8    /* Supply voltage */
#define SD_CMD8_VHS_S           4

/* SD Voltage Supplied (VHS) */
typedef enum SD_CMD8_VHS {
    SD_CMD8_VHS_NONE,    /* Not Defined */
    SD_CMD8_VHS_HIGH,    /* 2.7-3.6V */
    SD_CMD8_VHS_LOW,     /* Low Voltage Range */
} SD_CMD8_VHS_TYPE;

/************** SD CMD6 **************/

/* SD CMD6 Argument fields */
#define SD_CMD6_GROUP_1_P      0    /* Group 1 */
#define SD_CMD6_GROUP_1_S      4
    #define CMD6_FUNC_DEFAULT_SPEED        0x0    /* Default-Speed function */
    #define CMD6_FUNC_HIGH_SPEED           0x1    /* High-Speed function */
#define SD_CMD6_GROUP_2_P      4    /* Group 2 */
#define SD_CMD6_GROUP_2_S      4
    #define CMD6_FUNC_STANDARD_CMD_SET     0x0    /* Standard command set */
    #define CMD6_FUNC_eCOMMERCE_CMD_SET    0x1    /* eCommerce command set */
    #define CMD6_FUNC_VNDR_CMD_SET         0xE    /* Vendor Specific Command set */
#define SD_CMD6_GROUP_3_P      8    /* Group 3 */
#define SD_CMD6_GROUP_3_S      4
#define SD_CMD6_GROUP_4_P      12   /* Group 4 */
#define SD_CMD6_GROUP_4_S      4
#define SD_CMD6_GROUP_5_P      16   /* Group 5 */
#define SD_CMD6_GROUP_5_S      4
#define SD_CMD6_GROUP_6_P      20   /* Group 6 */
#define SD_CMD6_GROUP_6_S      4
#define SD_CMD6_MODE_P         31    /* Mode */
#define SD_CMD6_MODE_S         1
    #define CMD6_CHECK_FUNC        0    /* Check function */
    #define CMD6_SWITCH_FUNC       1    /* Switch function */

#define CMD6_SELECT_DEFAULT_FUNC   0x0    /* Selection of default function */
#define CMD6_SELECT_CURRENT_FUNC   0xF    /* Selection of current function */

/* SD Speed Mode Values */
typedef enum SDHC_SPEED_MODE {
    SD_DEFAULT_SPEED_MODE, /* Default (12.5MB/sec interface speed) */
    SD_HIGH_SPEED_MODE     /* High (25MB/sec interface speed) */
} SDHC_SPEED_MODE;

/* SD Switch function status */
// Maximum current consumption [511:496]
#define SDHC_CMD6_STS_MAX_CUR        ((SDHC_CMD6_STS[0] << 8) + (card_info.csd[1]))
// Function group 6, information [495:480]
#define SDHC_CMD6_STS_GRP6_INFO      ((SDHC_CMD6_STS[2] << 8) + (card_info.csd[3]))
// Function group 5, information [479:464]
#define SDHC_CMD6_STS_GRP5_INFO      ((SDHC_CMD6_STS[4] << 8) + (card_info.csd[5]))
// Function group 4, information [463:448]
#define SDHC_CMD6_STS_GRP4_INFO      ((SDHC_CMD6_STS[6] << 8) + (card_info.csd[7]))
// Function group 3, information [447:432]
#define SDHC_CMD6_STS_GRP3_INFO      ((SDHC_CMD6_STS[8] << 8) + (card_info.csd[9]))
// Function group 2, information [431:416]
#define SDHC_CMD6_STS_GRP2_INFO      ((SDHC_CMD6_STS[10] << 8) + (card_info.csd[11]))
// Function group 1, information [415:400]
#define SDHC_CMD6_STS_GRP1_INFO      ((SDHC_CMD6_STS[12] << 8) + (card_info.csd[13]))
// Function group 6, switched function [399:396]
#define SDHC_CMD6_STS_GRP6_SWITCH    (SDHC_CMD6_STS[14] >> 4)
// Function group 5, switched function [395:392]
#define SDHC_CMD6_STS_GRP5_SWITCH    (SDHC_CMD6_STS[14] & 0x0F)
// Function group 4, switched function [391:388]
#define SDHC_CMD6_STS_GRP4_SWITCH    (SDHC_CMD6_STS[15] >> 4)
// Function group 3, switched function [387:384]
#define SDHC_CMD6_STS_GRP3_SWITCH    (SDHC_CMD6_STS[15] & 0x0F)
// Function group 2, switched function [383:380]
#define SDHC_CMD6_STS_GRP2_SWITCH    (SDHC_CMD6_STS[16] >> 4)
// Function group 1, switched function [379:376]
#define SDHC_CMD6_STS_GRP1_SWITCH    (SDHC_CMD6_STS[16] & 0x0F)

/************** MMC CMD6 *************/

/* MMC CMD6 Argument fields */
#define MMC_CMD6_CMD_SET_P      0    /* Cmd Set */
#define MMC_CMD6_CMD_SET_S      3
#define MMC_CMD6_VALUE_P        8    /* Value */
#define MMC_CMD6_VALUE_S        8
#define MMC_CMD6_INDEX_P        16   /* Index */
#define MMC_CMD6_INDEX_S        8
#define MMC_CMD6_ACCESS_P       24   /* Access */
#define MMC_CMD6_ACCESS_S       2

/* EXT_CSD Access Modes */
typedef enum MMC_CMD6_ACCESS {
    MMC_CMD6_ACCESS_COMMAND_SET,
    MMC_CMD6_ACCESS_SET_BITS,
    MMC_CMD6_ACCESS_CLEAR_BITS,
    MMC_CMD6_ACCESS_WRITE_BYTE
} MMC_CMD6_ACCESS_TYPE;

/********************************* RESPONSES *********************************/

/* SD/MMC Response Types */
typedef enum SDHC_RESPONSE {
    SDHC_RESP_ILLEGAL = 0,
    SDHC_RESP_NONE,
    SDHC_RESP_R1,
    SDHC_RESP_R1b,
    SDHC_RESP_R2,
    SDHC_RESP_R3,
    SDHC_RESP_R4,
    SDHC_RESP_R5,
    SDHC_RESP_R5b,
    SDHC_RESP_R6,
    SDHC_RESP_R7
} SDHC_RESPONSE_TYPE;

/******************************** CARD STATUS ********************************/

/* Card Status fields (32-bit response format R1) */
#define SDHC_STS_AKE_SEQ_ERROR          ((UINT32) 1 << 3)
#define SDHC_STS_APP_CMD                ((UINT32) 1 << 5)
#define SDHC_STS_SWITCH_ERROR           ((UINT32) 1 << 7)
#define SDHC_STS_READY_FOR_DATA         ((UINT32) 1 << 8)
#define SDHC_STS_CURRENT_STATE_P        9
#define SDHC_STS_CURRENT_STATE_S        4
#define SDHC_STS_ERASE_RESET            ((UINT32) 1 << 13)
#define SDHC_STS_CARD_ECC_DISABLED      ((UINT32) 1 << 14)
#define SDHC_STS_WP_ERASE_SKIP          ((UINT32) 1 << 15)
#define SDHC_STS_CID_CSD_OVERWRITE      ((UINT32) 1 << 16)
#define SDHC_STS_OVERRUN                ((UINT32) 1 << 17)
#define SDHC_STS_UNDERRUN               ((UINT32) 1 << 18)
#define SDHC_STS_ERROR                  ((UINT32) 1 << 19)
#define SDHC_STS_CC_ERROR               ((UINT32) 1 << 20)
#define SDHC_STS_CARD_ECC_FAILED        ((UINT32) 1 << 21)
#define SDHC_STS_ILLEGAL_COMMAND        ((UINT32) 1 << 22)
#define SDHC_STS_COM_CRC_ERROR          ((UINT32) 1 << 23)
#define SDHC_STS_LOCK_UNLOCK_FAILED     ((UINT32) 1 << 24)
#define SDHC_STS_CARD_IS_LOCKED         ((UINT32) 1 << 25)
#define SDHC_STS_WP_VIOLATION           ((UINT32) 1 << 26)
#define SDHC_STS_ERASE_PARAM            ((UINT32) 1 << 27)
#define SDHC_STS_ERASE_SEQ_ERROR        ((UINT32) 1 << 28)
#define SDHC_STS_BLOCK_LEN_ERROR        ((UINT32) 1 << 29)
#define SDHC_STS_ADDRESS_ERROR          ((UINT32) 1 << 30)
#define SDHC_STS_OUT_OF_RANGE           ((UINT32) 1 << 31)

/* Mask representing all Error bits in Card Status - MMC R1 Response */
#define SDHC_STS_MMC_R1_ERROR_MASK   (SDHC_STS_OUT_OF_RANGE    |  SDHC_STS_ADDRESS_ERROR \
    | SDHC_STS_BLOCK_LEN_ERROR      | SDHC_STS_ERASE_SEQ_ERROR | SDHC_STS_ERASE_PARAM     | SDHC_STS_WP_VIOLATION \
    | SDHC_STS_LOCK_UNLOCK_FAILED   | SDHC_STS_COM_CRC_ERROR   | SDHC_STS_ILLEGAL_COMMAND | SDHC_STS_CARD_ECC_FAILED \
    | SDHC_STS_CC_ERROR             | SDHC_STS_ERROR           | SDHC_STS_UNDERRUN        | SDHC_STS_OVERRUN \
    | SDHC_STS_CID_CSD_OVERWRITE    | SDHC_STS_WP_ERASE_SKIP   | SDHC_STS_ERASE_RESET     | SDHC_STS_SWITCH_ERROR)

/* Mask representing all Error bits in Card Status - SD R1 Response */
#define SDHC_STS_SD_R1_ERROR_MASK    (SDHC_STS_OUT_OF_RANGE    |  SDHC_STS_ADDRESS_ERROR \
    | SDHC_STS_BLOCK_LEN_ERROR      | SDHC_STS_ERASE_SEQ_ERROR | SDHC_STS_ERASE_PARAM     | SDHC_STS_WP_VIOLATION \
    | SDHC_STS_LOCK_UNLOCK_FAILED   | SDHC_STS_COM_CRC_ERROR   | SDHC_STS_ILLEGAL_COMMAND | SDHC_STS_CARD_ECC_FAILED \
    | SDHC_STS_CC_ERROR             | SDHC_STS_ERROR           | SDHC_STS_CID_CSD_OVERWRITE | SDHC_STS_AKE_SEQ_ERROR)

/* Mask representing all Error bits in Card Status - SD R6 Response:
   Card status bits are: 23, 22, 19, 12-0 (16 bits all together) -
               -------------------------
   bit no.     15  |  14  |  13  |  12-0
               -------------------------
   status no.  23  |  22  |  19  |  12-0
               -------------------------   */
#define SDHC_STS_SD_R6_ERROR_MASK    \
    (((UINT32) 1 << 15)   /* SDHC_STS_COM_CRC_ERROR */    \
   | ((UINT32) 1 << 14)   /* SDHC_STS_ILLEGAL_COMMAND */  \
   | ((UINT32) 1 << 13)   /* SDHC_STS_ERROR */            \
   | ((UINT32) 1 << 3))   /* SDHC_STS_AKE_SEQ_ERROR */

/* Card State */
typedef enum SDHC_STATE {
    SDHC_STATE_IDLE,    /* Idle */
    SDHC_STATE_READY,   /* Ready */
    SDHC_STATE_IDENT,   /* Identification */
    SDHC_STATE_STBY,    /* Stand-by */
    SDHC_STATE_TRAN,    /* Transfer */
    SDHC_STATE_DATA,    /* Sending-data */
    SDHC_STATE_RCV,     /* Receive-data */
    SDHC_STATE_PRG,     /* Programming */
    SDHC_STATE_DIS      /* Disconnect */
} SDHC_STATE_TYPE;

/******************************** CID REGISTER *******************************/

/* MMC CID Register */
typedef struct MMC_CID_DATA {
    UINT8  ManufactureDate;
    UINT8  ProductSerialNumber[4];
    UINT8  ProductVersion;
    UINT8   ProductName[6];
    UINT8  ApplicationID[2];
    UINT8  ManufacturerID;
} MMC_CID_DATA_TYPE;

/* SD CID Register */
typedef struct SD_CID_DATA {
    UINT8  ManufactureDate[2];
    UINT8  ProductSerialNumber[4];
    UINT8  ProductVersion;
    UINT8   ProductName[5];
    UINT8  ApplicationID[2];
    UINT8  ManufacturerID;
} SD_CID_DATA_TYPE;


/******************************** OCR REGISTER *******************************/
/* OCR bit position */
#define OCR_3_2V            ((UINT32) 1 << 20)  /* 3.2-3.3 VDD voltage window */
#define OCR_3_3V            ((UINT32) 1 << 21)  /* 3.3-3.4 VDD voltage window */
#define OCR_HCS             ((UINT32) 1 << 30)  /* Host Capacity Support */
#define OCR_CCS             ((UINT32) 1 << 30)  /* Card Capacity Status */
#define OCR_BUSY            ((UINT32) 1 << 31)  /* Card power up status bit */

/* Card Capacity */
typedef enum SDHC_CAPACITY {
    SDHC_CAPACITY_STANDARD,     /* Up to and including 2G bytes */
    SDHC_CAPACITY_HIGH          /* More than 2G bytes limited up to and including 32GB */
} SDHC_CAPACITY_TYPE;

/* In High Capacity Cards, block length is fixed to 512 bytes */
#define SDHC_HCS_BLOCK_LENGTH   512

/* Default VDD voltage window */
#define OCR_DEFAULT            (OCR_3_2V|OCR_3_3V)


/******************************** CSD REGISTER *******************************/
/* Please note that CRC is already omitted from response, 
   hence all offsets are decremented by 1 */

/* SD ver. 1.X/MMC Common register fields */
// CSD structure [127:126]
#define CSD_STRUCTURE           (card_info.csd[14] >> 6)
// data read access-time-1 [119:112]
#define CSD_TAAC                (card_info.csd[13])
// Data read access-time 2 in CLK cycles (NSAC*100) [111:104]
#define CSD_NSAC                (card_info.csd[12])
// Max. bus clock frequency [103:96]
#define CSD_TRAN_SPEED          (card_info.csd[11])
#define CSD_TRAN_SPEED_UNIT_P   0
#define CSD_TRAN_SPEED_UNIT_S   3
#define CSD_TRAN_SPEED_TIME_VALUE_P   3
#define CSD_TRAN_SPEED_TIME_VALUE_S   4
// card command classes [95:84]
#define CSD_CCC                 ((card_info.csd[10] << 4) + (card_info.csd[9] >> 4))
// Max. read data block length [83:80]
#define CSD_READ_BL_LEN         (card_info.csd[9] & 0x0F)
// Partial blocks for read allowed [79:79]
#define CSD_READ_BL_PARTIAL     ((card_info.csd[8] & 0x80) >> 7)
// Write block misalignment [78:78]
#define CSD_WRITE_BLK_MISALIGN  ((card_info.csd[8] & 0x40) >> 6)
// Read block misalignment [77:77]
#define CSD_READ_BLK_MISALIGN   ((card_info.csd[8] & 0x20) >> 5)
// DSR implemented [76:76]
#define CSD_DSR_IMP             ((card_info.csd[8] & 0x10) >> 4)
// Device size [73:62]
#define CSD_C_SIZE              (((card_info.csd[8] & 0x03) << 10) + (card_info.csd[7] << 2) + (card_info.csd[6] >> 6))
// Max. read current @ VDD min [61:59]
#define CSD_VDD_R_CURR_MIN      ((card_info.csd[6] & 0x38) >> 3)
// Max. read current @ VDD max [58:56]
#define CSD_VDD_R_CURR_MAX      (card_info.csd[6] & 0x07)
// Max. write current @ VDD min [55:53]
#define CSD_VDD_W_CURR_MIN      ((card_info.csd[5] & 0xE0) >> 5)
// Max. write current @ VDD max [52:50]
#define CSD_VDD_W_CURR_MAX      ((card_info.csd[5] & 0x1C) >> 2)
// Device size multiplier [49:47]
#define CSD_C_SIZE_MULT         (((card_info.csd[5] & 0x03) << 1) + (card_info.csd[4] >> 7))
// Write protect group enable [31:31]
#define CSD_WP_GRP_ENABLE       ((card_info.csd[2] & 0x80) >> 7)
// Write speed factor [28:26]
#define CSD_R2W_FACTOR          ((card_info.csd[2] & 0x1C) >> 2)
// Max. write data block length [25:22]
#define CSD_WRITE_BL_LEN        (((card_info.csd[2] & 0x03) << 2) + (card_info.csd[1] >> 6))
// Partial blocks for write allowed [21:21]
#define CSD_WRITE_BL_PARTIAL    ((card_info.csd[1] & 0x20) >> 5)
// File format group [15:15]
#define CSD_FILE_FORMAT_GRP     ((card_info.csd[0] & 0x80) >> 7)
// Copy flag (OTP) [14:14]
#define CSD_COPY                ((card_info.csd[0] & 0x40) >> 6)
// Permanent write protection [13:13]
#define CSD_PERM_WRITE_PROTECT  ((card_info.csd[0] & 0x20) >> 5)
// Temporary write protection [12:12]
#define CSD_TMP_WRITE_PROTECT   ((card_info.csd[0] & 0x10) >> 4)
// File format [11:10]
#define CSD_FILE_FORMAT         ((card_info.csd[0] & 0x0C) >> 2)

/* MMC-Specific fields */
// System specification version [125:122]
#define CSD_MMC_SPEC_VERS       ((card_info.csd[14] & 0x3C) >> 2)
// Erase group size [46:42]
#define CSD_MMC_ERASE_GRP_SIZE  ((card_info.csd[4] & 0x7C) >> 2)
// Erase group size multiplier [41:37]
#define CSD_MMC_ERASE_GRP_MULT  (((card_info.csd[4] & 0x03) << 3) + (card_info.csd[3] >> 5))
// Write protect group size [36:32]
#define CSD_MMC_WP_GRP_SIZE     (card_info.csd[3] & 0x1F)
// Manufacturer default ECC [30:29]
#define CSD_MMC_DEFAULT_ECC     ((card_info.csd[2] & 0x60) >> 5)
// Content protection application [16:16]
#define CSD_MMC_CONTENT_PROT_APP (card_info.csd[1] & 0x01)
// ECC code [9:8]
#define CSD_MMC_ECC             (card_info.csd[0] & 0x03)

/* SD-Specific fields */
// Erase single block enable [46:46]
#define CSD_SD_ERASE_BLK_EN     ((card_info.csd[4] & 0x40) >> 6)
// Erase sector size [45:39]
#define CSD_SD_SECTOR_SIZE      (((card_info.csd[4] & 0x3F) << 1) + (card_info.csd[3] >> 7))
// Write protect group size [38:32]
#define CSD_SD_WP_GRP_SIZE      (card_info.csd[3] & 0x7F)

/* SD-Specific High Capacity fields */
// Device size [69:48]
#define CSD_SD_HCS_C_SIZE       (((card_info.csd[7] & 0x3F) << 16) + (card_info.csd[6] << 8) + card_info.csd[5])

/* SD CSD register structure versions */
typedef enum SD_CSD_STRUCTURE {
    SD_CSD_VER_1,  /* Version 1.01-1.10, Version 2.00/Standard Capacity */
    SD_CSD_VER_2   /* Version 2.00/High Capacity */
} SD_CSD_STRUCTURE_TYPE;

/* MMC CSD register structure versions */
typedef enum MMC_CSD_STRUCTURE {
    MMC_CSD_VER_1_0,  /* Version 1.0 - 1.2 */
    MMC_CSD_VER_1_1,  /* Version 1.4 - 2.2 */
    MMC_CSD_VER_1_2,  /* Version 3.1 - 3.2 - 3.31 - 4.0 - 4.1 */
    MMC_CSD_EXT_CSD   /* Version is coded in the CSD_STRUCTURE byte in the EXT_CSD register */
} MMC_CSD_STRUCTURE_TYPE;

/* MMC System Specification versions */
typedef enum MMC_SPEC_VERS {
    MMC_SPEC_VER_1,     /* Version 1.0-1.2 */
    MMC_SPEC_VER_1_4,   /* Version 1.4 */
    MMC_SPEC_VER_2,     /* Version 2.0 - 2.2 */
    MMC_SPEC_VER_3,     /* Version 3.1 - 3.2 -3.31 */
    MMC_SPEC_VER_4      /* Version 4.0 - 4.1 */
} MMC_SPEC_VERS_TYPE;


/****************************** EXT_CSD REGISTER *****************************/

/* EXT_CSD register field offsets */
typedef enum EXT_CSD_INDEX {
    /* Various offsets in Modes Segment */
    EXT_CSD_BUS_WIDTH       = 183,    /* Bus Width Mode */
    EXT_CSD_HS_TIMING       = 185,    /* High Speed Interface Timing */
    EXT_CSD_POWER_CLASS     = 187,    /* Power Class */
    EXT_CSD_CMD_SET_REV     = 189,    /* Command Set Revision */
    EXT_CSD_CMD_SET         = 191,    /* Command Set */
    /* Various offsets in Properties Segment */
    EXT_CSD_EXT_CSD_REV     = 192,    /* Extended CSD Revision */
    EXT_CSD_CSD_STRUCTURE   = 194,    /* CSD Structure Version */
    EXT_CSD_CARD_TYPE       = 196,    /* Card Type */
    EXT_CSD_S_CMD_SET       = 504,    /* Supported Command Sets */
} EXT_CSD_INDEX_TYPE;

/* MMC Card Supported Command Sets */
typedef enum MMC_CMD_SET {
    STANDARD_MMC,
    SECURE_MMC,
    CONTENT_PROTECTION_SECURE_MMC
} MMC_CMD_SET_TYPE;

/* MMC Bus Mode Values */
typedef enum MMC_BUS_WIDTH {
    _1_BIT_DATA_MMC_BUS,
    _4_BIT_DATA_MMC_BUS,
    _8_BIT_DATA_MMC_BUS,
} MMC_BUS_WIDTH_TYPE;


/******************************** SCR REGISTER *******************************/

// SCR structure [63:60]
#define SCR_STRUCTURE                (card_info.scr[0] >> 4)
// SD Memory Card - Spec. Version [59:56]
#define SCR_SD_SPEC                  (card_info.scr[0] & 0x0F)
// Data_status_after erases [55:55]
#define SCR_DATA_STAT_AFTER_ERASE   ((card_info.scr[1] & 0x80) >> 7)
// SD Security Support [54:52]
#define SCR_SD_SECURITY             ((card_info.scr[1] & 0x70) >> 4)
// DAT Bus widths supported [51:48]
#define SCR_SD_BUS_WIDTHS            (card_info.scr[1] & 0x0F)

/* SD System Specification versions */
typedef enum SD_SPEC_VERS {
    SD_SPEC_VER_1_0,    /* Version 1.0-1.01 */
    SD_SPEC_VER_1_1,    /* Version 1.10 */
    SD_SPEC_VER_2_0,    /* Version 2.00 */
} SD_SPEC_VERS_TYPE;

/* SD Bus Mode Values */
typedef enum SD_BUS_WIDTH {
    _1_BIT_DATA_SD_BUS = 0, /* 1 bit (DAT0) */
    _4_BIT_DATA_SD_BUS = 2, /* 4 bit (DAT0-3) */
} SD_BUS_WIDTH_TYPE;


/******************************** SSR REGISTER *******************************/

// Currently defined data bus width [511:510]
#define SSR_DAT_BUS_WIDTH            (SDHC_SSR[0] >> 6)
// Secured Mode of operation [509:509]
#define SSR_SECURED_MODE            ((SDHC_SSR[0] & 0x20) >> 5)
// SD Memory Cards [495:480]
#define SSR_SD_CARD_TYPE            ((SDHC_SSR[2] << 8) + SDHC_SSR[3])
// Size of protected area [479:448]
#define SSR_SIZE_OF_PROTECTED_AREA  ((SDHC_SSR[4] << 24) + (SDHC_SSR[5] << 16) + \
                                     (SDHC_SSR[6] << 8)  +  SDHC_SSR[7])
// Speed Class of the card [447:440]
#define SSR_SPEED_CLASS               SDHC_SSR[8]
// Performance of move indicated by 1 [MB/s] step. [439:432]
#define SSR_PERFORMANCE_MOVE          SDHC_SSR[9]
// Currently defined data bus width [431:428]
#define SSR_AU_SIZE                  (SDHC_SSR[10] >> 4)
// Number of AUs to be erased at a time [423:408]
#define SSR_ERASE_SIZE              ((SDHC_SSR[11] << 8) + SDHC_SSR[12])
// Timeout value for erasing areas specified by UNIT_OF_ERASE_AU [407:402]
#define SSR_ERASE_TIMEOUT            (SDHC_SSR[13] >> 2)
// Fixed offset value added to erase time [401:400]
#define SSR_ERASE_OFFSET             (SDHC_SSR[13] & 0x03)

/* SD Speed Class Code */
typedef enum SD_SPEED_CLASS {
    SD_SPEED_CLASS_0,    /* Class 0 - do not specify performance */
    SD_SPEED_CLASS_2,    /* Class 2 - more than or equal to 2 MB/sec performance */
    SD_SPEED_CLASS_4,    /* Class 4 - more than or equal to 4 MB/sec performance */
    SD_SPEED_CLASS_6     /* Class 6 - more than or equal to 6 MB/sec performance */
} SD_SPEED_CLASS_TYPE;


/*-----------------------------------------------------------------------
 * 
 *-----------------------------------------------------------------------*/
struct mmc_data
{
    SDHC_CMDS_TYPE cmd_idx;
    UINT32 arg;
    
    UINT32 resp[4];
    
    UINT8 *buffer;
    
    UINT32 blocks;
    
    /* internal use  for command register */
    UINT8 cmd_type;
    UINT8 date_prsnt_sel;
    UINT8 cmd_idx_chk_en;
    UINT8 cmd_crc_chk_en;
    UINT8 rsp_type_sel;
    UINT8 is_writing;
    UINT8 show_info;
};


struct mmc_card_info
{
    /* Type of current inserted card */
    CARD_TYPE_TYPE card_type;
    
    /* OCR register */
    UINT32 ocr;
    
    /* clock frequency */
    UINT32 selected_clock;
    
    /* card size, unit in block */
    UINT32 size;
    
    /* Card IDentification register (128 bits wide) */
    UINT8 cid[16];
    
    /* Card-Specific Data register (128 bits wide) */
    UINT8 csd[16];
    
    /* SD card Configuration Register (64 bits wide) */
    UINT8 scr[8];
    
    /* Relative Card Address register (16 bits wide) */
    UINT16 rca;
    
    /* card version */
    UINT8 version;
    
    UINT8 card_inserted;
    UINT8 write_protected;
    UINT8 current_state;
};
