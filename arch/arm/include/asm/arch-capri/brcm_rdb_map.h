/************************************************************************************************/
/*                                                                                              */
/*  Copyright 2010 Broadcom Corporation                                                         */
/*                                                                                              */
/*     Unless you and Broadcom execute a separate written software license agreement governing  */
/*     use of this software, this software is licensed to you under the terms of the GNU        */
/*     General Public License version 2 (the GPL), available at                                 */
/*                                                                                              */
/*          http://www.broadcom.com/licenses/GPLv2.php                                          */
/*                                                                                              */
/*     with the following added to such license:                                                */
/*                                                                                              */
/*     As a special exception, the copyright holders of this software give you permission to    */
/*     link this software with independent modules, and to copy and distribute the resulting    */
/*     executable under terms of your choice, provided that you also meet, for each linked      */
/*     independent module, the terms and conditions of the license of that module.              */
/*     An independent module is a module which is not derived from this software.  The special  */
/*     exception does not apply to any modifications of the software.                           */
/*                                                                                              */
/*     Notwithstanding the above, under no circumstances may you combine this software in any   */
/*     way with any other Broadcom software provided under a license other than the GPL,        */
/*     without Broadcom's express prior written consent.                                        */
/*                                                                                              */
/************************************************************************************************/

#include <asm/arch/brcm_rdb_padctrlreg.h>

/* RDB mnemonic mapping between Hana and BigIsland */
/* Note: BigIsland mnemonics are used by CSP code  */

#define UARTB1_BASE_ADDR                     UARTB_BASE_ADDR
#define SYS_EMI_SECURE_BASE_ADDR             MEMC0_SECURE_BASE_ADDR
#define SYS_EMI_OPEN_BASE_ADDR               MEMC0_OPEN_BASE_ADDR
#define SYS_EMI_OPEN_PWRWDOG_BASE_ADDR       MEMC0_OPEN_PWRWDOG_BASE_ADDR
#define SYS_EMI_DDR3_CTL_BASE_ADDR           MEMC0_OPEN_APHY_BASE_ADDR
#define SYS_EMI_DDR3_PHY_ADDR_CTL_BASE_ADDR  MEMC0_OPEN_DPHY_BASE_ADDR
#define ARM_FUNNEL_BASE_ADDR                 FUNNEL_BASE_ADDR

/* TODO: DDR3 and LPDDR2 register space does not shadow on Capri so pretty sure this is wrong */
#define VC4_EMI_DDR3_CTL_BASE_ADDR           VC4_EMI_OPEN_APHY_BASE_ADDR
#define VC4_EMI_DDR3_PHY_ADDR_CTL_BASE_ADDR  VC4_EMI_OPEN_DPHY_BASE_ADDR

#define HSOTG_CTRL_PHY_P1CTL_PLL_SUSPEND_ENABLE_MASK  HSOTG_CTRL_PHY_P1CTL_USB11_OEB_IS_TXEB_MASK
#define HSOTG_CTRL_BC11_STATUS_OFFSET                 HSOTG_CTRL_BC12_STATUS_OFFSET
#define HSOTG_CTRL_BC11_STATUS_SHP_MASK               HSOTG_CTRL_BC12_STATUS_SDP_MASK
#define HSOTG_CTRL_BC11_CFG_OFFSET                    HSOTG_CTRL_BC12_CFG_OFFSET
#define HSOTG_CTRL_BC11_CFG_BC11_OVWR_KEY_MASK        HSOTG_CTRL_BC12_CFG_BC12_OVWR_KEY_MASK
#define HSOTG_CTRL_BC11_CFG_SW_OVWR_EN_MASK           HSOTG_CTRL_BC12_CFG_SW_OVWR_EN_MASK
#define HSOTG_CTRL_BC11_CFG_BC11_OVWR_SET_M0_MASK     HSOTG_CTRL_BC12_CFG_BC12_OVWR_SET_M0_MASK
#define HSOTG_CTRL_BC11_CFG_BC11_OVWR_SET_P0_MASK     HSOTG_CTRL_BC12_CFG_BC12_OVWR_SET_P0_MASK

/**** CHIPREG Peripheral Spare ****/
#define CHIPREG_PERIPH_SPARE_REG2_OFFSET     CHIPREG_PERIPH_MISC_REG2_OFFSET


#define    CHIPREG_VC_CAM1_SCL_PUP_SHIFT     PADCTRLREG_VC_CAM1_SCL_PUP_2_0_SHIFT
#define    CHIPREG_VC_CAM1_SCL_PUP_MASK      PADCTRLREG_VC_CAM1_SCL_PUP_2_0_MASK

/* Missing definitions */
#define NVSRAM_BASE_ADDR          0x34003000 /* brcm_rdb_nvsram_axi.h */
#define SSP1_BASE_ADDR            0x35028000 /* brcm_rdb_sspil.h */
#define D1W_BASE_ADDR             0x3E015000 /* brcm_rdb_d1w.h */
#define VINTC_APB_BASE_ADDR       0x38002000 /* brcm_rdb_vintc_apb.h */
#define AXITRACE21_BASE_ADDR      0x3800D000 /* brcm_rdb_axitp1.h */

/* BBL no longer exists in Capri, so this will need to be removed/changed */
#define BBL_WATCHDOG_BASE_ADDR    0x00004000 /* brcm_rdb_secwatchdog.h */
#define BBL_BASE_ADDR             0x3E01B000 /* brcm_rdb_bbl_apb.h */

#define    CPH_AADMAC_CH1_AADMAC_CR_2_CH1_AADMAC_DONT_PAUSE_SHIFT         27
#define    CPH_AADMAC_CH1_AADMAC_CR_2_CH1_AADMAC_DONT_PAUSE_MASK          0x08000000

/* Capri does not support D1W, so this can be removed once the build is cleaned-up to remove D1W from testapps */
#define    KONATZCFG_KONA_SLV_APB2_TZPROT_DALLAS_1_WIRE_TZPROT_MASK       0x00000020

#define    CHIPREG_VC_CAM1_SCL_SEL_SHIFT                                  0
#define    CHIPREG_VC_CAM1_SCL_SEL_MASK                                   0x00000007

#define    AUDIOH_AUDIORX_VMIC_AUDIORX_VMIC_CTRL_SHIFT                    8
#define    AUDIOH_AUDIORX_VMIC_AUDIORX_VMIC_CTRL_MASK                     0x00000300

#define ACI_ADC_CTRL_OFFSET                                               0x000000D4
#define    ACI_ADC_CTRL_AUDIORX_VREF_PWRUP_MASK                           0x00000080
#define    ACI_ADC_CTRL_AUDIORX_BIAS_PWRUP_MASK                           0x00000040

#define    CPH_AADMAC_CH1_AADMAC_CR_2_CH1_SW_READY_HIGH_SHIFT             31
#define    CPH_AADMAC_CH1_AADMAC_CR_2_CH1_SW_READY_HIGH_MASK              0x80000000
#define    CPH_AADMAC_CH1_AADMAC_CR_2_CH1_SW_READY_LOW_SHIFT              30
#define    CPH_AADMAC_CH1_AADMAC_CR_2_CH1_SW_READY_LOW_MASK               0x40000000
#define    CPH_AADMAC_CH1_AADMAC_CR_2_CH1_AADMAC_FIFO_RST_MASK            0x04000000

