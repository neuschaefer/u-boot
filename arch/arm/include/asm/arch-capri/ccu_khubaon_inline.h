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
* @file  ccu_khubaon_inline.c
*
* @brief KONA HUB Alway On Clock Control Unit inline functions
*
* @note
*
*******************************************************************************/
#ifndef _CCU_KHUBAON_INLINE_H_
#define _CCU_KHUBAON_INLINE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <asm/arch/ccu_inline.h>
#include <asm/arch/ccu_util_inline.h>
#include <asm/arch/brcm_rdb_khubaon_clk_mgr_reg.h>


/* type definitions */

typedef enum
{
   ccu_khubaon_peri_volt_normal = KHUBAON_CLK_MGR_REG_VLT_PERI_VLT_HIGH_PERI_SHIFT,
   ccu_khubaon_peri_volt_high   = KHUBAON_CLK_MGR_REG_VLT_PERI_VLT_NORMAL_PERI_SHIFT

} ccu_khubaon_peri_volt_e;

typedef enum
{
   ccu_khubaon_lvm_0 = 0,
   ccu_khubaon_lvm_1,
   ccu_khubaon_lvm_2,
   ccu_khubaon_lvm_3,
   ccu_khubaon_lvm_4,
   ccu_khubaon_lvm_5,
   ccu_khubaon_lvm_6,
   ccu_khubaon_lvm_7

} ccu_khubaon_lvm_e;

typedef enum
{
   ccu_khubaon_vlt_0 = 0,
   ccu_khubaon_vlt_1,
   ccu_khubaon_vlt_2,
   ccu_khubaon_vlt_3,
   ccu_khubaon_vlt_4,
   ccu_khubaon_vlt_5,
   ccu_khubaon_vlt_6,
   ccu_khubaon_vlt_7

} ccu_khubaon_vlt_e;

typedef enum
{
   ccu_khubaon_hub_clkgate_hw_sw_gating_sel   = KHUBAON_CLK_MGR_REG_HUB_CLKGATE_HUBAON_HW_SW_GATING_SEL_SHIFT,
   ccu_khubaon_hub_clkgate_hyst_val           = KHUBAON_CLK_MGR_REG_HUB_CLKGATE_HUBAON_HYST_VAL_SHIFT,
   ccu_khubaon_hub_clkgate_hyst_en            = KHUBAON_CLK_MGR_REG_HUB_CLKGATE_HUBAON_HYST_EN_SHIFT,
   ccu_khubaon_hub_clkgate_stprsts            = KHUBAON_CLK_MGR_REG_HUB_CLKGATE_HUBAON_STPRSTS_SHIFT,
   ccu_khubaon_hub_clkgate_voltate_level      = KHUBAON_CLK_MGR_REG_HUB_CLKGATE_HUBAON_VOLTAGE_LEVEL_SHIFT

} ccu_khubaon_hub_clkgate_e;

typedef enum
{
   ccu_khubaon_pwrmgr_clkgate_axi_clk_en           = KHUBAON_CLK_MGR_REG_PWRMGR_CLKGATE_PWRMGR_AXI_CLK_EN_SHIFT,
   ccu_khubaon_pwrmgr_clkgate_axi_hw_sw_gating_sel = KHUBAON_CLK_MGR_REG_PWRMGR_CLKGATE_PWRMGR_AXI_HW_SW_GATING_SEL_SHIFT,
   ccu_khubaon_pwrmgr_clkgate_axi_stprsts          = KHUBAON_CLK_MGR_REG_PWRMGR_CLKGATE_PWRMGR_AXI_STPRSTS_SHIFT

} ccu_khubaon_pwrmgr_clkgate_e;

typedef enum
{
   ccu_khubaon_apb6_clkgate_hw_sw_gating_sel  = KHUBAON_CLK_MGR_REG_APB6_CLKGATE_APB6_HW_SW_GATING_SEL_SHIFT,
   ccu_khubaon_apb6_clkgate_stprsts           = KHUBAON_CLK_MGR_REG_APB6_CLKGATE_APB6_STPRSTS_SHIFT

} ccu_khubaon_apb6_clkgate_e;

typedef enum
{
   ccu_khubaon_gpiokp_clkgate_apb_clk_en            = KHUBAON_CLK_MGR_REG_GPIOKP_CLKGATE_GPIOKP_APB_CLK_EN_SHIFT,
   ccu_khubaon_gpiokp_clkgate_apb_hw_sw_gating_sel  = KHUBAON_CLK_MGR_REG_GPIOKP_CLKGATE_GPIOKP_APB_HW_SW_GATING_SEL_SHIFT,
   ccu_khubaon_gpiokp_clkgate_apb_hyst_val          = KHUBAON_CLK_MGR_REG_GPIOKP_CLKGATE_GPIOKP_APB_HYST_VAL_SHIFT,
   ccu_khubaon_gpiokp_clkgate_apb_hyst_en           = KHUBAON_CLK_MGR_REG_GPIOKP_CLKGATE_GPIOKP_APB_HYST_EN_SHIFT,
   ccu_khubaon_gpiokp_clkgate_apb_stprsts           = KHUBAON_CLK_MGR_REG_GPIOKP_CLKGATE_GPIOKP_APB_STPRSTS_SHIFT,
   ccu_khubaon_gpiokp_clkgate_voltage_level         = KHUBAON_CLK_MGR_REG_GPIOKP_CLKGATE_GPIOKP_VOLTAGE_LEVEL_SHIFT

} ccu_khubaon_gpiokp_clkgate_e;

typedef enum
{
   ccu_khubaon_hubtmr_clkgate_clk_en                = KHUBAON_CLK_MGR_REG_HUB_TIMER_CLKGATE_HUB_TIMER_CLK_EN_SHIFT,
   ccu_khubaon_hubtmr_clkgate_hw_sw_gating_sel      = KHUBAON_CLK_MGR_REG_HUB_TIMER_CLKGATE_HUB_TIMER_HW_SW_GATING_SEL_SHIFT,
   ccu_khubaon_hubtmr_clkgate_apb_clk_en            = KHUBAON_CLK_MGR_REG_HUB_TIMER_CLKGATE_HUB_TIMER_APB_CLK_EN_SHIFT,
   ccu_khubaon_hubtmr_clkgate_apb_hw_sw_gating_sel  = KHUBAON_CLK_MGR_REG_HUB_TIMER_CLKGATE_HUB_TIMER_APB_HW_SW_GATING_SEL_SHIFT,
   ccu_khubaon_hubtmr_clkgate_hyst_val              = KHUBAON_CLK_MGR_REG_HUB_TIMER_CLKGATE_HUB_TIMER_HYST_VAL_SHIFT,
   ccu_khubaon_hubtmr_clkgate_hyst_en               = KHUBAON_CLK_MGR_REG_HUB_TIMER_CLKGATE_HUB_TIMER_HYST_EN_SHIFT,
   ccu_khubaon_hubtmr_clkgate_apb_hyst_val          = KHUBAON_CLK_MGR_REG_HUB_TIMER_CLKGATE_HUB_TIMER_APB_HYST_VAL_SHIFT,
   ccu_khubaon_hubtmr_clkgate_apb_hyst_en           = KHUBAON_CLK_MGR_REG_HUB_TIMER_CLKGATE_HUB_TIMER_APB_HYST_EN_SHIFT,
   ccu_khubaon_hubtmr_clkgate_stprsts               = KHUBAON_CLK_MGR_REG_HUB_TIMER_CLKGATE_HUB_TIMER_STPRSTS_SHIFT,
   ccu_khubaon_hubtmr_clkgate_apb_stprsts           = KHUBAON_CLK_MGR_REG_HUB_TIMER_CLKGATE_HUB_TIMER_APB_STPRSTS_SHIFT,
   ccu_khubaon_hubtmr_clkgate_voltage_level         = KHUBAON_CLK_MGR_REG_HUB_TIMER_CLKGATE_HUB_TIMER_VOLTAGE_LEVEL_SHIFT

} ccu_khubaon_hubtmr_clkgate_e;

