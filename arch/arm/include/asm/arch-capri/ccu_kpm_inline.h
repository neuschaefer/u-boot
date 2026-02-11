/*****************************************************************************
*
*    (c) 2008 Broadcom Corporation
*
* This program is the proprietary software of Broadcom Corporation and/or
* its licensors, and may only be used, duplicated, modified or distributed
* pursuant to the terms and conditions of a separate, written license
* agreement executed between you and Broadcom (an "Authorized License").
* Except as set forth in an Authorized License, Broadcom grants no license
* (express or implied), right to use, or waiver of any kind with respect to
* the Software, and Broadcom expressly reserves all rights in and to the
* Software and all intellectual property rights therein.
* IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS
* SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE
* ALL USE OF THE SOFTWARE.
*
* Except as expressly set forth in the Authorized License,
*
* 1. This program, including its structure, sequence and organization,
*    constitutes the valuable trade secrets of Broadcom, and you shall use all
*    reasonable efforts to protect the confidentiality thereof, and to use
*    this information only in connection with your use of Broadcom integrated
*    circuit products.
*
* 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
*    RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
*    IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS
*    FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS,
*    QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU
*    ASSUME THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
*
* 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
*    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
*    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
*    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
*    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
*    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
*    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
*    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
*
*****************************************************************************/


/**
*
* @file  ccu_kpm_inline.c
*
* @brief KONA Peripheral Master Clock Control Unit inline functions
*
* @note
*
*******************************************************************************/
#ifndef _CCU_KPM_INLINE_H_
#define _CCU_KPM_INLINE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <asm/arch/ccu_inline.h>
#include <asm/arch/ccu_util_inline.h>
#include <asm/arch/brcm_rdb_kpm_clk_mgr_reg.h>


/* type definitions */

typedef enum
{
   ccu_kpm_peri_volt_normal = KPM_CLK_MGR_REG_VLT_PERI_VLT_NORMAL_PERI_SHIFT,
   ccu_kpm_peri_volt_high   = KPM_CLK_MGR_REG_VLT_PERI_VLT_HIGH_PERI_SHIFT

} ccu_kpm_peri_volt_e;

typedef enum
{
   ccu_kpm_lvm_0 = 0,
   ccu_kpm_lvm_1,
   ccu_kpm_lvm_2,
   ccu_kpm_lvm_3,
   ccu_kpm_lvm_4,
   ccu_kpm_lvm_5,
   ccu_kpm_lvm_6,
   ccu_kpm_lvm_7

} ccu_kpm_lvm_e;

typedef enum
{
   ccu_kpm_vlt_0 = 0,
   ccu_kpm_vlt_1,
   ccu_kpm_vlt_2,
   ccu_kpm_vlt_3,
   ccu_kpm_vlt_4,
   ccu_kpm_vlt_5,
   ccu_kpm_vlt_6,
   ccu_kpm_vlt_7

} ccu_kpm_vlt_e;

typedef enum
{
   ccu_kpm_bus_quiesc_switch_axi_reqgnt_inh = KPM_CLK_MGR_REG_BUS_QUIESC_SYS_SWITCH_AXI_SWITCH_REQGNT_INH_SHIFT

} ccu_kpm_bus_quiesc_e;

typedef enum
{
   ccu_kpm_axi_sys_switch_clkgate_axi_hw_sw_gating_sel = KPM_CLK_MGR_REG_AXI_SYS_SWITCH_CLKGATE_SYS_SWITCH_AXI_HW_SW_GATING_SEL_SHIFT,
   ccu_kpm_axi_sys_switch_clkgate_axi_hyst_val         = KPM_CLK_MGR_REG_AXI_SYS_SWITCH_CLKGATE_SYS_SWITCH_AXI_HYST_VAL_SHIFT,
   ccu_kpm_axi_sys_switch_clkgate_axi_hyst_en          = KPM_CLK_MGR_REG_AXI_SYS_SWITCH_CLKGATE_SYS_SWITCH_AXI_HYST_EN_SHIFT,
   ccu_kpm_axi_sys_switch_clkgate_axi_stprsts          = KPM_CLK_MGR_REG_AXI_SYS_SWITCH_CLKGATE_SYS_SWITCH_AXI_STPRSTS_SHIFT,
   ccu_kpm_axi_sys_switch_clkgate_voltage_level        = KPM_CLK_MGR_REG_AXI_SYS_SWITCH_CLKGATE_SYS_SWITCH_VOLTAGE_LEVEL_SHIFT

} ccu_kpm_axi_sys_switch_clkgate_e;

typedef enum
{
   ccu_kpm_axi_mst_switch_clkgate_axi_hw_sw_gating_sel = KPM_CLK_MGR_REG_AXI_MST_SWITCH_CLKGATE_MASTER_SWITCH_AXI_HW_SW_GATING_SEL_SHIFT,
   ccu_kpm_axi_mst_switch_clkgate_ahb_clk_en           = KPM_CLK_MGR_REG_AXI_MST_SWITCH_CLKGATE_MASTER_SWITCH_AHB_CLK_EN_SHIFT,
   ccu_kpm_axi_mst_switch_clkgate_ahb_hw_sw_gating_sel = KPM_CLK_MGR_REG_AXI_MST_SWITCH_CLKGATE_MASTER_SWITCH_AHB_HW_SW_GATING_SEL_SHIFT,
   ccu_kpm_axi_mst_switch_clkgate_axi_hyst_val         = KPM_CLK_MGR_REG_AXI_MST_SWITCH_CLKGATE_MASTER_SWITCH_AXI_HYST_VAL_SHIFT,
   ccu_kpm_axi_mst_switch_clkgate_axi_hyst_en          = KPM_CLK_MGR_REG_AXI_MST_SWITCH_CLKGATE_MASTER_SWITCH_AXI_HYST_EN_SHIFT,
   ccu_kpm_axi_mst_switch_clkgate_ahb_hyst_val         = KPM_CLK_MGR_REG_AXI_MST_SWITCH_CLKGATE_MASTER_SWITCH_AHB_HYST_VAL_SHIFT,
   ccu_kpm_axi_mst_switch_clkgate_ahb_hyst_en          = KPM_CLK_MGR_REG_AXI_MST_SWITCH_CLKGATE_MASTER_SWITCH_AHB_HYST_EN_SHIFT,
   ccu_kpm_axi_mst_switch_clkgate_axi_stprsts          = KPM_CLK_MGR_REG_AXI_MST_SWITCH_CLKGATE_MASTER_SWITCH_AXI_STPRSTS_SHIFT,
   ccu_kpm_axi_mst_switch_clkgate_ahb_stprsts          = KPM_CLK_MGR_REG_AXI_MST_SWITCH_CLKGATE_MASTER_SWITCH_AHB_STPRSTS_SHIFT,
   ccu_kpm_axi_mst_switch_clkgate_voltage_level        = KPM_CLK_MGR_REG_AXI_MST_SWITCH_CLKGATE_MASTER_SWITCH_VOLTAGE_LEVEL_SHIFT

} ccu_kpm_axi_mst_switch_clkgate_e;