#define EHCI_MODE_OFFSET                                                  0x00008000

#define NON_DMAC_INTEN_OFFSET                                             0x00000020

#define KEYPAD_KPIOC_OFFSET                                               0x00000008
#define    KEYPAD_KPIOC_ROWOCONTRL_SHIFT                                  16

#define    CHIPREG_SDIO2_DATA_3_PUP_MASK                                  0x00000020

#define KONATZCFG_KONA_PERIPH_AHB2_TZPROT_OFFSET                          0x00000204
#define    KONATZCFG_KONA_PERIPH_AHB2_TZPROT_UNMG_NAND_TZPROT_MASK        0x00000002
#define    KONATZCFG_KONA_PERIPH_AHB2_TZPROT_SDIO4_TZPROT_MASK            0x00000800
#define    KONATZCFG_KONA_PERIPH_AHB2_TZPROT_SDIO3_TZPROT_MASK            0x00000400
#define    KONATZCFG_KONA_PERIPH_AHB2_TZPROT_SDIO2_TZPROT_MASK            0x00000200
#define    KONATZCFG_KONA_PERIPH_AHB2_TZPROT_SDIO1_TZPROT_MASK            0x00000100

#define    IKPS_CLK_MGR_REG_DIV_TRIG_SSP0_AUDIO_TRIGGER_MASK              0x00001000
#define    IKPS_CLK_MGR_REG_DIV_TRIG_SSP2_AUDIO_TRIGGER_MASK              0x00004000
#define    KHUB_CLK_MGR_REG_PERIPH_SEG_TRG_SSP3_AUDIO_TRIGGER_MASK        0x00008000
#define    KHUB_CLK_MGR_REG_PERIPH_SEG_TRG_SSP4_AUDIO_TRIGGER_MASK        0x00040000

//#define HSOTG_CTRL_PHY_P1CTL_PLL_SUSPEND_ENABLE_MASK     HSOTG_CTRL_PHY_P1CTL_USB11_TX_EN_DURING_XMT_MASK

#define CHIPREG_GPIO_0_OFFSET                   PADCTRLREG_GPIO00_OFFSET
#define CHIPREG_GPIO_0_TYPE                     PADCTRLREG_GPIO00_TYPE
#define CHIPREG_GPIO_0_RESERVED_MASK            PADCTRLREG_GPIO00_RESERVED_MASK
#define    CHIPREG_GPIO_0_PINSEL_2_0_SHIFT         PADCTRLREG_GPIO00_PINSEL_2_0_SHIFT
#define    CHIPREG_GPIO_0_PINSEL_2_0_MASK          PADCTRLREG_GPIO00_PINSEL_2_0_MASK
#define    CHIPREG_GPIO_0_HYS_EN_SHIFT             PADCTRLREG_GPIO00_HYS_EN_SHIFT
#define    CHIPREG_GPIO_0_HYS_EN_MASK              PADCTRLREG_GPIO00_HYS_EN_MASK
#define    CHIPREG_GPIO_0_PDN_SHIFT                PADCTRLREG_GPIO00_PDN_SHIFT
#define    CHIPREG_GPIO_0_PDN_MASK                 PADCTRLREG_GPIO00_PDN_MASK
#define    CHIPREG_GPIO_0_PUP_SHIFT                PADCTRLREG_GPIO00_PUP_SHIFT
#define    CHIPREG_GPIO_0_PUP_MASK                 PADCTRLREG_GPIO00_PUP_MASK
#define    CHIPREG_GPIO_0_SRC_SHIFT                PADCTRLREG_GPIO00_SRC_SHIFT
#define    CHIPREG_GPIO_0_SRC_MASK                 PADCTRLREG_GPIO00_SRC_MASK
#define    CHIPREG_GPIO_0_IND_SHIFT                PADCTRLREG_GPIO00_IND_SHIFT
#define    CHIPREG_GPIO_0_IND_MASK                 PADCTRLREG_GPIO00_IND_MASK
#define    CHIPREG_GPIO_0_SEL_SHIFT                PADCTRLREG_GPIO00_SEL_SHIFT
#define    CHIPREG_GPIO_0_SEL_MASK                 PADCTRLREG_GPIO00_SEL_MASK

#define    CHIPREG_VC_CAM1_SCL_SRC_SHIFT           PADCTRLREG_VC_CAM1_SCL_SRC_SHIFT
#define    CHIPREG_VC_CAM1_SCL_SRC_MASK            PADCTRLREG_VC_CAM1_SCL_SRC_MASK

#define    CHIPREG_VC_CAM1_SCL_IND_SHIFT           PADCTRLREG_VC_CAM1_SCL_IND_SHIFT
#define    CHIPREG_VC_CAM1_SCL_IND_MASK            PADCTRLREG_VC_CAM1_SCL_IND_MASK

#define CHIPREG_UARTB_UCTSN_OFFSET        CHIPREG_UARTB1_UCTS_OFFSET
#define CHIPREG_UARTB_URTSN_OFFSET        CHIPREG_UARTB1_URTS_OFFSET
#define CHIPREG_UARTB_UTXD_OFFSET         CHIPREG_UARTB1_UTXD_OFFSET
#define CHIPREG_UARTB_URXD_OFFSET         CHIPREG_UARTB1_URXD_OFFSET

/**** DMACHW ****/
#define  DMACHW_RDB_MAP_MODULE_0_NUM_OF_CHANNELS            4 /**< Number of Channels supported by Module 0 */
#define  DMACHW_RDB_MAP_MODULE_1_NUM_OF_CHANNELS            0 /**< Number of Channels supported by Module 1 */

#define  DMACHW_RDB_MAP_REG_INT_RAW_BASE_CHANNEL(module)    8 /**< Used for calculating register offset */


/* TODO: Clean-up everything below this line */

/* The definitions below this line are stictly to maintain sw compatibility
*  with existing code that is based on the BI RDB
*/