typedef enum
{
   ccu_khubaon_pmubsc_clkgate_clk_en                = KHUBAON_CLK_MGR_REG_PMU_BSC_CLKGATE_PMU_BSC_CLK_EN_SHIFT,
   ccu_khubaon_pmubsc_clkgate_hw_sw_gating_sel      = KHUBAON_CLK_MGR_REG_PMU_BSC_CLKGATE_PMU_BSC_HW_SW_GATING_SEL_SHIFT,
   ccu_khubaon_pmubsc_clkgate_apb_clk_en            = KHUBAON_CLK_MGR_REG_PMU_BSC_CLKGATE_PMU_BSC_APB_CLK_EN_SHIFT,
   ccu_khubaon_pmubsc_clkgate_apb_hw_sw_gating_sel  = KHUBAON_CLK_MGR_REG_PMU_BSC_CLKGATE_PMU_BSC_APB_HW_SW_GATING_SEL_SHIFT,
   ccu_khubaon_pmubsc_clkgate_hyst_val              = KHUBAON_CLK_MGR_REG_PMU_BSC_CLKGATE_PMU_BSC_HYST_VAL_SHIFT,
   ccu_khubaon_pmubsc_clkgate_hyst_en               = KHUBAON_CLK_MGR_REG_PMU_BSC_CLKGATE_PMU_BSC_HYST_EN_SHIFT,
   ccu_khubaon_pmubsc_clkgate_apb_hyst_val          = KHUBAON_CLK_MGR_REG_PMU_BSC_CLKGATE_PMU_BSC_APB_HYST_VAL_SHIFT,
   ccu_khubaon_pmubsc_clkgate_apb_hyst_en           = KHUBAON_CLK_MGR_REG_PMU_BSC_CLKGATE_PMU_BSC_APB_HYST_EN_SHIFT,
   ccu_khubaon_pmubsc_clkgate_stprsts               = KHUBAON_CLK_MGR_REG_PMU_BSC_CLKGATE_PMU_BSC_STPRSTS_SHIFT,
   ccu_khubaon_pmubsc_clkgate_apb_stprsts           = KHUBAON_CLK_MGR_REG_PMU_BSC_CLKGATE_PMU_BSC_APB_STPRSTS_SHIFT,
   ccu_khubaon_pmubsc_clkgate_voltage_level         = KHUBAON_CLK_MGR_REG_PMU_BSC_CLKGATE_PMU_BSC_VOLTAGE_LEVEL_SHIFT

} ccu_khubaon_pmubsc_clkgate_e;

typedef enum
{
   ccu_khubaon_chipreg_clkgate_apb_clk_en           = KHUBAON_CLK_MGR_REG_CHIPREG_CLKGATE_CHIPREG_APB_CLK_EN_SHIFT,
   ccu_khubaon_chipreg_clkgate_apb_hw_sw_gating_sel = KHUBAON_CLK_MGR_REG_CHIPREG_CLKGATE_CHIPREG_APB_HW_SW_GATING_SEL_SHIFT,
   ccu_khubaon_chipreg_clkgate_apb_hyst_val         = KHUBAON_CLK_MGR_REG_CHIPREG_CLKGATE_CHIPREG_APB_HYST_VAL_SHIFT,
   ccu_khubaon_chipreg_clkgate_apb_hyst_en          = KHUBAON_CLK_MGR_REG_CHIPREG_CLKGATE_CHIPREG_APB_HYST_EN_SHIFT,
   ccu_khubaon_chipreg_clkgate_apb_stprsts          = KHUBAON_CLK_MGR_REG_CHIPREG_CLKGATE_CHIPREG_APB_STPRSTS_SHIFT

} ccu_khubaon_chipreg_clkgate_e;

typedef enum
{
   ccu_khubaon_fmon_clkgate_apb_clk_en     = KHUBAON_CLK_MGR_REG_FMON_CLKGATE_FMON_APB_CLK_EN_SHIFT,
   ccu_khubaon_fmon_clkgate_apb_hyst_val   = KHUBAON_CLK_MGR_REG_FMON_CLKGATE_FMON_APB_HYST_VAL_SHIFT,
   ccu_khubaon_fmon_clkgate_apb_hyst_en    = KHUBAON_CLK_MGR_REG_FMON_CLKGATE_FMON_APB_HYST_EN_SHIFT,
   ccu_khubaon_fmon_clkgate_apb_stprsts    = KHUBAON_CLK_MGR_REG_FMON_CLKGATE_FMON_APB_STPRSTS_SHIFT,
   ccu_khubaon_fmon_clkgate_voltage_level  = KHUBAON_CLK_MGR_REG_FMON_CLKGATE_FMON_VOLTAGE_LEVEL_SHIFT

} ccu_khubaon_fmon_clkgate_e;

typedef enum
{
   hal_ccu_khubaon_hubtzcfg_clkgate_apb_clk_en           = KHUBAON_CLK_MGR_REG_HUB_TZCFG_CLKGATE_HUB_TZCFG_APB_CLK_EN_SHIFT,
   hal_ccu_khubaon_hubtzcfg_clkgate_apb_hw_sw_gating_sel = KHUBAON_CLK_MGR_REG_HUB_TZCFG_CLKGATE_HUB_TZCFG_APB_HW_SW_GATING_SEL_SHIFT,
   hal_ccu_khubaon_hubtzcfg_clkgate_apb_hyst_val         = KHUBAON_CLK_MGR_REG_HUB_TZCFG_CLKGATE_HUB_TZCFG_APB_HYST_VAL_SHIFT,
   hal_ccu_khubaon_hubtzcfg_clkgate_apb_hyst_en          = KHUBAON_CLK_MGR_REG_HUB_TZCFG_CLKGATE_HUB_TZCFG_APB_HYST_EN_SHIFT,
   hal_ccu_khubaon_hubtzcfg_clkgate_apb_stprsts          = KHUBAON_CLK_MGR_REG_HUB_TZCFG_CLKGATE_HUB_TZCFG_APB_STPRSTS_SHIFT

} ccu_khubaon_hubtzcfg_clkgate_e;

typedef enum
{
   ccu_khubaon_secwd_clkgate_apb_clk_en    = KHUBAON_CLK_MGR_REG_SEC_WD_CLKGATE_SEC_WD_APB_CLK_EN_SHIFT,
   ccu_khubaon_secwd_clkgate_apb_hyst_val  = KHUBAON_CLK_MGR_REG_SEC_WD_CLKGATE_SEC_WD_APB_HYST_VAL_SHIFT,
   ccu_khubaon_secwd_clkgate_apb_hyst_en   = KHUBAON_CLK_MGR_REG_SEC_WD_CLKGATE_SEC_WD_APB_HYST_EN_SHIFT,
   ccu_khubaon_secwd_clkgate_apb_stprsts   = KHUBAON_CLK_MGR_REG_SEC_WD_CLKGATE_SEC_WD_APB_STPRSTS_SHIFT,
   ccu_khubaon_secwd_clkgate_voltage_level = KHUBAON_CLK_MGR_REG_SEC_WD_CLKGATE_SEC_WD_VOLTAGE_LEVEL_SHIFT

} ccu_khubaon_secwd_clkgate_e;

typedef enum
{
   ccu_khubaon_sysemi_clkgate_open_apb_clk_en             = KHUBAON_CLK_MGR_REG_SYSEMI_CLKGATE_SYSEMI_OPEN_APB_CLK_EN_SHIFT,
   ccu_khubaon_sysemi_clkgate_open_apb_hw_sw_gating_sel   = KHUBAON_CLK_MGR_REG_SYSEMI_CLKGATE_SYSEMI_OPEN_APB_HW_SW_GATING_SEL_SHIFT,
   ccu_khubaon_sysemi_clkgate_sec_apb_clk_en              = KHUBAON_CLK_MGR_REG_SYSEMI_CLKGATE_SYSEMI_SEC_APB_CLK_EN_SHIFT,
   ccu_khubaon_sysemi_clkgate_sec_apb_hw_sw_gating_sel    = KHUBAON_CLK_MGR_REG_SYSEMI_CLKGATE_SYSEMI_SEC_APB_HW_SW_GATING_SEL_SHIFT,
   ccu_khubaon_sysemi_clkgate_open_apb_hyst_val           = KHUBAON_CLK_MGR_REG_SYSEMI_CLKGATE_SYSEMI_OPEN_APB_HYST_VAL_SHIFT,
   ccu_khubaon_sysemi_clkgate_open_apb_hyst_en            = KHUBAON_CLK_MGR_REG_SYSEMI_CLKGATE_SYSEMI_OPEN_APB_HYST_EN_SHIFT,
   ccu_khubaon_sysemi_clkgate_sec_apb_hyst_val            = KHUBAON_CLK_MGR_REG_SYSEMI_CLKGATE_SYSEMI_SEC_APB_HYST_VAL_SHIFT,
   ccu_khubaon_sysemi_clkgate_sec_apb_hyst_en             = KHUBAON_CLK_MGR_REG_SYSEMI_CLKGATE_SYSEMI_SEC_APB_HYST_EN_SHIFT,
   ccu_khubaon_sysemi_clkgate_open_apb_stprsts            = KHUBAON_CLK_MGR_REG_SYSEMI_CLKGATE_SYSEMI_OPEN_APB_STPRSTS_SHIFT,
   ccu_khubaon_sysemi_clkgate_sec_apb_stprsts             = KHUBAON_CLK_MGR_REG_SYSEMI_CLKGATE_SYSEMI_SEC_APB_STPRSTS_SHIFT,
   ccu_khubaon_sysemi_clkgate_voltage_level               = KHUBAON_CLK_MGR_REG_SYSEMI_CLKGATE_SYSEMI_VOLTAGE_LEVEL_SHIFT

} ccu_khubaon_sysemi_clkgate_e;