typedef enum
{
   ccu_kpm_axi_core_clkgate_clk_en            = KPM_CLK_MGR_REG_AXI_CORE_CLKGATE_ARMCORE_AXI_CLK_EN_SHIFT,
   ccu_kpm_axi_core_clkgate_hw_sw_gating_sel  = KPM_CLK_MGR_REG_AXI_CORE_CLKGATE_ARMCORE_AXI_HW_SW_GATING_SEL_SHIFT,
   ccu_kpm_axi_core_clkgate_hyst_val          = KPM_CLK_MGR_REG_AXI_CORE_CLKGATE_ARMCORE_AXI_HYST_VAL_SHIFT,
   ccu_kpm_axi_core_clkgate_hyst_en           = KPM_CLK_MGR_REG_AXI_CORE_CLKGATE_ARMCORE_AXI_HYST_EN_SHIFT,
   ccu_kpm_axi_core_clkgate_stprsts           = KPM_CLK_MGR_REG_AXI_CORE_CLKGATE_ARMCORE_AXI_STPRSTS_SHIFT,
   ccu_kpm_axi_core_clkgate_voltage_level     = KPM_CLK_MGR_REG_AXI_CORE_CLKGATE_ARMCORE_VOLTAGE_LEVEL_SHIFT

} ccu_kpm_axi_core_clkgate_e;

typedef enum
{
   ccu_kpm_apb4_clkgate_clk_en             = KPM_CLK_MGR_REG_APB4_CLKGATE_APB4_CLK_EN_SHIFT,
   ccu_kpm_apb4_clkgate_hw_sw_gating_sel   = KPM_CLK_MGR_REG_APB4_CLKGATE_APB4_HW_SW_GATING_SEL_SHIFT,
   ccu_kpm_apb4_clkgate_hyst_val           = KPM_CLK_MGR_REG_APB4_CLKGATE_APB4_HYST_VAL_SHIFT,
   ccu_kpm_apb4_clkgate_hyst_en            = KPM_CLK_MGR_REG_APB4_CLKGATE_APB4_HYST_EN_SHIFT,
   ccu_kpm_apb4_clkgate_stprsts            = KPM_CLK_MGR_REG_APB4_CLKGATE_APB4_STPRSTS_SHIFT

} ccu_kpm_apb4_clkgate_e;

typedef enum
{
   ccu_kpm_apb8_clkgate_clk_en             = KPM_CLK_MGR_REG_APB8_CLKGATE_APB8_CLK_EN_SHIFT,
   ccu_kpm_apb8_clkgate_hw_sw_gating_sel   = KPM_CLK_MGR_REG_APB8_CLKGATE_APB8_HW_SW_GATING_SEL_SHIFT,
   ccu_kpm_apb8_clkgate_hyst_val           = KPM_CLK_MGR_REG_APB8_CLKGATE_APB8_HYST_VAL_SHIFT,
   ccu_kpm_apb8_clkgate_hyst_en            = KPM_CLK_MGR_REG_APB8_CLKGATE_APB8_HYST_EN_SHIFT,
   ccu_kpm_apb8_clkgate_stprsts            = KPM_CLK_MGR_REG_APB8_CLKGATE_APB8_STPRSTS_SHIFT,
   ccu_kpm_apb8_clkgate_voltage_level      = KPM_CLK_MGR_REG_APB8_CLKGATE_APB8_VOLTAGE_LEVEL_SHIFT

} ccu_kpm_apb8_clkgate_e;

typedef enum
{
   ccu_kpm_dmac_clkgate_axi_clk_en            = KPM_CLK_MGR_REG_DMAC_CLKGATE_DMA_AXI_CLK_EN_SHIFT,
   ccu_kpm_dmac_clkgate_axi_hw_sw_gating_sel  = KPM_CLK_MGR_REG_DMAC_CLKGATE_DMA_AXI_HW_SW_GATING_SEL_SHIFT,
   ccu_kpm_dmac_clkgate_axi_hyst_val          = KPM_CLK_MGR_REG_DMAC_CLKGATE_DMA_AXI_HYST_VAL_SHIFT,
   ccu_kpm_dmac_clkgate_axi_hyst_en           = KPM_CLK_MGR_REG_DMAC_CLKGATE_DMA_AXI_HYST_EN_SHIFT,
   ccu_kpm_dmac_clkgate_axi_stprsts           = KPM_CLK_MGR_REG_DMAC_CLKGATE_DMA_AXI_STPRSTS_SHIFT,
   ccu_kpm_dmac_clkgate_voltage_level         = KPM_CLK_MGR_REG_DMAC_CLKGATE_DMA_VOLTAGE_LEVEL_SHIFT

} ccu_kpm_dmac_clkgate_e;

typedef enum
{
   ccu_kpm_nand_clkgate_ahb_clk_en            = KPM_CLK_MGR_REG_NAND_CLKGATE_NAND_AHB_CLK_EN_SHIFT,
   ccu_kpm_nand_clkgate_ahb_hw_sw_gating_sel  = KPM_CLK_MGR_REG_NAND_CLKGATE_NAND_AHB_HW_SW_GATING_SEL_SHIFT,
   ccu_kpm_nand_clkgate_clk_en                = KPM_CLK_MGR_REG_NAND_CLKGATE_NAND_CLK_EN_SHIFT,
   ccu_kpm_nand_clkgate_hw_sw_gating_sel      = KPM_CLK_MGR_REG_NAND_CLKGATE_NAND_HW_SW_GATING_SEL_SHIFT,
   ccu_kpm_nand_clkgate_ahb_stprsts           = KPM_CLK_MGR_REG_NAND_CLKGATE_NAND_AHB_STPRSTS_SHIFT,
   ccu_kpm_nand_clkgate_stprsts               = KPM_CLK_MGR_REG_NAND_CLKGATE_NAND_STPRSTS_SHIFT,
   ccu_kpm_nand_clkgate_voltage_level         = KPM_CLK_MGR_REG_NAND_CLKGATE_NAND_VOLTAGE_LEVEL_SHIFT

} ccu_kpm_nand_clkgate_e;

typedef enum
{
   ccu_kpm_usb_otg_clkgate_ahb_clk_en               = KPM_CLK_MGR_REG_USB_OTG_CLKGATE_USB_OTG_AHB_CLK_EN_SHIFT,
   ccu_kpm_usb_otg_clkgate_ahb_hw_sw_gating_select  = KPM_CLK_MGR_REG_USB_OTG_CLKGATE_USB_OTG_AHB_HW_SW_GATING_SEL_SHIFT,
   ccu_kpm_usb_otg_clkgate_ahb_stprsts              = KPM_CLK_MGR_REG_USB_OTG_CLKGATE_USB_OTG_AHB_STPRSTS_SHIFT,
   ccu_kpm_usb_otg_clkgate_voltage_level            = KPM_CLK_MGR_REG_USB_OTG_CLKGATE_USB_OTG_VOLTAGE_LEVEL_SHIFT

} ccu_kpm_usb_otg_clkgate_e;