/* GPIO MUX definitions that need to be updated for Capri */
#define CHIPREG_GPIO_1_OFFSET             0
#define CHIPREG_GPIO_2_OFFSET             0
#define CHIPREG_GPIO_3_OFFSET             0
#define CHIPREG_GPIO_4_OFFSET             0
#define CHIPREG_GPIO_5_OFFSET             0
#define CHIPREG_GPIO_6_OFFSET             0
#define CHIPREG_GPIO_7_OFFSET             0
#define CHIPREG_ARM_SLB_DATA_OFFSET       0
#define CHIPREG_ARM_SLB_CLK_OFFSET        0
#define CHIPREG_SSP3_EXTCLK_OFFSET        0
#define CHIPREG_SSP1_TXD_OFFSET           0
#define CHIPREG_SSP1_RXD_OFFSET           0
#define CHIPREG_SSP1_CLK_OFFSET           0
#define CHIPREG_SSP1_FS_OFFSET            0
#define CHIPREG_SDIO3_CLK_OFFSET          0
#define CHIPREG_SDIO3_CMD_OFFSET          0
#define CHIPREG_SDIO3_DATA_0_OFFSET       0
#define CHIPREG_SDIO3_DATA_1_OFFSET       0
#define CHIPREG_SDIO3_DATA_2_OFFSET       0
#define CHIPREG_SDIO3_DATA_3_OFFSET       0
#define CHIPREG_SDIO2_CLK_OFFSET          0
#define CHIPREG_SDIO2_CMD_OFFSET          0
#define CHIPREG_SDIO2_DATA_0_OFFSET       0
#define CHIPREG_SDIO2_DATA_1_OFFSET       0
#define CHIPREG_SDIO2_DATA_2_OFFSET       0
#define CHIPREG_SDIO2_DATA_3_OFFSET       0
#define CHIPREG_ULPI1_STP_OFFSET          0
#define CHIPREG_ULPI1_NXT_OFFSET          0
#define CHIPREG_ULPI1_DIR_OFFSET          0
#define CHIPREG_ULPI1_DATA_7_OFFSET       0
#define CHIPREG_ULPI1_DATA_6_OFFSET       0
#define CHIPREG_ULPI1_DATA_5_OFFSET       0
#define CHIPREG_ULPI1_DATA_4_OFFSET       0
#define CHIPREG_ULPI1_DATA_3_OFFSET       0
#define CHIPREG_ULPI1_DATA_2_OFFSET       0
#define CHIPREG_ULPI1_DATA_1_OFFSET       0
#define CHIPREG_ULPI1_DATA_0_OFFSET       0
#define CHIPREG_ULPI1_CLOCK_OFFSET        0
#define CHIPREG_ULPI0_STP_OFFSET          0
#define CHIPREG_ULPI0_NXT_OFFSET          0
#define CHIPREG_ULPI0_DIR_OFFSET          0
#define CHIPREG_ULPI0_DATA_7_OFFSET       0
#define CHIPREG_ULPI0_DATA_6_OFFSET       0
#define CHIPREG_ULPI0_DATA_5_OFFSET       0
#define CHIPREG_ULPI0_DATA_4_OFFSET       0
#define CHIPREG_ULPI0_DATA_3_OFFSET       0
#define CHIPREG_ULPI0_DATA_2_OFFSET       0
#define CHIPREG_ULPI0_DATA_1_OFFSET       0
#define CHIPREG_ULPI0_DATA_0_OFFSET       0
#define CHIPREG_ULPI0_CLOCK_OFFSET        0
#define CHIPREG_NORFLSH_CLK_N_OFFSET      0
#define CHIPREG_NORFLSH_RDY_OFFSET        0
#define CHIPREG_NORFLSH_AD_00_OFFSET      0
#define CHIPREG_NORFLSH_AD_01_OFFSET      0
#define CHIPREG_NORFLSH_AD_02_OFFSET      0
#define CHIPREG_NORFLSH_AD_03_OFFSET      0
#define CHIPREG_NORFLSH_AD_04_OFFSET      0
#define CHIPREG_NORFLSH_AD_05_OFFSET      0
#define CHIPREG_NORFLSH_AD_06_OFFSET      0
#define CHIPREG_NORFLSH_AD_07_OFFSET      0
#define CHIPREG_NORFLSH_AD_08_OFFSET      0
#define CHIPREG_NORFLSH_AD_09_OFFSET      0
#define CHIPREG_NORFLSH_AD_10_OFFSET      0
#define CHIPREG_NORFLSH_AD_11_OFFSET      0
#define CHIPREG_NORFLSH_AD_12_OFFSET      0
#define CHIPREG_NORFLSH_AD_13_OFFSET      0
#define CHIPREG_NORFLSH_AD_14_OFFSET      0
#define CHIPREG_NORFLSH_AD_15_OFFSET      0
#define CHIPREG_NORFLSH_ADLAT_EN_OFFSET   0
#define CHIPREG_NORFLSH_AADLAT_EN_OFFSET  0
#define CHIPREG_NORFLSH_ADDR_16_OFFSET    0
#define CHIPREG_NORFLSH_ADDR_17_OFFSET    0
#define CHIPREG_NORFLSH_ADDR_18_OFFSET    0
#define CHIPREG_NORFLSH_ADDR_19_OFFSET    0
#define CHIPREG_NORFLSH_ADDR_20_OFFSET    0
#define CHIPREG_NORFLSH_ADDR_21_OFFSET    0
#define CHIPREG_NORFLSH_ADDR_22_OFFSET    0
#define CHIPREG_NORFLSH_ADDR_23_OFFSET    0
#define CHIPREG_NORFLSH_OE_N_OFFSET       0
#define CHIPREG_NORFLSH_CE0_N_OFFSET      0
#define CHIPREG_NORFLSH_CE1_N_OFFSET      0
#define CHIPREG_NORFLSH_WE_N_OFFSET       0
#define CHIPREG_MDMGPIO08_OFFSET          0
#define CHIPREG_MDMGPIO07_OFFSET          0
#define CHIPREG_MDMGPIO06_OFFSET          0
#define CHIPREG_MDMGPIO05_OFFSET          0
#define CHIPREG_MDMGPIO04_OFFSET          0
#define CHIPREG_MDMGPIO03_OFFSET          0
#define CHIPREG_MDMGPIO02_OFFSET          0
#define CHIPREG_MDMGPIO01_OFFSET          0
#define CHIPREG_MDMGPIO00_OFFSET          0
#define CHIPREG_GPS_HOSTREQ_OFFSET        0
#define CHIPREG_GPS_CALREQ_OFFSET         0
#define CHIPREG_ADCSYN_OFFSET             0

#define KONATZCFG_KONA_HUB_HSM_TZPROT_OFFSET KONATZCFG_KONA_HUB_APB7_TZPROT_OFFSET /* brcm_rdb_konatzcfg.h */

#define BINTC_BASE_ADDR                      0x3A050000  /* brcm_rdb_bintc.h */
#define BMODEM_SYSCFG_BASE_ADDR              0x3A004000  /* brcm_rdb_bmodem_syscfg.h */
#define AHB_DSP_TL3R_BASE_ADDR               0x3B400000  /* brcm_rdb_dsp_tl3r.h */
#define WCDMAL2INT_ASYNC_BASE_ADDR           0x3A10F000  /* brcm_rdb_layer_2_async.h */


/**** APB15 ****/
#define BMDM_CCU_BASE_ADDR                   0x3A055000 /* brcm_rdb_bmdm_clk_mgr_reg.h */
#define BMDM_RST_BASE_ADDR                   0x3A055F00 /* brcm_rdb_bmdm_rst_mgr_reg.h */
#define BMDM_PWRMGR_BASE_ADDR                0x3A057000 /* brcm_rdb_bmdm_pwrmgr.h */

/* Some of the following CCU items are no longer required in Capri since the block no longer exists */
/* These blocks no longer exist in Capri:  HSM, MSPRO, CRC, IrDA, D1W */