typedef enum
{
   ccu_khubaon_vcemi_clkgate_open_apb_clk_en           = KHUBAON_CLK_MGR_REG_VCEMI_CLKGATE_VCEMI_OPEN_APB_CLK_EN_SHIFT,
   ccu_khubaon_vcemi_clkgate_open_apb_hw_sw_gating_sel = KHUBAON_CLK_MGR_REG_VCEMI_CLKGATE_VCEMI_OPEN_APB_HW_SW_GATING_SEL_SHIFT,
   ccu_khubaon_vcemi_clkgate_sec_apb_clk_en            = KHUBAON_CLK_MGR_REG_VCEMI_CLKGATE_VCEMI_SEC_APB_CLK_EN_SHIFT,
   ccu_khubaon_vcemi_clkgate_sec_apb_hw_sw_gating_sel  = KHUBAON_CLK_MGR_REG_VCEMI_CLKGATE_VCEMI_SEC_APB_HW_SW_GATING_SEL_SHIFT,
   ccu_khubaon_vcemi_clkgate_open_apb_hyst_val         = KHUBAON_CLK_MGR_REG_VCEMI_CLKGATE_VCEMI_OPEN_APB_HYST_VAL_SHIFT,
   ccu_khubaon_vcemi_clkgate_open_apb_hyst_en          = KHUBAON_CLK_MGR_REG_VCEMI_CLKGATE_VCEMI_OPEN_APB_HYST_EN_SHIFT,
   ccu_khubaon_vcemi_clkgate_sec_apb_hyst_val          = KHUBAON_CLK_MGR_REG_VCEMI_CLKGATE_VCEMI_SEC_APB_HYST_VAL_SHIFT,
   ccu_khubaon_vcemi_clkgate_sec_apb_hyst_en           = KHUBAON_CLK_MGR_REG_VCEMI_CLKGATE_VCEMI_SEC_APB_HYST_EN_SHIFT,
   ccu_khubaon_vcemi_clkgate_open_apb_stprsts          = KHUBAON_CLK_MGR_REG_VCEMI_CLKGATE_VCEMI_OPEN_APB_STPRSTS_SHIFT,
   ccu_khubaon_vcemi_clkgate_sec_apb_stprsts           = KHUBAON_CLK_MGR_REG_VCEMI_CLKGATE_VCEMI_SEC_APB_STPRSTS_SHIFT,
   ccu_khubaon_vcemi_clkgate_voltage_level             = KHUBAON_CLK_MGR_REG_VCEMI_CLKGATE_VCEMI_VOLTAGE_LEVEL_SHIFT

} ccu_khubaon_vcemi_clkgate_e;

typedef enum
{
   ccu_khubaon_aci_clkgate_apb_clk_en            = KHUBAON_CLK_MGR_REG_ACI_CLKGATE_ACI_APB_CLK_EN_SHIFT,
   ccu_khubaon_aci_clkgate_apb_hw_sw_gating_sel  = KHUBAON_CLK_MGR_REG_ACI_CLKGATE_ACI_APB_HW_SW_GATING_SEL_SHIFT,
   ccu_khubaon_aci_clkgate_apb_hyst_val          = KHUBAON_CLK_MGR_REG_ACI_CLKGATE_ACI_APB_HYST_VAL_SHIFT,
   ccu_khubaon_aci_clkgate_apb_hyst_en           = KHUBAON_CLK_MGR_REG_ACI_CLKGATE_ACI_APB_HYST_EN_SHIFT,
   ccu_khubaon_aci_clkgate_apb_stprsts           = KHUBAON_CLK_MGR_REG_ACI_CLKGATE_ACI_APB_STPRSTS_SHIFT,
   ccu_khubaon_aci_clkgate_voltage_level         = KHUBAON_CLK_MGR_REG_ACI_CLKGATE_ACI_VOLTAGE_LEVEL_SHIFT

} ccu_khubaon_aci_clkgate_e;

typedef enum
{
   ccu_khubaon_sim_clkgate_clk_en                = KHUBAON_CLK_MGR_REG_SIM_CLKGATE_SIM_CLK_EN_SHIFT,
   ccu_khubaon_sim_clkgate_hw_sw_gating_sel      = KHUBAON_CLK_MGR_REG_SIM_CLKGATE_SIM_HW_SW_GATING_SEL_SHIFT,
   ccu_khubaon_sim_clkgate_apb_clk_en            = KHUBAON_CLK_MGR_REG_SIM_CLKGATE_SIM_APB_CLK_EN_SHIFT,
   ccu_khubaon_sim_clkgate_apb_hw_sw_gating_sel  = KHUBAON_CLK_MGR_REG_SIM_CLKGATE_SIM_APB_HW_SW_GATING_SEL_SHIFT,
   ccu_khubaon_sim_clkgate_hyst_val              = KHUBAON_CLK_MGR_REG_SIM_CLKGATE_SIM_HYST_VAL_SHIFT,
   ccu_khubaon_sim_clkgate_hyst_en               = KHUBAON_CLK_MGR_REG_SIM_CLKGATE_SIM_HYST_EN_SHIFT,
   ccu_khubaon_sim_clkgate_apb_hyst_val          = KHUBAON_CLK_MGR_REG_SIM_CLKGATE_SIM_APB_HYST_VAL_SHIFT,
   ccu_khubaon_sim_clkgate_apb_hyst_en           = KHUBAON_CLK_MGR_REG_SIM_CLKGATE_SIM_APB_HYST_EN_SHIFT,
   ccu_khubaon_sim_clkgate_stprsts               = KHUBAON_CLK_MGR_REG_SIM_CLKGATE_SIM_STPRSTS_SHIFT,
   ccu_khubaon_sim_clkgate_apb_stprsts           = KHUBAON_CLK_MGR_REG_SIM_CLKGATE_SIM_APB_STPRSTS_SHIFT,
   ccu_khubaon_sim_clkgate_voltage_level         = KHUBAON_CLK_MGR_REG_SIM_CLKGATE_SIM_VOLTAGE_LEVEL_SHIFT

} ccu_khubaon_sim_clkgate_e;

typedef enum
{
   ccu_khubaon_spm_clkgate_apb_clk_en      = KHUBAON_CLK_MGR_REG_SPM_CLKGATE_SPM_APB_CLK_EN_SHIFT,
   ccu_khubaon_spm_clkgate_apb_hyst_val    = KHUBAON_CLK_MGR_REG_SPM_CLKGATE_SPM_APB_HYST_VAL_SHIFT,
   ccu_khubaon_spm_clkgate_apb_hyst_en     = KHUBAON_CLK_MGR_REG_SPM_CLKGATE_SPM_APB_HYST_EN_SHIFT,
   ccu_khubaon_spm_clkgate_apb_stprsts     = KHUBAON_CLK_MGR_REG_SPM_CLKGATE_SPM_APB_STPRSTS_SHIFT,
   ccu_khubaon_spm_clkgate_voltage_level   = KHUBAON_CLK_MGR_REG_SPM_CLKGATE_SPM_VOLTAGE_LEVEL_SHIFT

} ccu_khubaon_spm_clkgate_e;

typedef enum
{
   ccu_khubaon_pscs_clkgate_clk_en           = KHUBAON_CLK_MGR_REG_PSCS_CLKGATE_PSCS_CLK_EN_SHIFT,
   ccu_khubaon_pscs_clkgate_hw_sw_gating_sel = KHUBAON_CLK_MGR_REG_PSCS_CLKGATE_PSCS_HW_SW_GATING_SEL_SHIFT,
   ccu_khubaon_pscs_clkgate_hyst_val         = KHUBAON_CLK_MGR_REG_PSCS_CLKGATE_PSCS_HYST_VAL_SHIFT,
   ccu_khubaon_pscs_clkgate_hyst_en          = KHUBAON_CLK_MGR_REG_PSCS_CLKGATE_PSCS_HYST_EN_SHIFT,
   ccu_khubaon_pscs_clkgate_stprsts          = KHUBAON_CLK_MGR_REG_PSCS_CLKGATE_PSCS_STPRSTS_SHIFT,
   ccu_khubaon_pscs_clkgate_voltage_level    = KHUBAON_CLK_MGR_REG_PSCS_CLKGATE_PSCS_VOLTAGE_LEVEL_SHIFT
	
} ccu_khubaon_pscs_clkgate_e;