typedef enum
{
   ccu_kpm_usb_ic_clkgate_ahb_clk_en             = KPM_CLK_MGR_REG_USB_IC_CLKGATE_USB_IC_AHB_CLK_EN_SHIFT,
   ccu_kpm_usb_ic_clkgate_ahb_hw_sw_gating_sel   = KPM_CLK_MGR_REG_USB_IC_CLKGATE_USB_IC_AHB_HW_SW_GATING_SEL_SHIFT,
   ccu_kpm_usb_ic_clkgate_clk_en                 = KPM_CLK_MGR_REG_USB_IC_CLKGATE_USB_IC_CLK_EN_SHIFT,
   ccu_kpm_usb_ic_clkgate_hw_sw_gating_sel       = KPM_CLK_MGR_REG_USB_IC_CLKGATE_USB_IC_HW_SW_GATING_SEL_SHIFT,
   ccu_kpm_usb_ic_clkgate_ahb_stprsts            = KPM_CLK_MGR_REG_USB_IC_CLKGATE_USB_IC_AHB_STPRSTS_SHIFT,
   ccu_kpm_usb_ic_clkgate_stprsts                = KPM_CLK_MGR_REG_USB_IC_CLKGATE_USB_IC_STPRSTS_SHIFT,
   ccu_kpm_usb_ic_clkgate_voltage_level          = KPM_CLK_MGR_REG_USB_IC_CLKGATE_USB_IC_VOLTAGE_LEVEL_SHIFT

} ccu_kpm_usb_ic_clkgate_e;

typedef enum
{
   ccu_kpm_sdio_clkgate_ahb_clk_en            = KPM_CLK_MGR_REG_SDIO1_CLKGATE_SDIO1_AHB_CLK_EN_SHIFT,
   ccu_kpm_sdio_clkgate_ahb_hw_sw_gating_sel  = KPM_CLK_MGR_REG_SDIO1_CLKGATE_SDIO1_AHB_HW_SW_GATING_SEL_SHIFT,
   ccu_kpm_sdio_clkgate_clk_en                = KPM_CLK_MGR_REG_SDIO1_CLKGATE_SDIO1_CLK_EN_SHIFT,
   ccu_kpm_sdio_clkgate_hw_sw_gating_sel      = KPM_CLK_MGR_REG_SDIO1_CLKGATE_SDIO1_HW_SW_GATING_SEL_SHIFT,
   ccu_kpm_sdio_clkgate_sleep_clk_en          = KPM_CLK_MGR_REG_SDIO1_CLKGATE_SDIO1_SLEEP_CLK_EN_SHIFT,
   ccu_kpm_sdio_clkgate_ahb_stprsts           = KPM_CLK_MGR_REG_SDIO1_CLKGATE_SDIO1_AHB_STPRSTS_SHIFT,
   ccu_kpm_sdio_clkgate_stprsts               = KPM_CLK_MGR_REG_SDIO1_CLKGATE_SDIO1_STPRSTS_SHIFT,
   ccu_kpm_sdio_clkgate_sleep_stprsts         = KPM_CLK_MGR_REG_SDIO1_CLKGATE_SDIO1_SLEEP_STPRSTS_SHIFT,
   ccu_kpm_sdio_clkgate_voltage_level         = KPM_CLK_MGR_REG_SDIO1_CLKGATE_SDIO1_VOLTAGE_LEVEL_SHIFT

} ccu_kpm_sdio_clkgate_e;

#if 0
typedef enum
{
   ccu_kpm_axi_div_sys_switch_axi_pll_sel           = KPM_CLK_MGR_REG_AXI_DIV_SYS_SWITCH_AXI_PLL_SELECT_SHIFT,
   ccu_kpm_axi_div_sys_switch_axi_div               = KPM_CLK_MGR_REG_AXI_DIV_SYS_SWITCH_AXI_DIV_SHIFT,
   ccu_kpm_axi_div_mst_switch_ahb_div               = KPM_CLK_MGR_REG_AXI_DIV_MASTER_SWITCH_AHB_DIV_SHIFT,
   ccu_kpm_axi_div_apb4_free_div                    = KPM_CLK_MGR_REG_AXI_DIV_APB4_FREE_DIV_SHIFT,
   ccu_kpm_axi_div_armcore_axi_div                  = KPM_CLK_MGR_REG_AXI_DIV_ARMCORE_AXI_DIV_SHIFT,
   ccu_kpm_axi_div_apb8_div                         = KPM_CLK_MGR_REG_AXI_DIV_APB8_DIV_SHIFT,
   ccu_kpm_axi_div_sys_switch_axi_pll_sel_override  = KPM_CLK_MGR_REG_AXI_DIV_SYS_SWITCH_AXI_PLL_SELECT_OVERRIDE_SHIFT,
   ccu_kpm_axi_div_sys_switch_axi_div_override      = KPM_CLK_MGR_REG_AXI_DIV_SYS_SWITCH_AXI_DIV_OVERRIDE_SHIFT,
   ccu_kpm_axi_div_mst_switch_ahb_div_override      = KPM_CLK_MGR_REG_AXI_DIV_MASTER_SWITCH_AHB_DIV_OVERRIDE_SHIFT,
   ccu_kpm_axi_div_apb4_free_div_override           = KPM_CLK_MGR_REG_AXI_DIV_APB4_FREE_DIV_OVERRIDE_SHIFT,
   ccu_kpm_axi_div_armcore_axi_div_override         = KPM_CLK_MGR_REG_AXI_DIV_ARMCORE_AXI_DIV_OVERRIDE_SHIFT,
   ccu_kpm_axi_div_apb8_div_override                = KPM_CLK_MGR_REG_AXI_DIV_APB8_DIV_OVERRIDE_SHIFT,
   ccu_kpm_axi_div_sys_switch_axi_trg_override      = KPM_CLK_MGR_REG_AXI_DIV_SYS_SWITCH_AXI_TRIGGER_OVERRIDE_SHIFT

} ccu_kpm_axi_div_e;
#endif

typedef enum
{
   ccu_kpm_nand_div_pll_sel = KPM_CLK_MGR_REG_NAND_DIV_NAND_PLL_SELECT_SHIFT,
   ccu_kpm_nand_div         = KPM_CLK_MGR_REG_NAND_DIV_NAND_DIV_SHIFT

} ccu_kpm_nand_div_e;