#define IKPS_CLK_MGR_REG_MSPRO_CLKGATE_MSPRO_AHB_CLK_EN_SHIFT                    0
#define IKPS_CLK_MGR_REG_MSPRO_CLKGATE_MSPRO_AHB_HW_SW_GATING_SEL_SHIFT          0
#define IKPS_CLK_MGR_REG_MSPRO_CLKGATE_MSPRO_CLK_EN_SHIFT                        0
#define IKPS_CLK_MGR_REG_MSPRO_CLKGATE_MSPRO_HW_SW_GATING_SEL_SHIFT              0
#define IKPS_CLK_MGR_REG_MSPRO_CLKGATE_MSPRO_AHB_STPRSTS_SHIFT                   0
#define IKPS_CLK_MGR_REG_MSPRO_CLKGATE_MSPRO_STPRSTS_SHIFT                       0
#define IKPS_CLK_MGR_REG_MSPRO_CLKGATE_MSPRO_VOLTAGE_LEVEL_SHIFT                 0
#define IKPS_CLK_MGR_REG_MAGIC_CLKGATE_MAGIC_AHB_CLK_EN_SHIFT                    0
#define IKPS_CLK_MGR_REG_MAGIC_CLKGATE_MAGIC_AHB_HW_SW_GATING_SEL_SHIFT          0
#define IKPS_CLK_MGR_REG_MAGIC_CLKGATE_MAGIC_AHB_STPRSTS_SHIFT                   0
#define IKPS_CLK_MGR_REG_MAGIC_CLKGATE_MAGIC_VOLTAGE_LEVEL_SHIFT                 0
#define IKPS_CLK_MGR_REG_CRC_CLKGATE_CRC_AHB_CLK_EN_SHIFT                        0
#define IKPS_CLK_MGR_REG_CRC_CLKGATE_CRC_AHB_HW_SW_GATING_SEL_SHIFT              0
#define IKPS_CLK_MGR_REG_CRC_CLKGATE_CRC_AHB_STPRSTS_SHIFT                       0
#define IKPS_CLK_MGR_REG_CRC_CLKGATE_CRC_VOLTAGE_LEVEL_SHIFT                     0
#define IKPS_CLK_MGR_REG_IRDA_CLKGATE_IRDA_APB_CLK_EN_SHIFT                      0
#define IKPS_CLK_MGR_REG_IRDA_CLKGATE_IRDA_APB_HW_SW_GATING_SEL_SHIFT            0
#define IKPS_CLK_MGR_REG_IRDA_CLKGATE_IRDA_CLK_EN_SHIFT                          0
#define IKPS_CLK_MGR_REG_IRDA_CLKGATE_IRDA_HW_SW_GATING_SEL_SHIFT                0
#define IKPS_CLK_MGR_REG_IRDA_CLKGATE_IRDA_APB_STPRSTS_SHIFT                     0
#define IKPS_CLK_MGR_REG_IRDA_CLKGATE_IRDA_STPRSTS_SHIFT                         0
#define IKPS_CLK_MGR_REG_IRDA_CLKGATE_IRDA_VOLTAGE_LEVEL_SHIFT                   0
#define IKPS_CLK_MGR_REG_I2S_CLKGATE_I2S_APB_CLK_EN_SHIFT                        0
#define IKPS_CLK_MGR_REG_I2S_CLKGATE_I2S_APB_HW_SW_GATING_SEL_SHIFT              0
#define IKPS_CLK_MGR_REG_I2S_CLKGATE_I2S_CLK_EN_SHIFT                            0
#define IKPS_CLK_MGR_REG_I2S_CLKGATE_I2S_HW_SW_GATING_SEL_SHIFT                  0
#define IKPS_CLK_MGR_REG_I2S_CLKGATE_I2S_SIDETONE_CLK_EN_SHIFT                   0
#define IKPS_CLK_MGR_REG_I2S_CLKGATE_I2S_SIDETONE_HW_SW_GATING_SEL_SHIFT         0
#define IKPS_CLK_MGR_REG_I2S_CLKGATE_I2S_APB_STPRSTS_SHIFT                       0
#define IKPS_CLK_MGR_REG_I2S_CLKGATE_I2S_STPRSTS_SHIFT                           0
#define IKPS_CLK_MGR_REG_I2S_CLKGATE_I2S_SIDETONE_STPRSTS_SHIFT                  0
#define IKPS_CLK_MGR_REG_I2S_CLKGATE_I2S_VOLTAGE_LEVEL_SHIFT                     0
#define IKPS_CLK_MGR_REG_HDMIKEY_CLKGATE_HDMIKEY_APB_CLK_EN_SHIFT                0
#define IKPS_CLK_MGR_REG_HDMIKEY_CLKGATE_HDMIKEY_APB_HW_SW_GATING_SEL_SHIFT      0
#define IKPS_CLK_MGR_REG_HDMIKEY_CLKGATE_HDMIKEY_APB_HYST_VAL_SHIFT              0
#define IKPS_CLK_MGR_REG_HDMIKEY_CLKGATE_HDMIKEY_APB_HYST_EN_SHIFT               0
#define IKPS_CLK_MGR_REG_HDMIKEY_CLKGATE_HDMIKEY_APB_STPRSTS_SHIFT               0
#define IKPS_CLK_MGR_REG_HDMIKEY_CLKGATE_HDMIKEY_VOLTAGE_LEVEL_SHIFT             0
#define IKPS_CLK_MGR_REG_D1W_CLKGATE_D1W_APB_CLK_EN_SHIFT                        0
#define IKPS_CLK_MGR_REG_D1W_CLKGATE_D1W_APB_HW_SW_GATING_SEL_SHIFT              0
#define IKPS_CLK_MGR_REG_D1W_CLKGATE_D1W_CLK_EN_SHIFT                            0
#define IKPS_CLK_MGR_REG_D1W_CLKGATE_D1W_HW_SW_GATING_SEL_SHIFT                  0
#define IKPS_CLK_MGR_REG_D1W_CLKGATE_D1W_APB_HYST_EN_SHIFT                       0
#define IKPS_CLK_MGR_REG_D1W_CLKGATE_D1W_APB_HYST_VAL_SHIFT                      0
#define IKPS_CLK_MGR_REG_D1W_CLKGATE_D1W_APB_STPRSTS_SHIFT                       0
#define IKPS_CLK_MGR_REG_D1W_CLKGATE_D1W_STPRSTS_SHIFT                           0
#define IKPS_CLK_MGR_REG_D1W_CLKGATE_D1W_VOLTAGE_LEVEL_SHIFT                     0
#define IKPS_CLK_MGR_REG_ADC_CLKGATE_AUXADC_APB_CLK_EN_SHIFT                     0
#define IKPS_CLK_MGR_REG_ADC_CLKGATE_AUXADC_APB_HW_SW_GATING_SEL_SHIFT           0
#define IKPS_CLK_MGR_REG_ADC_CLKGATE_AUXADC_CLK_EN_SHIFT                         0
#define IKPS_CLK_MGR_REG_ADC_CLKGATE_AUXADC_HW_SW_GATING_SEL_SHIFT               0
#define IKPS_CLK_MGR_REG_ADC_CLKGATE_AUXADC_APB_HYST_EN_SHIFT                    0
#define IKPS_CLK_MGR_REG_ADC_CLKGATE_AUXADC_APB_HYST_VAL_SHIFT                   0
#define IKPS_CLK_MGR_REG_ADC_CLKGATE_AUXADC_APB_STPRSTS_SHIFT                    0
#define IKPS_CLK_MGR_REG_ADC_CLKGATE_AUXADC_STPRSTS_SHIFT                        0
#define IKPS_CLK_MGR_REG_ADC_CLKGATE_AUXADC_VOLTAGE_LEVEL_SHIFT                  0
#define IKPS_CLK_MGR_REG_HSM_CLKGATE_HSM_AHB_CLK_EN_SHIFT                        0
#define IKPS_CLK_MGR_REG_HSM_CLKGATE_HSM_AHB_HW_SW_GATING_SEL_SHIFT              0
#define IKPS_CLK_MGR_REG_HSM_CLKGATE_HSM_APB_CLK_EN_SHIFT                        0
#define IKPS_CLK_MGR_REG_HSM_CLKGATE_HSM_APB_HW_SW_GATING_SEL_SHIFT              0
#define IKPS_CLK_MGR_REG_HSM_CLKGATE_HSM_AHB_HYST_VAL_SHIFT                      0
#define IKPS_CLK_MGR_REG_HSM_CLKGATE_HSM_AHB_HYST_EN_SHIFT                       0
#define IKPS_CLK_MGR_REG_HSM_CLKGATE_HSM_APB_HYST_VAL_SHIFT                      0
#define IKPS_CLK_MGR_REG_HSM_CLKGATE_HSM_APB_HYST_EN_SHIFT                       0
#define IKPS_CLK_MGR_REG_HSM_CLKGATE_HSM_AHB_STPRSTS_SHIFT                       0
#define IKPS_CLK_MGR_REG_HSM_CLKGATE_HSM_APB_STPRSTS_SHIFT                       0
#define    IKPS_CLK_MGR_REG_AXI_DIV_HSM_APB_DIV_SHIFT                     14
#define IKPS_CLK_MGR_REG_AXI_DIV_HSM_APB_DIV_OVERRIDE_SHIFT                      0
#define IKPS_CLK_MGR_REG_MSPRO_DIV_MSPRO_PLL_SELECT_SHIFT                        0
#define    IKPS_CLK_MGR_REG_MSPRO_DIV_MSPRO_DIV_SHIFT                     4
#define IKPS_CLK_MGR_REG_SSP1_AUDIO_DIV_SSP1_AUDIO_PRE_PLL_SELECT_SHIFT          0
#define    IKPS_CLK_MGR_REG_SSP1_AUDIO_DIV_SSP1_AUDIO_PRE_DIV_SHIFT       4
#define    IKPS_CLK_MGR_REG_SSP1_AUDIO_DIV_SSP1_AUDIO_DIV_SHIFT           16
#define IKPS_CLK_MGR_REG_IRDA_DIV_IRDA_PLL_SELECT_SHIFT                          0
#define IKPS_CLK_MGR_REG_IRDA_DIV_IRDA_DIV_SHIFT                                 0
#define IKPS_CLK_MGR_REG_REF_I2S_DIV_REF_I2S_FRAC_PLL_SELECT_SHIFT               0
#define IKPS_CLK_MGR_REG_REF_I2S_DIV_REF_I2S_FRAC_DIV_SHIFT                      0
#define IKPS_CLK_MGR_REG_I2S_DIV_I2S_DIV_SHIFT                                   0
#define IKPS_CLK_MGR_REG_D1W_DIV_D1W_PLL_SELECT_SHIFT                            0
#define IKPS_CLK_MGR_REG_AUXADC_DIV_AUXADC_PLL_SELECT_SHIFT                      0
#define IKPS_CLK_MGR_REG_I2S_SIDETONE_DIV_I2S_SIDETONE_PLL_SELECT_SHIFT          0
#define IKPS_CLK_MGR_REG_I2S_SIDETONE_DIV_I2S_SIDETONE_DIV_SHIFT                 0
#define IKPS_CLK_MGR_REG_DIV_TRIG_MSPRO_TRIGGER_SHIFT                            0
#define IKPS_CLK_MGR_REG_DIV_TRIG_SSP1_TRIGGER_SHIFT                             0
#define IKPS_CLK_MGR_REG_DIV_TRIG_SSP1_AUDIO_PRE_TRIGGER_SHIFT                   0
#define IKPS_CLK_MGR_REG_DIV_TRIG_SSP0_AUDIO_TRIGGER_SHIFT                       0
#define IKPS_CLK_MGR_REG_DIV_TRIG_SSP1_AUDIO_TRIGGER_SHIFT                       0
#define IKPS_CLK_MGR_REG_DIV_TRIG_SSP2_AUDIO_TRIGGER_SHIFT                       0
#define IKPS_CLK_MGR_REG_DIV_TRIG_IRDA_TRIGGER_SHIFT                             0
#define IKPS_CLK_MGR_REG_DIV_TRIG_REF_I2S_FRAC_TRIGGER_SHIFT                     0
#define IKPS_CLK_MGR_REG_DIV_TRIG_D1W_TRIGGER_SHIFT                              0
#define IKPS_CLK_MGR_REG_DIV_TRIG_AUXADC_TRIGGER_SHIFT                           0
#define IKPS_CLK_MGR_REG_DIV_TRIG_I2S_SIDETONE_TRIGGER_SHIFT                     0