typedef enum
{
   ccu_khubaon_async_prediv_pll_select  = KHUBAON_CLK_MGR_REG_ASYNC_PRE_DIV_ASYNC_PRE_PLL_SELECT_SHIFT,
   ccu_khubaon_async_prediv_div         = KHUBAON_CLK_MGR_REG_ASYNC_PRE_DIV_ASYNC_PRE_DIV_SHIFT

} ccu_khubaon_async_prediv_e;

typedef enum
{
   ccu_khubaon_pmubsc_div_pll_select = KHUBAON_CLK_MGR_REG_PMU_BSC_DIV_PMU_BSC_PLL_SELECT_SHIFT,
   ccu_khubaon_pmubsc_div_div        = KHUBAON_CLK_MGR_REG_PMU_BSC_DIV_PMU_BSC_DIV_SHIFT

} ccu_khubaon_pmubsc_div_e;

typedef enum
{
   ccu_khubaon_hubtmr_div_pll_select = KHUBAON_CLK_MGR_REG_HUB_TIMER_DIV_HUB_TIMER_PLL_SELECT_SHIFT

} ccu_khubaon_hubtmr_div_e;

typedef enum
{
   ccu_khubaon_simdiv_pre_pll_select = KHUBAON_CLK_MGR_REG_SIM_DIV_SIM_PRE_PLL_SELECT_SHIFT,
   ccu_khubaon_simdiv_pre_div        = KHUBAON_CLK_MGR_REG_SIM_DIV_SIM_PRE_DIV_SHIFT,
   ccu_khubaon_simdiv_div            = KHUBAON_CLK_MGR_REG_SIM_DIV_SIM_DIV_SHIFT

} ccu_khubaon_simdiv_e;

typedef enum
{
   ccu_khubaon_pscs_div_pll_sel = KHUBAON_CLK_MGR_REG_PSCS_DIV_PSCS_PLL_SELECT_SHIFT,
   ccu_khubaon_pscs_div         = KHUBAON_CLK_MGR_REG_PSCS_DIV_PSCS_DIV_SHIFT
	
} ccu_khubaon_pscs_div_e;

typedef enum
{
   ccu_khubaon_segtrg_pmubsc_trigger    = KHUBAON_CLK_MGR_REG_PERIPH_SEG_TRG_PMU_BSC_TRIGGER_SHIFT,
   ccu_khubaon_segtrg_async_pre_trigger = KHUBAON_CLK_MGR_REG_PERIPH_SEG_TRG_ASYNC_PRE_TRIGGER_SHIFT,
   ccu_khubaon_segtrg_hubtmr_trigger    = KHUBAON_CLK_MGR_REG_PERIPH_SEG_TRG_HUB_TIMER_TRIGGER_SHIFT,
   ccu_khubaon_segtrg_sim_pre_trigger   = KHUBAON_CLK_MGR_REG_PERIPH_SEG_TRG_SIM_PRE_TRIGGER_SHIFT,
   ccu_khubaon_segtrg_sim2_pre_trigger  = KHUBAON_CLK_MGR_REG_PERIPH_SEG_TRG_SIM2_PRE_TRIGGER_SHIFT,
   ccu_khubaon_segtrg_pscs_trigger      = KHUBAON_CLK_MGR_REG_PERIPH_SEG_TRG_PSCS_TRIGGER_SHIFT

} ccu_khubaon_segtrg_e;

typedef enum
{
   ccu_khubaon_hub_div_hubaon_pll_select            = KHUBAON_CLK_MGR_REG_HUB_DIV_HUBAON_PLL_SELECT_SHIFT,
   ccu_khubaon_hub_div_hubaon_pll_select_override   = KHUBAON_CLK_MGR_REG_HUB_DIV_HUBAON_PLL_SELECT_OVERRIDE_SHIFT,
   ccu_khubaon_hub_div_hubaon_div                   = KHUBAON_CLK_MGR_REG_HUB_DIV_HUBAON_DIV_SHIFT,
   ccu_khubaon_hub_div_hubaon_div_override          = KHUBAON_CLK_MGR_REG_HUB_DIV_HUBAON_DIV_OVERRIDE_SHIFT,
   ccu_khubaon_hub_div_apb6_free_div                = KHUBAON_CLK_MGR_REG_HUB_DIV_APB6_FREE_DIV_SHIFT,
   ccu_khubaon_hub_div_apb6_free_div_override       = KHUBAON_CLK_MGR_REG_HUB_DIV_APB6_FREE_DIV_OVERRIDE_SHIFT

} ccu_khubaon_hub_div_e;

typedef enum
{
   ccu_khubaon_hub_segtrg_hubaon_trigger = KHUBAON_CLK_MGR_REG_HUB_SEG_TRG_HUBAON_TRIGGER_SHIFT

} ccu_khubaon_hub_segtrg_e;

typedef enum
{
   ccu_khubaon_hub_segtr_hubaon_trigger_ovrride = KHUBAON_CLK_MGR_REG_HUB_SEG_TRG_OVERRIDE_HUBAON_TRIGGER_OVERRIDE_SHIFT

} ccu_khubaon_hub_segtr_ovrride_e;

typedef enum
{
   ccu_khubaon_clkmon_sel = KHUBAON_CLK_MGR_REG_CLKMON_CLKMON_SEL_SHIFT,
   ccu_khubaon_clkmon_ctl = KHUBAON_CLK_MGR_REG_CLKMON_CLKMON_CTL_SHIFT

} ccu_khubaon_clkmon_e;


/* Function Prototypes */