typedef enum
{
   ccu_kpm_usb_ic_div_pll_sel = KPM_CLK_MGR_REG_USB_IC_DIV_USB_IC_PLL_SELECT_SHIFT

} ccu_kpm_usb_ic_div_e;

typedef enum
{
   ccu_kpm_sdio_div_pll_sel = KPM_CLK_MGR_REG_SDIO1_DIV_SDIO1_PLL_SELECT_SHIFT,
   ccu_kpm_sdio_div         = KPM_CLK_MGR_REG_SDIO1_DIV_SDIO1_DIV_SHIFT

} ccu_kpm_sdio_div_e;

typedef enum
{
   ccu_kpm_div_trig_nand_trig           = KPM_CLK_MGR_REG_DIV_TRIG_NAND_TRIGGER_SHIFT,
   ccu_kpm_div_trig_hsic2_48m_trig       = KPM_CLK_MGR_REG_DIV_TRIG_HSIC2_48M_TRIGGER_SHIFT,
   ccu_kpm_div_trig_usb_ic_trig         = KPM_CLK_MGR_REG_DIV_TRIG_USB_IC_TRIGGER_SHIFT,
   ccu_kpm_div_trig_sdio1_trig          = KPM_CLK_MGR_REG_DIV_TRIG_SDIO1_TRIGGER_SHIFT,
   ccu_kpm_div_trig_sdio2_trig          = KPM_CLK_MGR_REG_DIV_TRIG_SDIO2_TRIGGER_SHIFT,
   ccu_kpm_div_trig_sdio4_trig          = KPM_CLK_MGR_REG_DIV_TRIG_SDIO4_TRIGGER_SHIFT,
   ccu_kpm_div_trig_sdio3_trig          = KPM_CLK_MGR_REG_DIV_TRIG_SDIO3_TRIGGER_SHIFT

} ccu_kpm_div_trig_e;

typedef enum
{
   ccu_kpm_clkmon_sel = KPM_CLK_MGR_REG_CLKMON_CLKMON_SEL_SHIFT,
   ccu_kpm_clkmon_ctl = KPM_CLK_MGR_REG_CLKMON_CLKMON_CTL_SHIFT

} ccu_kpm_clkmon_e;




/* Function Prototypes */

static inline void      ccu_set_kpm_peri_volt( ccu_kpm_peri_volt_e field, uint32_t val );
static inline uint32_t  ccu_get_kpm_peri_volt( ccu_kpm_peri_volt_e field );
static inline void      ccu_set_kpm_lvm( ccu_kpm_lvm_e field, uint32_t val );
static inline uint32_t  ccu_get_kpm_lvm( ccu_kpm_lvm_e field );
static inline void      ccu_set_kpm_vlt( ccu_kpm_vlt_e field, uint32_t val );
static inline uint32_t  ccu_get_kpm_vlt( ccu_kpm_vlt_e field );
static inline void      ccu_set_kpm_bus_quiesc( ccu_kpm_bus_quiesc_e field, uint32_t val );
static inline uint32_t  ccu_get_kpm_bus_quiesc( ccu_kpm_bus_quiesc_e field );
static inline void      ccu_set_kpm_axi_sys_switch_clkgate( ccu_kpm_axi_sys_switch_clkgate_e field, uint32_t val );
static inline uint32_t  ccu_get_kpm_axi_sys_switch_clkgate( ccu_kpm_axi_sys_switch_clkgate_e field );
static inline void      ccu_set_kpm_axi_mst_switch_clkgate( ccu_kpm_axi_mst_switch_clkgate_e field, uint32_t val );
static inline uint32_t  ccu_get_kpm_axi_mst_switch_clkgate( ccu_kpm_axi_mst_switch_clkgate_e field );
static inline void      ccu_set_kpm_axi_core_clkgate( ccu_kpm_axi_core_clkgate_e field, uint32_t val );
static inline uint32_t  ccu_get_kpm_axi_core_clkgate( ccu_kpm_axi_core_clkgate_e field );
static inline void      ccu_set_kpm_apb4_clkgate( ccu_kpm_apb4_clkgate_e field, uint32_t val );
static inline uint32_t  ccu_get_kpm_apb4_clkgate( ccu_kpm_apb4_clkgate_e field );
static inline void      ccu_set_kpm_apb8_clkgate( ccu_kpm_apb8_clkgate_e field, uint32_t val );
static inline uint32_t  ccu_get_kpm_apb8_clkgate( ccu_kpm_apb8_clkgate_e field );
static inline void      ccu_set_kpm_dmac_clkgate( ccu_kpm_dmac_clkgate_e field, uint32_t val );
static inline uint32_t  ccu_get_kpm_dmac_clkgate( ccu_kpm_dmac_clkgate_e field );
static inline void      ccu_set_kpm_nand_clkgate( ccu_kpm_nand_clkgate_e field, uint32_t val );
static inline uint32_t  ccu_get_kpm_nand_clkgate( ccu_kpm_nand_clkgate_e field );
static inline void      ccu_set_kpm_usb_otg_clkgate( ccu_kpm_usb_otg_clkgate_e field, uint32_t val );
static inline uint32_t  ccu_get_kpm_usb_otg_clkgate( ccu_kpm_usb_otg_clkgate_e field );
static inline void      ccu_set_kpm_usb_ic_clkgate( ccu_kpm_usb_ic_clkgate_e field, uint32_t val );
static inline uint32_t  ccu_get_kpm_usb_ic_clkgate( ccu_kpm_usb_ic_clkgate_e field );
static inline void      ccu_set_kpm_sdio1_clkgate( ccu_kpm_sdio_clkgate_e field, uint32_t val );
static inline uint32_t  ccu_get_kpm_sdio1_clkgate( ccu_kpm_sdio_clkgate_e field );
static inline void      ccu_set_kpm_sdio2_clkgate( ccu_kpm_sdio_clkgate_e field, uint32_t val );
static inline uint32_t  ccu_get_kpm_sdio2_clkgate( ccu_kpm_sdio_clkgate_e field );
static inline void      ccu_set_kpm_sdio3_clkgate( ccu_kpm_sdio_clkgate_e field, uint32_t val );
static inline uint32_t  ccu_get_kpm_sdio3_clkgate( ccu_kpm_sdio_clkgate_e field );
static inline void      ccu_set_kpm_sdio4_clkgate( ccu_kpm_sdio_clkgate_e field, uint32_t val );
static inline uint32_t  ccu_get_kpm_sdio4_clkgate( ccu_kpm_sdio_clkgate_e field );
#if 0
static inline void      ccu_set_kpm_axi_div( ccu_kpm_axi_div_e field, uint32_t val );
static inline uint32_t  ccu_get_kpm_axi_div( ccu_kpm_axi_div_e field );
#endif
static inline void      ccu_set_kpm_nand_div( ccu_kpm_nand_div_e field, uint32_t val );
static inline uint32_t  ccu_get_kpm_nand_div( ccu_kpm_nand_div_e field );
static inline void      ccu_set_kpm_usb_ic_div( ccu_kpm_usb_ic_div_e field, uint32_t val );
static inline uint32_t  ccu_get_kpm_usb_ic_div( ccu_kpm_usb_ic_div_e field );
static inline void      ccu_set_kpm_sdio1_div( ccu_kpm_sdio_div_e field, uint32_t val );
static inline uint32_t  ccu_get_kpm_sdio1_div( ccu_kpm_sdio_div_e field );
static inline void      ccu_set_kpm_sdio2_div( ccu_kpm_sdio_div_e field, uint32_t val );
static inline uint32_t  ccu_get_kpm_sdio2_div( ccu_kpm_sdio_div_e field );
static inline void      ccu_set_kpm_sdio3_div( ccu_kpm_sdio_div_e field, uint32_t val );
static inline uint32_t  ccu_get_kpm_sdio3_div( ccu_kpm_sdio_div_e field );
static inline void      ccu_set_kpm_sdio4_div( ccu_kpm_sdio_div_e field, uint32_t val );
static inline uint32_t  ccu_get_kpm_sdio4_div( ccu_kpm_sdio_div_e field );
static inline void      ccu_set_kpm_div_trig( ccu_kpm_div_trig_e field, uint32_t val );
static inline uint32_t  ccu_get_kpm_div_trig( ccu_kpm_div_trig_e field );
static inline void      ccu_set_kpm_clkmon( ccu_kpm_clkmon_e field, uint32_t val );
static inline uint32_t  ccu_get_kpm_clkmon( ccu_kpm_clkmon_e field );