#define IKPS_CLK_MGR_REG_MSPRO_CLKGATE_OFFSET                                    0
#define IKPS_CLK_MGR_REG_MAGIC_CLKGATE_OFFSET                                    0
#define IKPS_CLK_MGR_REG_CRC_CLKGATE_OFFSET                                      0
#define IKPS_CLK_MGR_REG_SSP1_CLKGATE_OFFSET                                     0
#define IKPS_CLK_MGR_REG_IRDA_CLKGATE_OFFSET                                     0
#define IKPS_CLK_MGR_REG_I2S_CLKGATE_OFFSET                                      0
#define IKPS_CLK_MGR_REG_HDMIKEY_CLKGATE_OFFSET                                  0
#define IKPS_CLK_MGR_REG_D1W_CLKGATE_OFFSET                                      0
#define IKPS_CLK_MGR_REG_ADC_CLKGATE_OFFSET                                      0
#define IKPS_CLK_MGR_REG_HSM_CLKGATE_OFFSET                                      0

#define IROOT_CLK_MGR_REG_PLL0C_PLL0_MDIV_SW_OVRRIDE_SHIFT                       0
#define IROOT_CLK_MGR_REG_PLL0C_PLL0_BYPCLK_EN_SHIFT                             0
#define IROOT_CLK_MGR_REG_PLL0C_PLL0_ENB_CLKOUT_SW_OVRRIDE_SHIFT                 0
#define IROOT_CLK_MGR_REG_PLL0C_PLL0_HOLD_SHIFT                                  0
#define IROOT_CLK_MGR_REG_PLL0C_PLL0_LOAD_EN_SHIFT                               0
#define IROOT_CLK_MGR_REG_PLL0C_PLL0_MDEL_SHIFT                                  0
#define IROOT_CLK_MGR_REG_PLL1C_PLL1_MDIV_SW_OVRRIDE_SHIFT                       0
#define IROOT_CLK_MGR_REG_PLL1C_PLL1_BYPCLK_EN_SHIFT                             0
#define IROOT_CLK_MGR_REG_PLL1C_PLL1_ENB_CLKOUT_SW_OVRRIDE_SHIFT                 0
#define IROOT_CLK_MGR_REG_PLL1C_PLL1_HOLD_SHIFT                                  0
#define IROOT_CLK_MGR_REG_PLL1C_PLL1_LOAD_EN_SHIFT                               0
#define IROOT_CLK_MGR_REG_PLL1C_PLL1_MDEL_SHIFT                                  0
#define IROOT_CLK_MGR_REG_PLL1_208M_PLL1_208M_MDIV_SHIFT                         0