static inline void      ccu_set_khubaon_peri_volt( ccu_khubaon_peri_volt_e field, uint32_t val );
static inline uint32_t  ccu_get_khubaon_peri_volt( ccu_khubaon_peri_volt_e field );
static inline void      ccu_set_khubaon_lvm( ccu_khubaon_lvm_e field, uint32_t val );
static inline uint32_t  ccu_get_khubaon_lvm( ccu_khubaon_lvm_e field );
static inline void      ccu_set_khubaon_vlt( ccu_khubaon_vlt_e field, uint32_t val );
static inline uint32_t  ccu_get_khubaon_vlt( ccu_khubaon_vlt_e field );
static inline void      ccu_set_khubaon_hub_clkgate( ccu_khubaon_hub_clkgate_e field, uint32_t val );
static inline uint32_t  ccu_get_khubaon_hub_clkgate( ccu_khubaon_hub_clkgate_e field );
static inline void      ccu_set_khubaon_pwrmgr_clkgate( ccu_khubaon_pwrmgr_clkgate_e field, uint32_t val );
static inline uint32_t  ccu_get_khubaon_pwrmgr_clkgate( ccu_khubaon_pwrmgr_clkgate_e field );
static inline void      ccu_set_khubaon_apb6_clkgate( ccu_khubaon_apb6_clkgate_e field, uint32_t val );
static inline uint32_t  ccu_get_khubaon_apb6_clkgate( ccu_khubaon_apb6_clkgate_e field );
static inline void      ccu_set_khubaon_gpiokp_clkgate( ccu_khubaon_gpiokp_clkgate_e field, uint32_t val );
static inline uint32_t  ccu_get_khubaon_gpiokp_clkgate( ccu_khubaon_gpiokp_clkgate_e field );
static inline void      ccu_set_khubaon_hubtmr_clkgate( ccu_khubaon_hubtmr_clkgate_e field, uint32_t val );
static inline uint32_t  ccu_get_khubaon_hubtmr_clkgate( ccu_khubaon_hubtmr_clkgate_e field );
static inline void      ccu_set_khubaon_pmubsc_clkgate( ccu_khubaon_pmubsc_clkgate_e field, uint32_t val );
static inline uint32_t  ccu_get_khubaon_pmubsc_clkgate( ccu_khubaon_pmubsc_clkgate_e field );
static inline void      ccu_set_khubaon_chipreg_clkgate( ccu_khubaon_chipreg_clkgate_e field, uint32_t val );
static inline uint32_t  ccu_get_khubaon_chipreg_clkgate( ccu_khubaon_chipreg_clkgate_e field );
static inline void      ccu_set_khubaon_fmon_clkgate( ccu_khubaon_fmon_clkgate_e field, uint32_t val );
static inline uint32_t  ccu_get_khubaon_fmon_clkgate( ccu_khubaon_fmon_clkgate_e field );
static inline void      ccu_set_khubaon_hubtzcfg_clkgate( ccu_khubaon_hubtzcfg_clkgate_e field, uint32_t val );
static inline uint32_t  ccu_get_khubaon_hubtzcfg_clkgate( ccu_khubaon_hubtzcfg_clkgate_e field );
static inline void      ccu_set_khubaon_secwd_clkgate( ccu_khubaon_secwd_clkgate_e field, uint32_t val );
static inline uint32_t  ccu_get_khubaon_secwd_clkgate( ccu_khubaon_secwd_clkgate_e field );
static inline void      ccu_set_khubaon_sysemi_clkgate( ccu_khubaon_sysemi_clkgate_e field, uint32_t val );
static inline uint32_t  ccu_get_khubaon_sysemi_clkgate( ccu_khubaon_sysemi_clkgate_e field );
static inline void      ccu_set_khubaon_vcemi_clkgate( ccu_khubaon_vcemi_clkgate_e field, uint32_t val );
static inline uint32_t  ccu_get_khubaon_vcemi_clkgate( ccu_khubaon_vcemi_clkgate_e field );
static inline void      ccu_set_khubaon_aci_clkgate( ccu_khubaon_aci_clkgate_e field, uint32_t val );
static inline uint32_t  ccu_get_khubaon_aci_clkgate( ccu_khubaon_aci_clkgate_e field );
static inline void      ccu_set_khubaon_sim_clkgate( ccu_khubaon_sim_clkgate_e field, uint32_t val );
static inline uint32_t  ccu_get_khubaon_sim_clkgate( ccu_khubaon_sim_clkgate_e field );
static inline void      ccu_set_khubaon_spm_clkgate( ccu_khubaon_spm_clkgate_e field, uint32_t val );
static inline uint32_t  ccu_get_khubaon_spm_clkgate( ccu_khubaon_spm_clkgate_e field );
static inline void      ccu_set_khubaon_sim2_clkgate( ccu_khubaon_sim_clkgate_e field, uint32_t val );
static inline uint32_t  ccu_get_khubaon_sim2_clkgate( ccu_khubaon_sim_clkgate_e field );
static inline void      ccu_set_khubaon_pscs_clkgate( ccu_khubaon_pscs_clkgate_e field, uint32_t val );
static inline uint32_t  ccu_get_khubaon_pscs_clkgate( ccu_khubaon_pscs_clkgate_e field );
static inline void      ccu_set_khubaon_async_prediv( ccu_khubaon_async_prediv_e field, uint32_t val );
static inline uint32_t  ccu_get_khubaon_async_prediv( ccu_khubaon_async_prediv_e field );
static inline void      ccu_set_khubaon_pmubsc_div( ccu_khubaon_pmubsc_div_e field, uint32_t val );
static inline uint32_t  ccu_get_khubaon_pmubsc_div( ccu_khubaon_pmubsc_div_e field );
static inline void      ccu_set_khubaon_hubtmr_div( ccu_khubaon_hubtmr_div_e field, uint32_t val );
static inline uint32_t  ccu_get_khubaon_hubtmr_div( ccu_khubaon_hubtmr_div_e field );
static inline void      ccu_set_khubaon_sim_div( ccu_khubaon_simdiv_e field, uint32_t val );
static inline uint32_t  ccu_get_khubaon_sim_div( ccu_khubaon_simdiv_e field );
static inline void      ccu_set_khubaon_sim2_div( ccu_khubaon_simdiv_e field, uint32_t val );
static inline uint32_t  ccu_get_khubaon_sim2_div( ccu_khubaon_simdiv_e field );
static inline void      ccu_set_khubaon_pscs_div( ccu_khubaon_pscs_div_e field, uint32_t val );
static inline uint32_t  ccu_get_khubaon_pscs_div( ccu_khubaon_pscs_div_e field );
static inline void      ccu_set_khubaon_seg_trigger( ccu_khubaon_segtrg_e field, uint32_t val );
static inline uint32_t  ccu_get_khubaon_seg_trigger( ccu_khubaon_segtrg_e field );
static inline void      ccu_set_khubaon_hub_div( ccu_khubaon_hub_div_e field, uint32_t val );
static inline uint32_t  ccu_get_khubaon_hub_div( ccu_khubaon_hub_div_e field );
static inline void      ccu_set_khubaon_hubseg_trigger( ccu_khubaon_hub_segtrg_e field, uint32_t val );
static inline uint32_t  ccu_get_khubaon_hubseg_trigger( ccu_khubaon_hub_segtrg_e field );
static inline void      ccu_set_khubaon_hubseg_override_trigger( ccu_khubaon_hub_segtr_ovrride_e field, uint32_t val );
static inline uint32_t  ccu_get_khubaon_hubseg_override_trigger( ccu_khubaon_hub_segtr_ovrride_e field );
static inline void      ccu_set_khubaon_clkmon( ccu_khubaon_clkmon_e field, uint32_t val );
static inline uint32_t  ccu_get_khubaon_clkmon( ccu_khubaon_clkmon_e field );







/* Local macros */
#define ccu_set_khubaon_bit(offset, field, val) \
            ccu_set_bit( (MM_IO_BASE_AON_CLK + (offset)), (field), (val) )

#define ccu_get_khubaon_bit(offset, field)      \
            ccu_get_bit( (MM_IO_BASE_AON_CLK + (offset)), (field) )

#define ccu_set_khubaon_reg_field(offset, mask, shift, val) \
            ccu_set_reg_field( (MM_IO_BASE_AON_CLK + (offset)), (mask), (shift), (val) );

#define ccu_get_khubaon_reg_field(offset, mask, shift)   \
            ccu_get_reg_field( (MM_IO_BASE_AON_CLK + (offset)), (mask), (shift) )


/* Functions */

static inline void ccu_set_khubaon_peri_volt( ccu_khubaon_peri_volt_e field, uint32_t val )
{
   uint32_t mask;

   if (field == ccu_khubaon_peri_volt_normal)
   {
      mask = KHUBAON_CLK_MGR_REG_VLT_PERI_VLT_NORMAL_PERI_MASK;
   }
   else if (field == ccu_khubaon_peri_volt_high)
   {
      mask = KHUBAON_CLK_MGR_REG_VLT_PERI_VLT_HIGH_PERI_MASK;
   }
   else
   {
      mask = 1 << field;
   }
   ccu_set_khubaon_reg_field( KHUBAON_CLK_MGR_REG_VLT_PERI_OFFSET, mask, field, val );
}

static inline uint32_t ccu_get_khubaon_peri_volt( ccu_khubaon_peri_volt_e field )
{
   uint32_t mask;

   if (field == ccu_khubaon_peri_volt_normal)
   {
      mask = KHUBAON_CLK_MGR_REG_VLT_PERI_VLT_NORMAL_PERI_MASK;
   }
   else if (field == ccu_khubaon_peri_volt_high)
   {
      mask = KHUBAON_CLK_MGR_REG_VLT_PERI_VLT_HIGH_PERI_MASK;
   }
   else
   {
      mask = 1 << field;
   }
   return ccu_get_khubaon_reg_field( KHUBAON_CLK_MGR_REG_VLT_PERI_OFFSET, mask, field );
}

static inline void ccu_set_khubaon_lvm( ccu_khubaon_lvm_e field, uint32_t val )
{
   uint32_t shift;
   uint32_t mask;
   uint32_t reg_offset;

   if (field < ccu_khubaon_lvm_4 )
   {
      reg_offset = KHUBAON_CLK_MGR_REG_LVM0_3_OFFSET;
   }
   else
   {
      reg_offset = KHUBAON_CLK_MGR_REG_LVM4_7_OFFSET;
      field = field - ccu_khubaon_lvm_4;
   }

   shift = field * 4;
   mask  = KHUBAON_CLK_MGR_REG_LVM0_3_LVM0_3_MD_00_MASK << (shift);

   ccu_set_khubaon_reg_field( reg_offset, mask, shift, val );
}

static inline uint32_t ccu_get_khubaon_lvm( ccu_khubaon_lvm_e field )
{
   uint32_t shift;
   uint32_t mask;
   uint32_t reg_offset;

   if (field < ccu_khubaon_lvm_4 )
   {
      reg_offset = KHUBAON_CLK_MGR_REG_LVM0_3_OFFSET;
   }
   else
   {
      reg_offset = KHUBAON_CLK_MGR_REG_LVM4_7_OFFSET;
      field = field - ccu_khubaon_lvm_4;
   }

   shift = field * 4;
   mask  = KHUBAON_CLK_MGR_REG_LVM0_3_LVM0_3_MD_00_MASK << (shift);

   return ccu_get_khubaon_reg_field( reg_offset, mask, shift );
}