/* Local macros */
#define ccu_set_kpm_bit(offset, field, val)  \
            ccu_set_bit( (MM_IO_BASE_KPM_CLK + (offset)), (field), (val) )

#define ccu_get_kpm_bit(offset, field)       \
            ccu_get_bit( (MM_IO_BASE_KPM_CLK + (offset)), (field) )

#define ccu_set_kpm_reg_field(offset, mask, shift, val)  \
            ccu_set_reg_field( (MM_IO_BASE_KPM_CLK + (offset)), (mask), (shift), (val) );

#define ccu_get_kpm_reg_field(offset, mask, shift)       \
            ccu_get_reg_field( (MM_IO_BASE_KPM_CLK + (offset)), (mask), (shift) )


/* Functions */

static inline void ccu_set_kpm_peri_volt( ccu_kpm_peri_volt_e field, uint32_t val )
{
   uint32_t mask;

   if (field == ccu_kpm_peri_volt_normal)
   {
      mask = KPM_CLK_MGR_REG_VLT_PERI_VLT_NORMAL_PERI_MASK;
   }
   else if (field == ccu_kpm_peri_volt_high)
   {
      mask = KPM_CLK_MGR_REG_VLT_PERI_VLT_HIGH_PERI_MASK;
   }
   else
   {
      mask = 1 << field;
   }
   ccu_set_kpm_reg_field( KPM_CLK_MGR_REG_VLT_PERI_OFFSET, mask, field, val );
}

static inline uint32_t ccu_get_kpm_peri_volt( ccu_kpm_peri_volt_e field )
{
   uint32_t mask;

   if (field == ccu_kpm_peri_volt_normal)
   {
      mask = KPM_CLK_MGR_REG_VLT_PERI_VLT_NORMAL_PERI_MASK;
   }
   else if (field == ccu_kpm_peri_volt_high)
   {
      mask = KPM_CLK_MGR_REG_VLT_PERI_VLT_HIGH_PERI_MASK;
   }
   else
   {
      mask = 1 << field;
   }
   return ccu_get_kpm_reg_field( KPM_CLK_MGR_REG_VLT_PERI_OFFSET, mask, field );
}

static inline void ccu_set_kpm_lvm( ccu_kpm_lvm_e field, uint32_t val )
{
   uint32_t shift;
   uint32_t mask;
   uint32_t reg_offset;

   if (field < ccu_kpm_lvm_4 )
   {
      reg_offset = KPM_CLK_MGR_REG_LVM0_3_OFFSET;
   }
   else
   {
      reg_offset = KPM_CLK_MGR_REG_LVM4_7_OFFSET;
      field = field - ccu_kpm_lvm_4;
   }

   shift = field * 4;
   mask  = KPM_CLK_MGR_REG_LVM0_3_LVM0_3_MD_00_MASK << (shift);

   ccu_set_kpm_reg_field( reg_offset, mask, shift, val );
}

static inline uint32_t ccu_get_kpm_lvm( ccu_kpm_lvm_e field )
{
   uint32_t shift;
   uint32_t mask;
   uint32_t reg_offset;

   if (field < ccu_kpm_lvm_4 )
   {
      reg_offset = KPM_CLK_MGR_REG_LVM0_3_OFFSET;
   }
   else
   {
      reg_offset = KPM_CLK_MGR_REG_LVM4_7_OFFSET;
      field = field - ccu_kpm_lvm_4;
   }

   shift = field * 4;
   mask  = KPM_CLK_MGR_REG_LVM0_3_LVM0_3_MD_00_MASK << (shift);

   return ccu_get_kpm_reg_field( reg_offset, mask, shift );
}

static inline void ccu_set_kpm_vlt( ccu_kpm_vlt_e field, uint32_t val )
{
   uint32_t shift;
   uint32_t mask;
   uint32_t reg_offset;

   if (field < ccu_kpm_vlt_4 )
   {
      reg_offset = KPM_CLK_MGR_REG_VLT0_3_OFFSET;
   }
   else
   {
      reg_offset = KPM_CLK_MGR_REG_VLT4_7_OFFSET;
      field = field - ccu_kpm_vlt_4;
   }

   shift = field * 8;
   mask  = KPM_CLK_MGR_REG_VLT0_3_VLT0_3_VV_00_MASK << (shift);

   ccu_set_kpm_reg_field( reg_offset, mask, shift, val );
}

static inline uint32_t ccu_get_kpm_vlt( ccu_kpm_vlt_e field )
{
   uint32_t shift;
   uint32_t mask;
   uint32_t reg_offset;

   if (field < ccu_kpm_vlt_4 )
   {
      reg_offset = KPM_CLK_MGR_REG_VLT0_3_OFFSET;
   }
   else
   {
      reg_offset = KPM_CLK_MGR_REG_VLT4_7_OFFSET;
      field = field - ccu_kpm_vlt_4;
   }

   shift = field * 8;
   mask  = KPM_CLK_MGR_REG_VLT0_3_VLT0_3_VV_00_MASK << (shift);

   return ccu_get_kpm_reg_field( reg_offset, mask, shift );
}

static inline void ccu_set_kpm_bus_quiesc( ccu_kpm_bus_quiesc_e field, uint32_t val )
{
   ccu_set_kpm_bit( KPM_CLK_MGR_REG_BUS_QUIESC_OFFSET, field, val );
}