#define    IROOT_CLK_MGR_REG_PLL0C_PLL0_MDIV_SW_OVRRIDE_MASK              0x000000FF
#define    IROOT_CLK_MGR_REG_PLL0C_PLL0_MDEL_MASK                         0x00007000
#define    IROOT_CLK_MGR_REG_PLL1C_PLL1_MDEL_MASK                         0x00007000
#define    IROOT_CLK_MGR_REG_PLL1_208M_PLL1_208M_MDIV_MASK                0x000000FF
#define    IROOT_CLK_MGR_REG_PLL1C_PLL1_MDIV_SW_OVRRIDE_MASK              0x000000FF

#define    IKPS_CLK_MGR_REG_AXI_DIV_HSM_APB_DIV_MASK                      0x0000C000
#define    IKPS_CLK_MGR_REG_MSPRO_DIV_MSPRO_PLL_SELECT_MASK               0x00000003
#define    IKPS_CLK_MGR_REG_MSPRO_DIV_MSPRO_DIV_MASK                      0x000001F0
#define IKPS_CLK_MGR_REG_MSPRO_DIV_OFFSET                                 0x00000A0C
#define IKPS_CLK_MGR_REG_SSP1_DIV_OFFSET                                  0x00000A24
#define IKPS_CLK_MGR_REG_SSP1_AUDIO_DIV_OFFSET                            0x00000A38
#define    IKPS_CLK_MGR_REG_IRDA_DIV_IRDA_PLL_SELECT_MASK                 0x00000003
#define    IKPS_CLK_MGR_REG_IRDA_DIV_IRDA_DIV_MASK                        0x0000FFF0
#define IKPS_CLK_MGR_REG_IRDA_DIV_OFFSET                                  0x00000A48
#define    IKPS_CLK_MGR_REG_REF_I2S_DIV_REF_I2S_FRAC_PLL_SELECT_MASK      0x00000003
#define    IKPS_CLK_MGR_REG_REF_I2S_DIV_REF_I2S_FRAC_DIV_MASK             0x0000FFF0
#define IKPS_CLK_MGR_REG_REF_I2S_DIV_OFFSET                               0x00000A4C
#define    IKPS_CLK_MGR_REG_D1W_DIV_D1W_PLL_SELECT_MASK                   0x00000003
#define IKPS_CLK_MGR_REG_D1W_DIV_OFFSET                                   0x00000A60
#define    IKPS_CLK_MGR_REG_AUXADC_DIV_AUXADC_PLL_SELECT_MASK             0x00000003
#define IKPS_CLK_MGR_REG_AUXADC_DIV_OFFSET                                0x00000A6C
#define    IKPS_CLK_MGR_REG_I2S_SIDETONE_DIV_I2S_SIDETONE_PLL_SELECT_MASK 0x00000003
#define IKPS_CLK_MGR_REG_I2S_SIDETONE_DIV_OFFSET                          0x00000A74
#define    IKPS_CLK_MGR_REG_I2S_DIV_I2S_DIV_MASK                          0x00000FF0
#define IKPS_CLK_MGR_REG_I2S_DIV_OFFSET                                   0x00000A50
#define    IKPS_CLK_MGR_REG_I2S_SIDETONE_DIV_I2S_SIDETONE_DIV_MASK        0x000001F0

#define CHIPREG_ISLAND_ID_OFFSET                                          0x00000008
#define CHIPREG_ISLAND_STRAP_OFFSET                                       0x0000000C
#define CHIPREG_ISLAND_STRAP_TYPE                                         UInt32
#define CHIPREG_ISLAND_STRAP_RESERVED_MASK                                0xFFFF0000
#define    CHIPREG_ISLAND_STRAP_STRAP_IN_15TO11_SHIFT                     11
#define    CHIPREG_ISLAND_STRAP_STRAP_IN_15TO11_MASK                      0x0000F800
#define    CHIPREG_ISLAND_STRAP_STRAP_IN_10_SHIFT                         10
#define    CHIPREG_ISLAND_STRAP_STRAP_IN_10_MASK                          0x00000400
#define    CHIPREG_ISLAND_STRAP_STRAP_IN_9_SHIFT                          9
#define    CHIPREG_ISLAND_STRAP_STRAP_IN_9_MASK                           0x00000200
#define    CHIPREG_ISLAND_STRAP_STRAP_IN_8_SHIFT                          8
#define    CHIPREG_ISLAND_STRAP_STRAP_IN_8_MASK                           0x00000100
#define    CHIPREG_ISLAND_STRAP_STRAP_IN_7TO6_SHIFT                       6
#define    CHIPREG_ISLAND_STRAP_STRAP_IN_7TO6_MASK                        0x000000C0
#define    CHIPREG_ISLAND_STRAP_STRAP_IN_5TO3_SHIFT                       3
#define    CHIPREG_ISLAND_STRAP_STRAP_IN_5TO3_MASK                        0x00000038
#define    CHIPREG_ISLAND_STRAP_STRAP_IN_2_SHIFT                          2
#define    CHIPREG_ISLAND_STRAP_STRAP_IN_2_MASK                           0x00000004
#define    CHIPREG_ISLAND_STRAP_STRAP_IN_1_SHIFT                          1
#define    CHIPREG_ISLAND_STRAP_STRAP_IN_1_MASK                           0x00000002
#define    CHIPREG_ISLAND_STRAP_STRAP_IN_0_SHIFT                          0
#define    CHIPREG_ISLAND_STRAP_STRAP_IN_0_MASK                           0x00000001

#define    CHIPREG_CHIP_SW_STRAP_SW_STRAP_EN_SHIFT                        31
#define    CHIPREG_CHIP_SW_STRAP_SW_STRAP_EN_MASK                         0x80000000