static inline void ccu_set_khubaon_vlt( ccu_khubaon_vlt_e field, uint32_t val )
{
   uint32_t shift;
   uint32_t mask;
   uint32_t reg_offset;

   if (field < ccu_khubaon_vlt_4 )
   {
      reg_offset = KHUBAON_CLK_MGR_REG_VLT0_3_OFFSET;
   }
   else
   {
      reg_offset = KHUBAON_CLK_MGR_REG_VLT4_7_OFFSET;
      field = field - ccu_khubaon_vlt_4;
   }

   shift = field * 8;
   mask  = KHUBAON_CLK_MGR_REG_VLT0_3_VLT0_3_VV_00_MASK << (shift);

   ccu_set_khubaon_reg_field( reg_offset, mask, shift, val );
}

static inline uint32_t ccu_get_khubaon_vlt( ccu_khubaon_vlt_e field )
{
   uint32_t shift;
   uint32_t mask;
   uint32_t reg_offset;

   if (field < ccu_khubaon_vlt_4 )
   {
      reg_offset = KHUBAON_CLK_MGR_REG_VLT0_3_OFFSET;
   }
   else
   {
      reg_offset = KHUBAON_CLK_MGR_REG_VLT4_7_OFFSET;
      field = field - ccu_khubaon_vlt_4;
   }

   shift = field * 8;
   mask  = KHUBAON_CLK_MGR_REG_VLT0_3_VLT0_3_VV_00_MASK << (shift);

   return ccu_get_khubaon_reg_field( reg_offset, mask, shift );
}

static inline void ccu_set_khubaon_hub_clkgate( ccu_khubaon_hub_clkgate_e field, uint32_t val )
{
   ccu_set_khubaon_bit( KHUBAON_CLK_MGR_REG_HUB_CLKGATE_OFFSET, field, val );
}

static inline uint32_t ccu_get_khubaon_hub_clkgate( ccu_khubaon_hub_clkgate_e field )
{
   return ccu_get_khubaon_bit( KHUBAON_CLK_MGR_REG_HUB_CLKGATE_OFFSET, field );
}

static inline void ccu_set_khubaon_pwrmgr_clkgate( ccu_khubaon_pwrmgr_clkgate_e field, uint32_t val )
{
   ccu_set_khubaon_bit( KHUBAON_CLK_MGR_REG_PWRMGR_CLKGATE_OFFSET, field, val );
}

static inline uint32_t ccu_get_khubaon_pwrmgr_clkgate( ccu_khubaon_pwrmgr_clkgate_e field )
{
   return ccu_get_khubaon_bit( KHUBAON_CLK_MGR_REG_PWRMGR_CLKGATE_OFFSET, field );
}

static inline void ccu_set_khubaon_apb6_clkgate( ccu_khubaon_apb6_clkgate_e field, uint32_t val )
{
   ccu_set_khubaon_bit( KHUBAON_CLK_MGR_REG_APB6_CLKGATE_OFFSET, field, val );
}

static inline uint32_t ccu_get_khubaon_apb6_clkgate( ccu_khubaon_apb6_clkgate_e field )
{
   return ccu_get_khubaon_bit( KHUBAON_CLK_MGR_REG_APB6_CLKGATE_OFFSET, field );
}

static inline void ccu_set_khubaon_gpiokp_clkgate( ccu_khubaon_gpiokp_clkgate_e field, uint32_t val )
{
   ccu_set_khubaon_bit( KHUBAON_CLK_MGR_REG_GPIOKP_CLKGATE_OFFSET, field, val );
}

static inline uint32_t ccu_get_khubaon_gpiokp_clkgate( ccu_khubaon_gpiokp_clkgate_e field )
{
   return ccu_get_khubaon_bit( KHUBAON_CLK_MGR_REG_GPIOKP_CLKGATE_OFFSET, field );
}

static inline void ccu_set_khubaon_hubtmr_clkgate( ccu_khubaon_hubtmr_clkgate_e field, uint32_t val )
{
   ccu_set_khubaon_bit( KHUBAON_CLK_MGR_REG_HUB_TIMER_CLKGATE_OFFSET, field, val );
}

static inline uint32_t ccu_get_khubaon_hubtmr_clkgate( ccu_khubaon_hubtmr_clkgate_e field )
{
   return ccu_get_khubaon_bit( KHUBAON_CLK_MGR_REG_HUB_TIMER_CLKGATE_OFFSET, field );
}

static inline void ccu_set_khubaon_pmubsc_clkgate( ccu_khubaon_pmubsc_clkgate_e field, uint32_t val )
{
   ccu_set_khubaon_bit( KHUBAON_CLK_MGR_REG_PMU_BSC_CLKGATE_OFFSET, field, val );
}

static inline uint32_t ccu_get_khubaon_pmubsc_clkgate( ccu_khubaon_pmubsc_clkgate_e field )
{
   return ccu_get_khubaon_bit( KHUBAON_CLK_MGR_REG_PMU_BSC_CLKGATE_OFFSET, field );
}

static inline void ccu_set_khubaon_chipreg_clkgate( ccu_khubaon_chipreg_clkgate_e field, uint32_t val )
{
   ccu_set_khubaon_bit( KHUBAON_CLK_MGR_REG_CHIPREG_CLKGATE_OFFSET, field, val );
}

static inline uint32_t ccu_get_khubaon_chipreg_clkgate( ccu_khubaon_chipreg_clkgate_e field )
{
   return ccu_get_khubaon_bit( KHUBAON_CLK_MGR_REG_CHIPREG_CLKGATE_OFFSET, field );
}

static inline void ccu_set_khubaon_fmon_clkgate( ccu_khubaon_fmon_clkgate_e field, uint32_t val )
{
   ccu_set_khubaon_bit( KHUBAON_CLK_MGR_REG_FMON_CLKGATE_OFFSET, field, val );
}

static inline uint32_t ccu_get_khubaon_fmon_clkgate( ccu_khubaon_fmon_clkgate_e field )
{
   return ccu_get_khubaon_bit( KHUBAON_CLK_MGR_REG_FMON_CLKGATE_OFFSET, field );
}

static inline void ccu_set_khubaon_hubtzcfg_clkgate( ccu_khubaon_hubtzcfg_clkgate_e field, uint32_t val )
{
   ccu_set_khubaon_bit( KHUBAON_CLK_MGR_REG_HUB_TZCFG_CLKGATE_OFFSET, field, val );
}

static inline uint32_t ccu_get_khubaon_hubtzcfg_clkgate( ccu_khubaon_hubtzcfg_clkgate_e field )
{
   return ccu_get_khubaon_bit( KHUBAON_CLK_MGR_REG_HUB_TZCFG_CLKGATE_OFFSET, field );
}

static inline void ccu_set_khubaon_secwd_clkgate( ccu_khubaon_secwd_clkgate_e field, uint32_t val )
{
   ccu_set_khubaon_bit( KHUBAON_CLK_MGR_REG_SEC_WD_CLKGATE_OFFSET, field, val );
}

static inline uint32_t ccu_get_khubaon_secwd_clkgate( ccu_khubaon_secwd_clkgate_e field )
{
   return ccu_get_khubaon_bit( KHUBAON_CLK_MGR_REG_SEC_WD_CLKGATE_OFFSET, field );
}

static inline void ccu_set_khubaon_sysemi_clkgate( ccu_khubaon_sysemi_clkgate_e field, uint32_t val )
{
   ccu_set_khubaon_bit( KHUBAON_CLK_MGR_REG_SYSEMI_CLKGATE_OFFSET, field, val );
}

static inline uint32_t ccu_get_khubaon_sysemi_clkgate( ccu_khubaon_sysemi_clkgate_e field )
{
   return ccu_get_khubaon_bit( KHUBAON_CLK_MGR_REG_SYSEMI_CLKGATE_OFFSET, field );
}

static inline void ccu_set_khubaon_vcemi_clkgate( ccu_khubaon_vcemi_clkgate_e field, uint32_t val )
{
   ccu_set_khubaon_bit( KHUBAON_CLK_MGR_REG_VCEMI_CLKGATE_OFFSET, field, val );
}

static inline uint32_t ccu_get_khubaon_vcemi_clkgate( ccu_khubaon_vcemi_clkgate_e field )
{
   return ccu_get_khubaon_bit( KHUBAON_CLK_MGR_REG_VCEMI_CLKGATE_OFFSET, field );
}