static inline uint32_t ccu_get_kpm_bus_quiesc( ccu_kpm_bus_quiesc_e field )
{
   return ccu_get_kpm_bit( KPM_CLK_MGR_REG_BUS_QUIESC_OFFSET, field );
}

static inline void ccu_set_kpm_axi_sys_switch_clkgate( ccu_kpm_axi_sys_switch_clkgate_e field, uint32_t val )
{
   ccu_set_kpm_bit( KPM_CLK_MGR_REG_AXI_SYS_SWITCH_CLKGATE_OFFSET, field, val );
}

static inline uint32_t ccu_get_kpm_axi_sys_switch_clkgate( ccu_kpm_axi_sys_switch_clkgate_e field )
{
   return ccu_get_kpm_bit( KPM_CLK_MGR_REG_AXI_SYS_SWITCH_CLKGATE_OFFSET, field );
}

static inline void ccu_set_kpm_axi_mst_switch_clkgate( ccu_kpm_axi_mst_switch_clkgate_e field, uint32_t val )
{
   ccu_set_kpm_bit( KPM_CLK_MGR_REG_AXI_MST_SWITCH_CLKGATE_OFFSET, field, val );
}

static inline uint32_t ccu_get_kpm_axi_mst_switch_clkgate( ccu_kpm_axi_mst_switch_clkgate_e field )
{
   return ccu_get_kpm_bit( KPM_CLK_MGR_REG_AXI_MST_SWITCH_CLKGATE_OFFSET, field );
}

static inline void ccu_set_kpm_axi_core_clkgate( ccu_kpm_axi_core_clkgate_e field, uint32_t val )
{
   ccu_set_kpm_bit( KPM_CLK_MGR_REG_AXI_CORE_CLKGATE_OFFSET, field, val );
}

static inline uint32_t ccu_get_kpm_axi_core_clkgate( ccu_kpm_axi_core_clkgate_e field )
{
   return ccu_get_kpm_bit( KPM_CLK_MGR_REG_AXI_CORE_CLKGATE_OFFSET, field );
}

static inline void ccu_set_kpm_apb4_clkgate( ccu_kpm_apb4_clkgate_e field, uint32_t val )
{
   ccu_set_kpm_bit( KPM_CLK_MGR_REG_APB4_CLKGATE_OFFSET, field, val );
}

static inline uint32_t ccu_get_kpm_apb4_clkgate( ccu_kpm_apb4_clkgate_e field )
{
   return ccu_get_kpm_bit( KPM_CLK_MGR_REG_APB4_CLKGATE_OFFSET, field );
}

static inline void ccu_set_kpm_apb8_clkgate( ccu_kpm_apb8_clkgate_e field, uint32_t val )
{
   ccu_set_kpm_bit( KPM_CLK_MGR_REG_APB8_CLKGATE_OFFSET, field, val );
}

static inline uint32_t ccu_get_kpm_apb8_clkgate( ccu_kpm_apb8_clkgate_e field )
{
   return ccu_get_kpm_bit( KPM_CLK_MGR_REG_APB8_CLKGATE_OFFSET, field );
}

static inline void ccu_set_kpm_dmac_clkgate( ccu_kpm_dmac_clkgate_e field, uint32_t val )
{
   ccu_set_kpm_bit( KPM_CLK_MGR_REG_DMAC_CLKGATE_OFFSET, field, val );
}

static inline uint32_t ccu_get_kpm_dmac_clkgate( ccu_kpm_dmac_clkgate_e field )
{
   return ccu_get_kpm_bit( KPM_CLK_MGR_REG_DMAC_CLKGATE_OFFSET, field );
}

static inline void ccu_set_kpm_nand_clkgate( ccu_kpm_nand_clkgate_e field, uint32_t val )
{
   ccu_set_kpm_bit( KPM_CLK_MGR_REG_NAND_CLKGATE_OFFSET, field, val );
}

static inline uint32_t ccu_get_kpm_nand_clkgate( ccu_kpm_nand_clkgate_e field )
{
   return ccu_get_kpm_bit( KPM_CLK_MGR_REG_NAND_CLKGATE_OFFSET, field );
}

static inline void ccu_set_kpm_usb_otg_clkgate( ccu_kpm_usb_otg_clkgate_e field, uint32_t val )
{
   ccu_set_kpm_bit( KPM_CLK_MGR_REG_USB_OTG_CLKGATE_OFFSET, field, val );
}

static inline uint32_t ccu_get_kpm_usb_otg_clkgate( ccu_kpm_usb_otg_clkgate_e field )
{
   return ccu_get_kpm_bit( KPM_CLK_MGR_REG_USB_OTG_CLKGATE_OFFSET, field );
}

static inline void ccu_set_kpm_usb_ic_clkgate( ccu_kpm_usb_ic_clkgate_e field, uint32_t val )
{
   ccu_set_kpm_bit( KPM_CLK_MGR_REG_USB_IC_CLKGATE_OFFSET, field, val );
}

static inline uint32_t ccu_get_kpm_usb_ic_clkgate( ccu_kpm_usb_ic_clkgate_e field )
{
   return ccu_get_kpm_bit( KPM_CLK_MGR_REG_USB_IC_CLKGATE_OFFSET, field );
}

static inline void ccu_set_kpm_sdio1_clkgate( ccu_kpm_sdio_clkgate_e field, uint32_t val )
{
   ccu_set_kpm_bit( KPM_CLK_MGR_REG_SDIO1_CLKGATE_OFFSET, field, val );
}

static inline uint32_t ccu_get_kpm_sdio1_clkgate( ccu_kpm_sdio_clkgate_e field )
{
   return ccu_get_kpm_bit( KPM_CLK_MGR_REG_SDIO1_CLKGATE_OFFSET, field );
}

static inline void ccu_set_kpm_sdio2_clkgate( ccu_kpm_sdio_clkgate_e field, uint32_t val )
{
   ccu_set_kpm_bit( KPM_CLK_MGR_REG_SDIO2_CLKGATE_OFFSET, field, val );
}

static inline uint32_t ccu_get_kpm_sdio2_clkgate( ccu_kpm_sdio_clkgate_e field )
{
   return ccu_get_kpm_bit( KPM_CLK_MGR_REG_SDIO2_CLKGATE_OFFSET, field );
}

static inline void ccu_set_kpm_sdio3_clkgate( ccu_kpm_sdio_clkgate_e field, uint32_t val )
{
   ccu_set_kpm_bit( KPM_CLK_MGR_REG_SDIO3_CLKGATE_OFFSET, field, val );
}

static inline uint32_t ccu_get_kpm_sdio3_clkgate( ccu_kpm_sdio_clkgate_e field )
{
   return ccu_get_kpm_bit( KPM_CLK_MGR_REG_SDIO3_CLKGATE_OFFSET, field );
}