/* GPIOMUX definitions.  It may have been easier to just #if in
*  gpiomux_hana_main.c since this is throwaway due to PINMUX changes
*/
#define CHIPREG_STAT_2_OFFSET                PADCTRLREG_STAT_2_OFFSET
#define CHIPREG_STAT_1_OFFSET                PADCTRLREG_STAT_1_OFFSET
#define CHIPREG_PMU_INT_OFFSET               PADCTRLREG_PMU_INT_OFFSET
#define CHIPREG_BAT_RM_OFFSET                PADCTRLREG_BAT_RM_OFFSET
#define CHIPREG_DIGMIC2_DQ_OFFSET            PADCTRLREG_DIGMIC2_DQ_OFFSET
#define CHIPREG_DIGMIC2_CLK_OFFSET           PADCTRLREG_DIGMIC2_CLK_OFFSET
#define CHIPREG_DIGMIC1_DQ_OFFSET            PADCTRLREG_DIGMIC1_DQ_OFFSET
#define CHIPREG_DIGMIC1_CLK_OFFSET           PADCTRLREG_DIGMIC1_CLK_OFFSET
#define CHIPREG_CLKOUT_1_OFFSET              PADCTRLREG_CLKOUT_1_OFFSET
#define CHIPREG_CLKOUT_0_OFFSET              PADCTRLREG_CLKOUT_0_OFFSET
#define CHIPREG_CLKREQ_IN_1_OFFSET           PADCTRLREG_CLKREQ_IN_1_OFFSET
#define CHIPREG_CLKREQ_IN_0_OFFSET           PADCTRLREG_CLKREQ_IN_0_OFFSET
#define CHIPREG_LCD_PCLK_OFFSET              PADCTRLREG_LCD_PCLK_OFFSET
#define CHIPREG_LCD_OE_OFFSET                PADCTRLREG_LCD_OE_OFFSET
#define CHIPREG_LCD_VSYNC_OFFSET             PADCTRLREG_LCD_VSYNC_OFFSET
#define CHIPREG_LCD_HSYNC_OFFSET             PADCTRLREG_LCD_HSYNC_OFFSET
#define CHIPREG_LCD_B_0_OFFSET               PADCTRLREG_LCD_B_0_OFFSET
#define CHIPREG_LCD_B_1_OFFSET               PADCTRLREG_LCD_B_1_OFFSET
#define CHIPREG_LCD_B_2_OFFSET               PADCTRLREG_LCD_B_2_OFFSET
#define CHIPREG_LCD_B_3_OFFSET               PADCTRLREG_LCD_B_3_OFFSET
#define CHIPREG_LCD_B_4_OFFSET               PADCTRLREG_LCD_B_4_OFFSET
#define CHIPREG_LCD_B_5_OFFSET               PADCTRLREG_LCD_B_5_OFFSET
#define CHIPREG_LCD_B_6_OFFSET               PADCTRLREG_LCD_B_6_OFFSET
#define CHIPREG_LCD_B_7_OFFSET               PADCTRLREG_LCD_B_7_OFFSET
#define CHIPREG_LCD_G_0_OFFSET               PADCTRLREG_LCD_G_0_OFFSET
#define CHIPREG_LCD_G_1_OFFSET               PADCTRLREG_LCD_G_1_OFFSET
#define CHIPREG_LCD_G_2_OFFSET               PADCTRLREG_LCD_G_2_OFFSET
#define CHIPREG_LCD_G_3_OFFSET               PADCTRLREG_LCD_G_3_OFFSET
#define CHIPREG_LCD_G_4_OFFSET               PADCTRLREG_LCD_G_4_OFFSET
#define CHIPREG_LCD_G_5_OFFSET               PADCTRLREG_LCD_G_5_OFFSET
#define CHIPREG_LCD_G_6_OFFSET               PADCTRLREG_LCD_G_6_OFFSET
#define CHIPREG_LCD_G_7_OFFSET               PADCTRLREG_LCD_G_7_OFFSET
#define CHIPREG_LCD_R_0_OFFSET               PADCTRLREG_LCD_R_0_OFFSET
#define CHIPREG_LCD_R_1_OFFSET               PADCTRLREG_LCD_R_1_OFFSET
#define CHIPREG_LCD_R_2_OFFSET               PADCTRLREG_LCD_R_2_OFFSET
#define CHIPREG_LCD_R_3_OFFSET               PADCTRLREG_LCD_R_3_OFFSET
#define CHIPREG_LCD_R_4_OFFSET               PADCTRLREG_LCD_R_4_OFFSET
#define CHIPREG_LCD_R_5_OFFSET               PADCTRLREG_LCD_R_5_OFFSET
#define CHIPREG_LCD_R_6_OFFSET               PADCTRLREG_LCD_R_6_OFFSET
#define CHIPREG_LCD_R_7_OFFSET               PADCTRLREG_LCD_R_7_OFFSET
#define CHIPREG_UARTB2_UTXD_OFFSET           PADCTRLREG_UARTB2_UTXD_OFFSET
#define CHIPREG_UARTB2_URXD_OFFSET           PADCTRLREG_UARTB2_URXD_OFFSET
#define CHIPREG_UARTB1_UCTS_OFFSET           PADCTRLREG_UARTB1_UCTS_OFFSET
#define CHIPREG_UARTB1_URTS_OFFSET           PADCTRLREG_UARTB1_URTS_OFFSET
#define CHIPREG_UARTB1_UTXD_OFFSET           PADCTRLREG_UARTB1_UTXD_OFFSET
#define CHIPREG_UARTB1_URXD_OFFSET           PADCTRLREG_UARTB1_URXD_OFFSET
#define CHIPREG_HDMI_SDA_OFFSET              PADCTRLREG_HDMI_SDA_OFFSET
#define CHIPREG_HDMI_SCL_OFFSET              PADCTRLREG_HDMI_SCL_OFFSET
#define CHIPREG_VC_CAM1_SDA_OFFSET           PADCTRLREG_VC_CAM1_SDA_OFFSET
#define CHIPREG_VC_CAM1_SCL_OFFSET           PADCTRLREG_VC_CAM1_SCL_OFFSET
#define CHIPREG_BSC2_SDA_OFFSET              PADCTRLREG_BSC2_SDA_OFFSET
#define CHIPREG_BSC2_SCL_OFFSET              PADCTRLREG_BSC2_SCL_OFFSET
#define CHIPREG_PMU_SDA_OFFSET               PADCTRLREG_PMU_SDA_OFFSET
#define CHIPREG_PMU_SCL_OFFSET               PADCTRLREG_PMU_SCL_OFFSET
#define CHIPREG_SSP3_TXD_OFFSET              PADCTRLREG_SSP3_TXD_OFFSET
#define CHIPREG_SSP3_RXD_OFFSET              PADCTRLREG_SSP3_RXD_OFFSET
#define CHIPREG_SSP3_CLK_OFFSET              PADCTRLREG_SSP3_CLK_OFFSET
#define CHIPREG_SSP3_FS_OFFSET               PADCTRLREG_SSP3_FS_OFFSET
#define CHIPREG_SSP2_FS_2_OFFSET             PADCTRLREG_SSP2_FS_2_OFFSET
#define CHIPREG_SSP2_TXD_1_OFFSET            PADCTRLREG_SSP2_TXD_1_OFFSET
#define CHIPREG_SSP2_RXD_1_OFFSET            PADCTRLREG_SSP2_RXD_1_OFFSET
#define CHIPREG_SSP2_FS_1_OFFSET             PADCTRLREG_SSP2_FS_1_OFFSET
#define CHIPREG_SSP2_TXD_0_OFFSET            PADCTRLREG_SSP2_TXD_0_OFFSET
#define CHIPREG_SSP2_RXD_0_OFFSET            PADCTRLREG_SSP2_RXD_0_OFFSET
#define CHIPREG_SSP2_CLK_OFFSET              PADCTRLREG_SSP2_CLK_OFFSET
#define CHIPREG_SSP2_FS_0_OFFSET             PADCTRLREG_SSP2_FS_0_OFFSET
#define CHIPREG_SSP0_TXD_OFFSET              PADCTRLREG_SSP0_TXD_OFFSET
#define CHIPREG_SSP0_RXD_OFFSET              PADCTRLREG_SSP0_RXD_OFFSET
#define CHIPREG_SSP0_CLK_OFFSET              PADCTRLREG_SSP0_CLK_OFFSET
#define CHIPREG_SSP0_FS_OFFSET               PADCTRLREG_SSP0_FS_OFFSET
#define CHIPREG_UARTB4_URXD_OFFSET           PADCTRLREG_UARTB4_URXD_OFFSET
#define CHIPREG_UARTB4_UTXD_OFFSET           PADCTRLREG_UARTB4_UTXD_OFFSET
#define CHIPREG_NAND_AD_0_OFFSET             PADCTRLREG_NAND_AD_0_OFFSET
#define CHIPREG_NAND_AD_1_OFFSET             PADCTRLREG_NAND_AD_1_OFFSET
#define CHIPREG_NAND_AD_2_OFFSET             PADCTRLREG_NAND_AD_2_OFFSET
#define CHIPREG_NAND_AD_3_OFFSET             PADCTRLREG_NAND_AD_3_OFFSET
#define CHIPREG_NAND_AD_4_OFFSET             PADCTRLREG_NAND_AD_4_OFFSET
#define CHIPREG_NAND_AD_5_OFFSET             PADCTRLREG_NAND_AD_5_OFFSET
#define CHIPREG_NAND_AD_6_OFFSET             PADCTRLREG_NAND_AD_6_OFFSET
#define CHIPREG_NAND_AD_7_OFFSET             PADCTRLREG_NAND_AD_7_OFFSET
#define CHIPREG_NAND_WEN_OFFSET              PADCTRLREG_NAND_WEN_OFFSET
#define CHIPREG_NAND_OEN_OFFSET              PADCTRLREG_NAND_OEN_OFFSET
#define CHIPREG_NAND_ALE_OFFSET              PADCTRLREG_NAND_ALE_OFFSET
#define CHIPREG_NAND_CLE_OFFSET              PADCTRLREG_NAND_CLE_OFFSET
#define CHIPREG_NAND_RDY_1_OFFSET            PADCTRLREG_NAND_RDY_1_OFFSET
#define CHIPREG_NAND_RDY_0_OFFSET            PADCTRLREG_NAND_RDY_0_OFFSET
#define CHIPREG_NAND_CEN_1_OFFSET            PADCTRLREG_NAND_CEN_1_OFFSET
#define CHIPREG_NAND_CEN_0_OFFSET            PADCTRLREG_NAND_CEN_0_OFFSET
#define CHIPREG_NAND_WP_OFFSET               PADCTRLREG_NAND_WP_OFFSET
#define CHIPREG_SIM2_DET_OFFSET              PADCTRLREG_SIM2_DET_OFFSET
#define CHIPREG_SIM2_DATA_OFFSET             PADCTRLREG_SIM2_DATA_OFFSET
#define CHIPREG_SIM2_CLK_OFFSET              PADCTRLREG_SIM2_CLK_OFFSET
#define CHIPREG_SIM2_RESETN_OFFSET           PADCTRLREG_SIM2_RESETN_OFFSET
#define CHIPREG_SIM_DET_OFFSET               PADCTRLREG_SIM_DET_OFFSET
#define CHIPREG_SIM_DATA_OFFSET              PADCTRLREG_SIM_DATA_OFFSET
#define CHIPREG_SIM_CLK_OFFSET               PADCTRLREG_SIM_CLK_OFFSET
#define CHIPREG_SIM_RESETN_OFFSET            PADCTRLREG_SIM_RESETN_OFFSET
#define CHIPREG_SYSCLKEN_OFFSET              PADCTRLREG_SYSCLKEN_OFFSET
#define CHIPREG_CLK_CX8_OFFSET               PADCTRLREG_CLK_CX8_OFFSET
#define CHIPREG_RXDATA3G2_OFFSET             PADCTRLREG_RXDATA3G2_OFFSET
#define CHIPREG_RXDATA3G1_OFFSET             PADCTRLREG_RXDATA3G1_OFFSET
#define CHIPREG_RXDATA3G0_OFFSET             PADCTRLREG_RXDATA3G0_OFFSET
#define CHIPREG_RTXEN2G_TXDATA3G2_OFFSET     PADCTRLREG_RTXEN2G_TXDATA3G2_OFFSET
#define CHIPREG_RTXDATA2G_TXDATA3G1_OFFSET   PADCTRLREG_RTXDATA2G_TXDATA3G1_OFFSET
#define CHIPREG_TXDATA3G0_OFFSET             PADCTRLREG_TXDATA3G0_OFFSET
#define CHIPREG_RFST2G_MTSLOTEN3G_OFFSET     PADCTRLREG_RFST2G_MTSLOTEN3G_OFFSET
#define CHIPREG_SRI_D_OFFSET                 PADCTRLREG_SRI_D_OFFSET
#define CHIPREG_SRI_E_OFFSET                 PADCTRLREG_SRI_E_OFFSET
#define CHIPREG_SRI_C_OFFSET                 PADCTRLREG_SRI_C_OFFSET
#define CHIPREG_TRACEDT00_OFFSET             PADCTRLREG_TRACEDT00_OFFSET
#define CHIPREG_TRACEDT01_OFFSET             PADCTRLREG_TRACEDT01_OFFSET
#define CHIPREG_TRACEDT02_OFFSET             PADCTRLREG_TRACEDT02_OFFSET
#define CHIPREG_TRACEDT03_OFFSET             PADCTRLREG_TRACEDT03_OFFSET
#define CHIPREG_TRACEDT04_OFFSET             PADCTRLREG_TRACEDT04_OFFSET
#define CHIPREG_TRACEDT05_OFFSET             PADCTRLREG_TRACEDT05_OFFSET
#define CHIPREG_TRACEDT06_OFFSET             PADCTRLREG_TRACEDT06_OFFSET
#define CHIPREG_TRACEDT07_OFFSET             PADCTRLREG_TRACEDT07_OFFSET
#define CHIPREG_TRACEDT08_OFFSET             PADCTRLREG_TRACEDT08_OFFSET
#define CHIPREG_TRACEDT09_OFFSET             PADCTRLREG_TRACEDT09_OFFSET
#define CHIPREG_TRACEDT10_OFFSET             PADCTRLREG_TRACEDT10_OFFSET
#define CHIPREG_TRACEDT11_OFFSET             PADCTRLREG_TRACEDT11_OFFSET
#define CHIPREG_TRACEDT12_OFFSET             PADCTRLREG_TRACEDT12_OFFSET
#define CHIPREG_TRACEDT13_OFFSET             PADCTRLREG_TRACEDT13_OFFSET
#define CHIPREG_TRACEDT14_OFFSET             PADCTRLREG_TRACEDT14_OFFSET
#define CHIPREG_TRACEDT15_OFFSET             PADCTRLREG_TRACEDT15_OFFSET
#define CHIPREG_TRACECLK_OFFSET              PADCTRLREG_TRACECLK_OFFSET
#define CHIPREG_GPEN15_OFFSET                PADCTRLREG_GPEN15_OFFSET
#define CHIPREG_GPEN13_OFFSET                PADCTRLREG_GPEN13_OFFSET
#define CHIPREG_GPS_PABLANK_OFFSET           PADCTRLREG_GPS_PABLANK_OFFSET
#define CHIPREG_GPS_TMARK_OFFSET             PADCTRLREG_GPS_TMARK_OFFSET