static inline void ccu_set_khubaon_aci_clkgate( ccu_khubaon_aci_clkgate_e field, uint32_t val )
{
   ccu_set_khubaon_bit( KHUBAON_CLK_MGR_REG_ACI_CLKGATE_OFFSET, field, val );
}

static inline uint32_t ccu_get_khubaon_aci_clkgate( ccu_khubaon_aci_clkgate_e field )
{
   return ccu_get_khubaon_bit( KHUBAON_CLK_MGR_REG_ACI_CLKGATE_OFFSET, field );
}

static inline void ccu_set_khubaon_sim_clkgate( ccu_khubaon_sim_clkgate_e field, uint32_t val )
{
   ccu_set_khubaon_bit( KHUBAON_CLK_MGR_REG_SIM_CLKGATE_OFFSET, field, val );
}

static inline uint32_t ccu_get_khubaon_sim_clkgate( ccu_khubaon_sim_clkgate_e field )
{
   return ccu_get_khubaon_bit( KHUBAON_CLK_MGR_REG_SIM_CLKGATE_OFFSET, field );
}

static inline void ccu_set_khubaon_spm_clkgate( ccu_khubaon_spm_clkgate_e field, uint32_t val )
{
   ccu_set_khubaon_bit( KHUBAON_CLK_MGR_REG_SPM_CLKGATE_OFFSET, field, val );
}

static inline uint32_t ccu_get_khubaon_spm_clkgate( ccu_khubaon_spm_clkgate_e field )
{
   return ccu_get_khubaon_bit( KHUBAON_CLK_MGR_REG_SPM_CLKGATE_OFFSET, field );
}

static inline void ccu_set_khubaon_sim2_clkgate( ccu_khubaon_sim_clkgate_e field, uint32_t val )
{
   ccu_set_khubaon_bit( KHUBAON_CLK_MGR_REG_SIM2_CLKGATE_OFFSET, field, val );
}

static inline uint32_t ccu_get_khubaon_sim2_clkgate( ccu_khubaon_sim_clkgate_e field )
{
   return ccu_get_khubaon_bit( KHUBAON_CLK_MGR_REG_SIM2_CLKGATE_OFFSET, field );
}

static inline void ccu_set_khubaon_pscs_clkgate( ccu_khubaon_pscs_clkgate_e field, uint32_t val )
{
   ccu_set_khubaon_bit( KHUBAON_CLK_MGR_REG_PSCS_CLKGATE_OFFSET, field, val );
}

static inline uint32_t ccu_get_khubaon_pscs_clkgate( ccu_khubaon_pscs_clkgate_e field )
{
   return ccu_get_khubaon_bit( KHUBAON_CLK_MGR_REG_PSCS_CLKGATE_OFFSET, field );
}

static inline void ccu_set_khubaon_async_prediv( ccu_khubaon_async_prediv_e field, uint32_t val )
{
   uint32_t mask;

   if (field == ccu_khubaon_async_prediv_pll_select)
   {
      mask = KHUBAON_CLK_MGR_REG_ASYNC_PRE_DIV_ASYNC_PRE_PLL_SELECT_MASK;
   }
   else if (field == ccu_khubaon_async_prediv_div)
   {
      mask = KHUBAON_CLK_MGR_REG_ASYNC_PRE_DIV_ASYNC_PRE_DIV_MASK;
   }
   else
   {
      mask = 1 << field;
   }
   ccu_set_khubaon_reg_field( KHUBAON_CLK_MGR_REG_ASYNC_PRE_DIV_OFFSET, mask, field, val );
}

static inline uint32_t ccu_get_khubaon_async_prediv( ccu_khubaon_async_prediv_e field )
{
   uint32_t mask;

   if (field == ccu_khubaon_async_prediv_pll_select)
   {
      mask = KHUBAON_CLK_MGR_REG_ASYNC_PRE_DIV_ASYNC_PRE_PLL_SELECT_MASK;
   }
   else if (field == ccu_khubaon_async_prediv_div)
   {
      mask = KHUBAON_CLK_MGR_REG_ASYNC_PRE_DIV_ASYNC_PRE_DIV_MASK;
   }
   else
   {
      mask = 1 << field;
   }
   return ccu_get_khubaon_reg_field( KHUBAON_CLK_MGR_REG_ASYNC_PRE_DIV_OFFSET, mask, field );
}

static inline void ccu_set_khubaon_pmubsc_div( ccu_khubaon_pmubsc_div_e field, uint32_t val )
{
   uint32_t mask;

   if (field == ccu_khubaon_pmubsc_div_pll_select)
   {
      mask = KHUBAON_CLK_MGR_REG_PMU_BSC_DIV_PMU_BSC_PLL_SELECT_MASK;
   }
   else if (field == ccu_khubaon_pmubsc_div_div)
   {
      mask = KHUBAON_CLK_MGR_REG_PMU_BSC_DIV_PMU_BSC_DIV_MASK;
   }
   else
   {
      mask = 1 << field;
   }
   ccu_set_khubaon_reg_field( KHUBAON_CLK_MGR_REG_PMU_BSC_DIV_OFFSET, mask, field, val );
}

static inline uint32_t ccu_get_khubaon_pmubsc_div( ccu_khubaon_pmubsc_div_e field )
{
   uint32_t mask;

   if (field == ccu_khubaon_pmubsc_div_pll_select)
   {
      mask = KHUBAON_CLK_MGR_REG_PMU_BSC_DIV_PMU_BSC_PLL_SELECT_MASK;
   }
   else if (field == ccu_khubaon_pmubsc_div_div)
   {
      mask = KHUBAON_CLK_MGR_REG_PMU_BSC_DIV_PMU_BSC_DIV_MASK;
   }
   else
   {
      mask = 1 << field;
   }
   return ccu_get_khubaon_reg_field( KHUBAON_CLK_MGR_REG_PMU_BSC_DIV_OFFSET, mask, field );
}

static inline void ccu_set_khubaon_hubtmr_div( ccu_khubaon_hubtmr_div_e field, uint32_t val )
{
   uint32_t mask;

   if (field == ccu_khubaon_hubtmr_div_pll_select)
   {
      mask = KHUBAON_CLK_MGR_REG_HUB_TIMER_DIV_HUB_TIMER_PLL_SELECT_MASK;
   }
   else
   {
      mask = 1 << field;
   }
   ccu_set_khubaon_reg_field( KHUBAON_CLK_MGR_REG_HUB_TIMER_DIV_OFFSET, mask, field, val );
}

static inline uint32_t ccu_get_khubaon_hubtmr_div( ccu_khubaon_hubtmr_div_e field )
{
   uint32_t mask;

   if (field == ccu_khubaon_hubtmr_div_pll_select)
   {
      mask = KHUBAON_CLK_MGR_REG_HUB_TIMER_DIV_HUB_TIMER_PLL_SELECT_MASK;
   }
   else
   {
      mask = 1 << field;
   }
   return ccu_get_khubaon_reg_field( KHUBAON_CLK_MGR_REG_HUB_TIMER_DIV_OFFSET, mask, field );
}

static inline void ccu_set_khubaon_sim_div( ccu_khubaon_simdiv_e field, uint32_t val )
{
   uint32_t mask;

   if (field == ccu_khubaon_simdiv_pre_pll_select)
   {
      mask = KHUBAON_CLK_MGR_REG_SIM_DIV_SIM_PRE_PLL_SELECT_MASK;
   }
   else if (field == ccu_khubaon_simdiv_pre_div)
   {
      mask = KHUBAON_CLK_MGR_REG_SIM_DIV_SIM_PRE_DIV_MASK;
   }
   else if (field == ccu_khubaon_simdiv_div)
   {
      mask = KHUBAON_CLK_MGR_REG_SIM_DIV_SIM_DIV_MASK;
   }
   else
   {
      mask = 1 << field;
   }
   ccu_set_khubaon_reg_field( KHUBAON_CLK_MGR_REG_SIM_DIV_OFFSET, mask, field, val );
}

static inline uint32_t ccu_get_khubaon_sim_div( ccu_khubaon_simdiv_e field )
{
   uint32_t mask;

   if (field == ccu_khubaon_simdiv_pre_pll_select)
   {
      mask = KHUBAON_CLK_MGR_REG_SIM_DIV_SIM_PRE_PLL_SELECT_MASK;
   }
   else if (field == ccu_khubaon_simdiv_pre_div)
   {
      mask = KHUBAON_CLK_MGR_REG_SIM_DIV_SIM_PRE_DIV_MASK;
   }
   else if (field == ccu_khubaon_simdiv_div)
   {
      mask = KHUBAON_CLK_MGR_REG_SIM_DIV_SIM_DIV_MASK;
   }
   else
   {
      mask = 1 << field;
   }
   return ccu_get_khubaon_reg_field( KHUBAON_CLK_MGR_REG_SIM_DIV_OFFSET, mask, field );
}