static inline void ccu_set_kpm_sdio4_clkgate( ccu_kpm_sdio_clkgate_e field, uint32_t val )
{
   ccu_set_kpm_bit( KPM_CLK_MGR_REG_SDIO4_CLKGATE_OFFSET, field, val );
}

static inline uint32_t ccu_get_kpm_sdio4_clkgate( ccu_kpm_sdio_clkgate_e field )
{
   return ccu_get_kpm_bit( KPM_CLK_MGR_REG_SDIO4_CLKGATE_OFFSET, field );
}

#if 0
static inline void ccu_set_kpm_axi_div( ccu_kpm_axi_div_e field, uint32_t val )
{
   uint32_t mask;

   switch( (uint32_t)field )
   {
      case ccu_kpm_axi_div_sys_switch_axi_pll_sel:
         mask = KPM_CLK_MGR_REG_AXI_DIV_SYS_SWITCH_AXI_PLL_SELECT_MASK;
         break;

      case ccu_kpm_axi_div_sys_switch_axi_div:
         mask = KPM_CLK_MGR_REG_AXI_DIV_SYS_SWITCH_AXI_DIV_MASK;
         break;

      case ccu_kpm_axi_div_mst_switch_ahb_div:
         mask = KPM_CLK_MGR_REG_AXI_DIV_MASTER_SWITCH_AHB_DIV_MASK;
         break;

      case ccu_kpm_axi_div_apb4_free_div:
         mask = KPM_CLK_MGR_REG_AXI_DIV_APB4_FREE_DIV_MASK;
         break;

      case ccu_kpm_axi_div_armcore_axi_div:
         mask = KPM_CLK_MGR_REG_AXI_DIV_ARMCORE_AXI_DIV_MASK;
         break;

      case ccu_kpm_axi_div_apb8_div:
         mask = KPM_CLK_MGR_REG_AXI_DIV_APB8_DIV_MASK;
         break;

      default:
         mask = 1 << field;
         break;
   }
   ccu_set_kpm_reg_field(KPM_CLK_MGR_REG_AXI_DIV_OFFSET, mask, field, val);
}

static inline uint32_t ccu_get_kpm_axi_div( ccu_kpm_axi_div_e field )
{
   uint32_t mask;

   switch( (uint32_t)field )
   {
      case ccu_kpm_axi_div_sys_switch_axi_pll_sel:
         mask = KPM_CLK_MGR_REG_AXI_DIV_SYS_SWITCH_AXI_PLL_SELECT_MASK;
         break;

      case ccu_kpm_axi_div_sys_switch_axi_div:
         mask = KPM_CLK_MGR_REG_AXI_DIV_SYS_SWITCH_AXI_DIV_MASK;
         break;

      case ccu_kpm_axi_div_mst_switch_ahb_div:
         mask = KPM_CLK_MGR_REG_AXI_DIV_MASTER_SWITCH_AHB_DIV_MASK;
         break;

      case ccu_kpm_axi_div_apb4_free_div:
         mask = KPM_CLK_MGR_REG_AXI_DIV_APB4_FREE_DIV_MASK;
         break;

      case ccu_kpm_axi_div_armcore_axi_div:
         mask = KPM_CLK_MGR_REG_AXI_DIV_ARMCORE_AXI_DIV_MASK;
         break;

      case ccu_kpm_axi_div_apb8_div:
         mask = KPM_CLK_MGR_REG_AXI_DIV_APB8_DIV_MASK;
         break;

      default:
         mask = 1 << field;
         break;
   }
   return ccu_get_kpm_reg_field(KPM_CLK_MGR_REG_AXI_DIV_OFFSET, mask, field);
}
#endif

static inline void ccu_set_kpm_nand_div( ccu_kpm_nand_div_e field, uint32_t val )
{
   uint32_t mask;

   if (field == ccu_kpm_nand_div_pll_sel)
   {
      mask = KPM_CLK_MGR_REG_NAND_DIV_NAND_PLL_SELECT_MASK;
   }
   else if (field == ccu_kpm_nand_div)
   {
      mask = KPM_CLK_MGR_REG_NAND_DIV_NAND_DIV_MASK;
   }
   else
   {
      mask = 1 << field;
   }
   ccu_set_kpm_reg_field(KPM_CLK_MGR_REG_NAND_DIV_OFFSET, mask, field, val);
}

static inline uint32_t ccu_get_kpm_nand_div( ccu_kpm_nand_div_e field )
{
   uint32_t mask;

   if (field == ccu_kpm_nand_div_pll_sel)
   {
      mask = KPM_CLK_MGR_REG_NAND_DIV_NAND_PLL_SELECT_MASK;
   }
   else if (field == ccu_kpm_nand_div)
   {
      mask = KPM_CLK_MGR_REG_NAND_DIV_NAND_DIV_MASK;
   }
   else
   {
      mask = 1 << field;
   }
   return ccu_get_kpm_reg_field( KPM_CLK_MGR_REG_NAND_DIV_OFFSET, mask, field );
}

static inline void ccu_set_kpm_usb_ic_div( ccu_kpm_usb_ic_div_e field, uint32_t val )
{
   uint32_t mask;

   if (field == ccu_kpm_usb_ic_div_pll_sel)
   {
      mask = KPM_CLK_MGR_REG_USB_IC_DIV_USB_IC_PLL_SELECT_MASK;
   }
   else
   {
      mask = 1 << field;
   }
   ccu_set_kpm_reg_field(KPM_CLK_MGR_REG_USB_IC_DIV_OFFSET, mask, field, val );
}

static inline uint32_t ccu_get_kpm_usb_ic_div( ccu_kpm_usb_ic_div_e field )
{
   uint32_t mask;

   if (field == ccu_kpm_usb_ic_div_pll_sel)
   {
      mask = KPM_CLK_MGR_REG_USB_IC_DIV_USB_IC_PLL_SELECT_MASK;
   }
   else
   {
      mask = 1 << field;
   }
   return ccu_get_kpm_reg_field(KPM_CLK_MGR_REG_USB_IC_DIV_OFFSET, mask, field );
}

static inline void ccu_set_kpm_sdio1_div( ccu_kpm_sdio_div_e field, uint32_t val )
{
   uint32_t mask;

   if (field == ccu_kpm_sdio_div_pll_sel)
   {
      mask = KPM_CLK_MGR_REG_SDIO1_DIV_SDIO1_PLL_SELECT_MASK;
   }
   else if (field == ccu_kpm_sdio_div)
   {
      mask = KPM_CLK_MGR_REG_SDIO1_DIV_SDIO1_DIV_MASK;
   }
   else
   {
      mask = 1 << field;
   }
   ccu_set_kpm_reg_field(KPM_CLK_MGR_REG_SDIO1_DIV_OFFSET, mask, field, val );
}

static inline uint32_t ccu_get_kpm_sdio1_div( ccu_kpm_sdio_div_e field )
{
   uint32_t mask;

   if (field == ccu_kpm_sdio_div_pll_sel)
   {
      mask = KPM_CLK_MGR_REG_SDIO1_DIV_SDIO1_PLL_SELECT_MASK;
   }
   else if (field == ccu_kpm_sdio_div)
   {
      mask = KPM_CLK_MGR_REG_SDIO1_DIV_SDIO1_DIV_MASK;
   }
   else
   {
      mask = 1 << field;
   }
   return ccu_get_kpm_reg_field(KPM_CLK_MGR_REG_SDIO1_DIV_OFFSET, mask, field );
}

static inline void ccu_set_kpm_sdio2_div( ccu_kpm_sdio_div_e field, uint32_t val )
{
   uint32_t mask;

   if (field == ccu_kpm_sdio_div_pll_sel)
   {
      mask = KPM_CLK_MGR_REG_SDIO1_DIV_SDIO1_PLL_SELECT_MASK;
   }
   else if (field == ccu_kpm_sdio_div)
   {
      mask = KPM_CLK_MGR_REG_SDIO1_DIV_SDIO1_DIV_MASK;
   }
   else
   {
      mask = 1 << field;
   }
   ccu_set_kpm_reg_field(KPM_CLK_MGR_REG_SDIO2_DIV_OFFSET, mask, field, val );
}

static inline uint32_t ccu_get_kpm_sdio2_div( ccu_kpm_sdio_div_e field )
{
   uint32_t mask;

   if (field == ccu_kpm_sdio_div_pll_sel)
   {
      mask = KPM_CLK_MGR_REG_SDIO1_DIV_SDIO1_PLL_SELECT_MASK;
   }
   else if (field == ccu_kpm_sdio_div)
   {
      mask = KPM_CLK_MGR_REG_SDIO1_DIV_SDIO1_DIV_MASK;
   }
   else
   {
      mask = 1 << field;
   }
   return ccu_get_kpm_reg_field(KPM_CLK_MGR_REG_SDIO2_DIV_OFFSET, mask, field );
}

static inline void ccu_set_kpm_sdio3_div( ccu_kpm_sdio_div_e field, uint32_t val )
{
   uint32_t mask;

   if (field == ccu_kpm_sdio_div_pll_sel)
   {
      mask = KPM_CLK_MGR_REG_SDIO1_DIV_SDIO1_PLL_SELECT_MASK;
   }
   else if (field == ccu_kpm_sdio_div)
   {
      mask = KPM_CLK_MGR_REG_SDIO1_DIV_SDIO1_DIV_MASK;
   }
   else
   {
      mask = 1 << field;
   }
   ccu_set_kpm_reg_field(KPM_CLK_MGR_REG_SDIO3_DIV_OFFSET, mask, field, val );
}

static inline uint32_t ccu_get_kpm_sdio3_div( ccu_kpm_sdio_div_e field )
{
   uint32_t mask;

   if (field == ccu_kpm_sdio_div_pll_sel)
   {
      mask = KPM_CLK_MGR_REG_SDIO1_DIV_SDIO1_PLL_SELECT_MASK;
   }
   else if (field == ccu_kpm_sdio_div)
   {
      mask = KPM_CLK_MGR_REG_SDIO1_DIV_SDIO1_DIV_MASK;
   }
   else
   {
      mask = 1 << field;
   }
   return ccu_get_kpm_reg_field(KPM_CLK_MGR_REG_SDIO3_DIV_OFFSET, mask, field );
}

static inline void ccu_set_kpm_sdio4_div( ccu_kpm_sdio_div_e field, uint32_t val )
{
   uint32_t mask;

   if (field == ccu_kpm_sdio_div_pll_sel)
   {
      mask = KPM_CLK_MGR_REG_SDIO1_DIV_SDIO1_PLL_SELECT_MASK;
   }
   else if (field == ccu_kpm_sdio_div)
   {
      mask = KPM_CLK_MGR_REG_SDIO1_DIV_SDIO1_DIV_MASK;
   }
   else
   {
      mask = 1 << field;
   }
   ccu_set_kpm_reg_field(KPM_CLK_MGR_REG_SDIO4_DIV_OFFSET, mask, field, val );
}

static inline uint32_t ccu_get_kpm_sdio4_div( ccu_kpm_sdio_div_e field )
{
   uint32_t mask;

   if (field == ccu_kpm_sdio_div_pll_sel)
   {
      mask = KPM_CLK_MGR_REG_SDIO1_DIV_SDIO1_PLL_SELECT_MASK;
   }
   else if (field == ccu_kpm_sdio_div)
   {
      mask = KPM_CLK_MGR_REG_SDIO1_DIV_SDIO1_DIV_MASK;
   }
   else
   {
      mask = 1 << field;
   }
   return ccu_get_kpm_reg_field(KPM_CLK_MGR_REG_SDIO4_DIV_OFFSET, mask, field );
}

static inline void ccu_set_kpm_div_trig( ccu_kpm_div_trig_e field, uint32_t val )
{
   ccu_set_kpm_bit( KPM_CLK_MGR_REG_DIV_TRIG_OFFSET, field, val );
}

static inline uint32_t ccu_get_kpm_div_trig( ccu_kpm_div_trig_e field )
{
   return ccu_get_kpm_bit( KPM_CLK_MGR_REG_DIV_TRIG_OFFSET, field );
}

static inline void ccu_set_kpm_clkmon( ccu_kpm_clkmon_e field, uint32_t val )
{
   uint32_t mask;

   if (field == ccu_kpm_clkmon_sel)
   {
      mask = KPM_CLK_MGR_REG_CLKMON_CLKMON_SEL_MASK;
   }
   else if (field == ccu_kpm_clkmon_ctl)
   {
      mask = KPM_CLK_MGR_REG_CLKMON_CLKMON_CTL_MASK;
   }
   else
   {
      mask = 1 << field;
   }
   ccu_set_kpm_reg_field( KPM_CLK_MGR_REG_CLKMON_OFFSET, mask, field, val );
}

static inline uint32_t ccu_get_kpm_clkmon( ccu_kpm_clkmon_e field )
{
   uint32_t mask;

   if (field == ccu_kpm_clkmon_sel)
   {
      mask = KPM_CLK_MGR_REG_CLKMON_CLKMON_SEL_MASK;
   }
   else if (field == ccu_kpm_clkmon_ctl)
   {
      mask = KPM_CLK_MGR_REG_CLKMON_CLKMON_CTL_MASK;
   }
   else
   {
      mask = 1 << field;
   }
   return ccu_get_kpm_reg_field( KPM_CLK_MGR_REG_CLKMON_OFFSET, mask, field );
}







#ifdef __cplusplus
}
#endif

#endif /* _ccu_KPM_INLINE_H_*/