static inline void ccu_set_khubaon_sim2_div( ccu_khubaon_simdiv_e field, uint32_t val )
{
   uint32_t mask;

   if (field == ccu_khubaon_simdiv_pre_pll_select)
   {
      mask = KHUBAON_CLK_MGR_REG_SIM_DIV_SIM_PRE_PLL_SELECT_MASK;
   }
   else if (field == ccu_khubaon_simdiv_pre_div)
   {
      mask = KHUBAON_CLK_MGR_REG_SIM_DIV_SIM_PRE_DIV_MASK;
   }
   else if (field == ccu_khubaon_simdiv_div)
   {
      mask = KHUBAON_CLK_MGR_REG_SIM_DIV_SIM_DIV_MASK;
   }
   else
   {
      mask = 1 << field;
   }
   ccu_set_khubaon_reg_field( KHUBAON_CLK_MGR_REG_SIM2_DIV_OFFSET, mask, field, val );
}

static inline uint32_t ccu_get_khubaon_sim2_div( ccu_khubaon_simdiv_e field )
{
   uint32_t mask;

   if (field == ccu_khubaon_simdiv_pre_pll_select)
   {
      mask = KHUBAON_CLK_MGR_REG_SIM_DIV_SIM_PRE_PLL_SELECT_MASK;
   }
   else if (field == ccu_khubaon_simdiv_pre_div)
   {
      mask = KHUBAON_CLK_MGR_REG_SIM_DIV_SIM_PRE_DIV_MASK;
   }
   else if (field == ccu_khubaon_simdiv_div)
   {
      mask = KHUBAON_CLK_MGR_REG_SIM_DIV_SIM_DIV_MASK;
   }
   else
   {
      mask = 1 << field;
   }
   return ccu_get_khubaon_reg_field( KHUBAON_CLK_MGR_REG_SIM2_DIV_OFFSET, mask, field );
}

static inline void ccu_set_khubaon_pscs_div( ccu_khubaon_pscs_div_e field, uint32_t val )
{
   uint32_t mask;

   if (field == ccu_khubaon_pscs_div_pll_sel)
   {
      mask = KHUBAON_CLK_MGR_REG_PSCS_DIV_PSCS_PLL_SELECT_MASK;
   }
   else if (field == ccu_khubaon_pscs_div)
   {
      mask = KHUBAON_CLK_MGR_REG_PSCS_DIV_PSCS_DIV_MASK;
   }
   else
   {
      mask = 1 << field;
   }
   ccu_set_khubaon_reg_field( KHUBAON_CLK_MGR_REG_PSCS_DIV_OFFSET, mask, field, val );
}

static inline uint32_t ccu_get_khubaon_pscs_div( ccu_khubaon_pscs_div_e field )
{
   uint32_t mask;

   if (field == ccu_khubaon_pscs_div_pll_sel)
   {
      mask = KHUBAON_CLK_MGR_REG_PSCS_DIV_PSCS_PLL_SELECT_MASK;
   }
   else if (field == ccu_khubaon_pscs_div)
   {
      mask = KHUBAON_CLK_MGR_REG_PSCS_DIV_PSCS_DIV_MASK;
   }
   else
   {
      mask = 1 << field;
   }
   return ccu_get_khubaon_reg_field( KHUBAON_CLK_MGR_REG_PSCS_DIV_OFFSET, mask, field );
}

static inline void ccu_set_khubaon_seg_trigger( ccu_khubaon_segtrg_e field, uint32_t val )
{
   ccu_set_khubaon_bit( KHUBAON_CLK_MGR_REG_PERIPH_SEG_TRG_OFFSET, field, val );
}

static inline uint32_t ccu_get_khubaon_seg_trigger( ccu_khubaon_segtrg_e field )
{
   return ccu_get_khubaon_bit( KHUBAON_CLK_MGR_REG_PERIPH_SEG_TRG_OFFSET, field );
}

static inline void ccu_set_khubaon_hub_div( ccu_khubaon_hub_div_e field, uint32_t val )
{
   uint32_t mask;

   if (field == ccu_khubaon_hub_div_hubaon_pll_select)
   {
      mask = KHUBAON_CLK_MGR_REG_HUB_DIV_HUBAON_PLL_SELECT_MASK;
   }
   else if (field == ccu_khubaon_hub_div_hubaon_div)
   {
      mask = KHUBAON_CLK_MGR_REG_HUB_DIV_HUBAON_DIV_MASK;
   }
   else if (field == ccu_khubaon_hub_div_apb6_free_div)
   {
      mask = KHUBAON_CLK_MGR_REG_HUB_DIV_APB6_FREE_DIV_MASK;
   }
   else
   {
      mask = 1 << field;
   }
   ccu_set_khubaon_reg_field( KHUBAON_CLK_MGR_REG_HUB_DIV_OFFSET, mask, field, val );
}

static inline uint32_t ccu_get_khubaon_hub_div( ccu_khubaon_hub_div_e field )
{
   uint32_t mask;

   if (field == ccu_khubaon_hub_div_hubaon_pll_select)
   {
      mask = KHUBAON_CLK_MGR_REG_HUB_DIV_HUBAON_PLL_SELECT_MASK;
   }
   else if (field == ccu_khubaon_hub_div_hubaon_div)
   {
      mask = KHUBAON_CLK_MGR_REG_HUB_DIV_HUBAON_DIV_MASK;
   }
   else if (field == ccu_khubaon_hub_div_apb6_free_div)
   {
      mask = KHUBAON_CLK_MGR_REG_HUB_DIV_APB6_FREE_DIV_MASK;
   }
   else
   {
      mask = 1 << field;
   }
   return ccu_get_khubaon_reg_field( KHUBAON_CLK_MGR_REG_HUB_DIV_OFFSET, mask, field );
}

static inline void ccu_set_khubaon_hubseg_trigger( ccu_khubaon_hub_segtrg_e field, uint32_t val )
{
   ccu_set_khubaon_bit( KHUBAON_CLK_MGR_REG_HUB_SEG_TRG_OFFSET, field, val );
}

static inline uint32_t ccu_get_khubaon_hubseg_trigger( ccu_khubaon_hub_segtrg_e field )
{
   return ccu_get_khubaon_bit( KHUBAON_CLK_MGR_REG_HUB_SEG_TRG_OFFSET, field );
}

static inline void ccu_set_khubaon_hubseg_override_trigger( ccu_khubaon_hub_segtr_ovrride_e field, uint32_t val )
{
   ccu_set_khubaon_bit( KHUBAON_CLK_MGR_REG_HUB_SEG_TRG_OVERRIDE_OFFSET, field, val );
}

static inline uint32_t ccu_get_khubaon_hubseg_override_trigger( ccu_khubaon_hub_segtr_ovrride_e field )
{
   return ccu_get_khubaon_bit( KHUBAON_CLK_MGR_REG_HUB_SEG_TRG_OVERRIDE_OFFSET, field );
}

static inline void ccu_set_khubaon_clkmon( ccu_khubaon_clkmon_e field, uint32_t val )
{
   uint32_t mask;

   if (field == ccu_khubaon_clkmon_sel)
   {
      mask = KHUBAON_CLK_MGR_REG_CLKMON_CLKMON_SEL_MASK;
   }
   else if (field == ccu_khubaon_clkmon_ctl)
   {
      mask = KHUBAON_CLK_MGR_REG_CLKMON_CLKMON_CTL_MASK;
   }
   else
   {
      mask = 1 << field;
   }
   ccu_set_khubaon_reg_field( KHUBAON_CLK_MGR_REG_CLKMON_OFFSET, mask, field, val );
}

static inline uint32_t ccu_get_khubaon_clkmon( ccu_khubaon_clkmon_e field )
{
   uint32_t mask;

   if (field == ccu_khubaon_clkmon_sel)
   {
      mask = KHUBAON_CLK_MGR_REG_CLKMON_CLKMON_SEL_MASK;
   }
   else if (field == ccu_khubaon_clkmon_ctl)
   {
      mask = KHUBAON_CLK_MGR_REG_CLKMON_CLKMON_CTL_MASK;
   }
   else
   {
      mask = 1 << field;
   }
   return ccu_get_khubaon_reg_field( KHUBAON_CLK_MGR_REG_CLKMON_OFFSET, mask, field );
}


#ifdef __cplusplus
}
#endif

#endif /* _CCU_KHUBAON_INLINE_H_*/

