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
* @file  ccu_kps_inline.c
*
* @brief KONA Peripheral Slave Clock Control Unit inline functions
*
* @note
*
*******************************************************************************/
#ifndef _CCU_KPS_INLINE_H_
#define _CCU_KPS_INLINE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <asm/arch/ccu_inline.h>
#include <asm/arch/ccu_util_inline.h>
#include <asm/arch/brcm_rdb_kps_clk_mgr_reg.h>
//#include <asm/arch/brcm_rdb_map.h>

//#define NFIX(label)	label


/* type definitions */

typedef enum
{
   ccu_kps_peri_volt_normal = NFIX(KPS_CLK_MGR_REG_VLT_PERI_VLT_NORMAL_PERI_SHIFT),
   ccu_kps_peri_volt_high   = NFIX(KPS_CLK_MGR_REG_VLT_PERI_VLT_HIGH_PERI_SHIFT)

} ccu_kps_peri_volt_e;

typedef enum
{
   ccu_kps_lvm_0 = 0,
   ccu_kps_lvm_1,
   ccu_kps_lvm_2,
   ccu_kps_lvm_3,
   ccu_kps_lvm_4,
   ccu_kps_lvm_5,
   ccu_kps_lvm_6,
   ccu_kps_lvm_7

} ccu_kps_lvm_e;

typedef enum
{
   ccu_kps_vlt_0 = 0,
   ccu_kps_vlt_1,
   ccu_kps_vlt_2,
   ccu_kps_vlt_3,
   ccu_kps_vlt_4,
   ccu_kps_vlt_5,
   ccu_kps_vlt_6,
   ccu_kps_vlt_7

} ccu_kps_vlt_e;

typedef enum
{
   ccu_kps_bus_quiesc_switch_axi_reqgnt_inh = NFIX(KPS_CLK_MGR_REG_BUS_QUIESC_SWITCH_AXI_SWITCH_REQGNT_INH_SHIFT)

} ccu_kps_bus_quiesc_e;

typedef enum
{
   ccu_kps_axi_switch_clkgate_axi_hw_sw_gating_sel  = NFIX(KPS_CLK_MGR_REG_AXI_SWITCH_CLKGATE_SWITCH_AXI_HW_SW_GATING_SEL_SHIFT),
   ccu_kps_axi_switch_clkgate_axi_hyst_val          = NFIX(KPS_CLK_MGR_REG_AXI_SWITCH_CLKGATE_SWITCH_AXI_HYST_VAL_SHIFT),
   ccu_kps_axi_switch_clkgate_axi_hyst_en           = NFIX(KPS_CLK_MGR_REG_AXI_SWITCH_CLKGATE_SWITCH_AXI_HYST_EN_SHIFT),
   ccu_kps_axi_switch_clkgate_axi_stprsts           = NFIX(KPS_CLK_MGR_REG_AXI_SWITCH_CLKGATE_SWITCH_AXI_STPRSTS_SHIFT),
   ccu_kps_axi_switch_clkgate_voltage_level         = NFIX(KPS_CLK_MGR_REG_AXI_SWITCH_CLKGATE_SWITCH_VOLTAGE_LEVEL_SHIFT)

} ccu_kps_axi_switch_clkgate_e;

typedef enum
{
   ccu_kps_axi_ext_clkgate_hw_sw_gating_sel   = NFIX(KPS_CLK_MGR_REG_AXI_EXT_CLKGATE_EXT_AXI_HW_SW_GATING_SEL_SHIFT),
   ccu_kps_axi_ext_clkgate_hyst_val           = NFIX(KPS_CLK_MGR_REG_AXI_EXT_CLKGATE_EXT_AXI_HYST_VAL_SHIFT),
   ccu_kps_axi_ext_clkgate_hyst_en            = NFIX(KPS_CLK_MGR_REG_AXI_EXT_CLKGATE_EXT_AXI_HYST_EN_SHIFT),
   ccu_kps_axi_ext_clkgate_stprsts            = NFIX(KPS_CLK_MGR_REG_AXI_EXT_CLKGATE_EXT_AXI_STPRSTS_SHIFT),
   ccu_kps_axi_ext_clkgate_voltage_level      = NFIX(KPS_CLK_MGR_REG_AXI_EXT_CLKGATE_EXT_VOLTAGE_LEVEL_SHIFT)

} ccu_kps_axi_ext_clkgate_e;

typedef enum
{
   ccu_kps_ahb1_clkgate_clk_en    = NFIX(KPS_CLK_MGR_REG_AHB1_CLKGATE_AHB1_CLK_EN_SHIFT),
   ccu_kps_ahb1_clkgate_hyst_val  = NFIX(KPS_CLK_MGR_REG_AHB1_CLKGATE_AHB1_HYST_VAL_SHIFT),
   ccu_kps_ahb1_clkgate_hyst_en   = NFIX(KPS_CLK_MGR_REG_AHB1_CLKGATE_AHB1_HYST_EN_SHIFT),
   ccu_kps_ahb1_clkgate_stprsts   = NFIX(KPS_CLK_MGR_REG_AHB1_CLKGATE_AHB1_STPRSTS_SHIFT)

} ccu_kps_ahb1_clkgate_e;

typedef enum
{
   ccu_kps_apb1_clkgate_clk_en           = NFIX(KPS_CLK_MGR_REG_APB1_CLKGATE_APB1_CLK_EN_SHIFT),
   ccu_kps_apb1_clkgate_hw_sw_gating_sel = NFIX(KPS_CLK_MGR_REG_APB1_CLKGATE_APB1_HW_SW_GATING_SEL_SHIFT),
   ccu_kps_apb1_clkgate_hyst_val         = NFIX(KPS_CLK_MGR_REG_APB1_CLKGATE_APB1_HYST_VAL_SHIFT),
   ccu_kps_apb1_clkgate_hyst_en          = NFIX(KPS_CLK_MGR_REG_APB1_CLKGATE_APB1_HYST_EN_SHIFT),
   ccu_kps_apb1_clkgate_stprsts          = NFIX(KPS_CLK_MGR_REG_APB1_CLKGATE_APB1_STPRSTS_SHIFT)

} ccu_kps_apb1_clkgate_e;

typedef enum
{
   ccu_kps_apb2_clkgate_clk_en             = NFIX(KPS_CLK_MGR_REG_APB2_CLKGATE_APB2_CLK_EN_SHIFT),
   ccu_kps_apb2_clkgate_hw_sw_gating_sel   = NFIX(KPS_CLK_MGR_REG_APB2_CLKGATE_APB2_HW_SW_GATING_SEL_SHIFT),
   ccu_kps_apb2_clkgate_hyst_val           = NFIX(KPS_CLK_MGR_REG_APB2_CLKGATE_APB2_HYST_VAL_SHIFT),
   ccu_kps_apb2_clkgate_hyst_en            = NFIX(KPS_CLK_MGR_REG_APB2_CLKGATE_APB2_HYST_EN_SHIFT),
   ccu_kps_apb2_clkgate_stprsts            = NFIX(KPS_CLK_MGR_REG_APB2_CLKGATE_APB2_STPRSTS_SHIFT)

} ccu_kps_apb2_clkgate_e;

typedef enum
{
   ccu_kps_apb3_clkgate_clk_en           = NFIX(KPS_CLK_MGR_REG_APB3_CLKGATE_APB3_CLK_EN_SHIFT),
   ccu_kps_apb3_clkgate_hw_sw_gating_sel = NFIX(KPS_CLK_MGR_REG_APB3_CLKGATE_APB3_HW_SW_GATING_SEL_SHIFT),
   ccu_kps_apb3_clkgate_hyst_val         = NFIX(KPS_CLK_MGR_REG_APB3_CLKGATE_APB3_HYST_VAL_SHIFT),
   ccu_kps_apb3_clkgate_hyst_en          = NFIX(KPS_CLK_MGR_REG_APB3_CLKGATE_APB3_HYST_EN_SHIFT),
   ccu_kps_apb3_clkgate_stprsts          = NFIX(KPS_CLK_MGR_REG_APB3_CLKGATE_APB3_STPRSTS_SHIFT)

} ccu_kps_apb3_clkgate_e;

typedef enum
{
   ccu_kps_apb2reg_clkgate_clk_en             = NFIX(KPS_CLK_MGR_REG_APB2_REG_CLKGATE_APB2_REG_CLK_EN_SHIFT),
   ccu_kps_apb2reg_clkgate_hyst_en            = NFIX(KPS_CLK_MGR_REG_APB2_REG_CLKGATE_APB2_REG_HYST_EN_SHIFT),
   ccu_kps_apb2reg_clkgate_hyst_val           = NFIX(KPS_CLK_MGR_REG_APB2_REG_CLKGATE_APB2_REG_HYST_VAL_SHIFT),
   ccu_kps_apb2reg_clkgate_stprsts            = NFIX(KPS_CLK_MGR_REG_APB2_REG_CLKGATE_APB2_REG_STPRSTS_SHIFT)

} ccu_kps_apb2reg_clkgate_e;

typedef enum
{
   ccu_kps_spum_sec_clkgate_axi_clk_en           = NFIX(KPS_CLK_MGR_REG_SPUM_SEC_CLKGATE_SPUM_SEC_AXI_CLK_EN_SHIFT),
   ccu_kps_spum_sec_clkgate_axi_hw_sw_gating_sel = NFIX(KPS_CLK_MGR_REG_SPUM_SEC_CLKGATE_SPUM_SEC_AXI_HW_SW_GATING_SEL_SHIFT),
   ccu_kps_spum_sec_clkgate_clk_en               = NFIX(KPS_CLK_MGR_REG_SPUM_SEC_CLKGATE_SPUM_SEC_CLK_EN_SHIFT),
   ccu_kps_spum_sec_clkgate_hw_sw_gating_sel     = NFIX(KPS_CLK_MGR_REG_SPUM_SEC_CLKGATE_SPUM_SEC_HW_SW_GATING_SEL_SHIFT),
   ccu_kps_spum_sec_clkgate_hyst_en              = NFIX(KPS_CLK_MGR_REG_SPUM_SEC_CLKGATE_SPUM_SEC_HYST_EN_SHIFT),
   ccu_kps_spum_sec_clkgate_hyst_val             = NFIX(KPS_CLK_MGR_REG_SPUM_SEC_CLKGATE_SPUM_SEC_HYST_VAL_SHIFT),
   ccu_kps_spum_sec_clkgate_axi_stprsts          = NFIX(KPS_CLK_MGR_REG_SPUM_SEC_CLKGATE_SPUM_SEC_AXI_STPRSTS_SHIFT),
   ccu_kps_spum_sec_clkgate_stprsts              = NFIX(KPS_CLK_MGR_REG_SPUM_SEC_CLKGATE_SPUM_SEC_STPRSTS_SHIFT)

} ccu_kps_spum_sec_clkgate_e;

typedef enum
{
   ccu_kps_spum_open_clkgate_axi_clk_en             = NFIX(KPS_CLK_MGR_REG_SPUM_OPEN_CLKGATE_SPUM_OPEN_AXI_CLK_EN_SHIFT),
   ccu_kps_spum_open_clkgate_axi_hw_sw_gating_sel   = NFIX(KPS_CLK_MGR_REG_SPUM_OPEN_CLKGATE_SPUM_OPEN_AXI_HW_SW_GATING_SEL_SHIFT),
   ccu_kps_spum_open_clkgate_clk_en                 = NFIX(KPS_CLK_MGR_REG_SPUM_OPEN_CLKGATE_SPUM_OPEN_CLK_EN_SHIFT),
   ccu_kps_spum_open_clkgate_hw_sw_gating_sel       = NFIX(KPS_CLK_MGR_REG_SPUM_OPEN_CLKGATE_SPUM_OPEN_HW_SW_GATING_SEL_SHIFT),
   ccu_kps_spum_open_clkgate_hyst_en                = NFIX(KPS_CLK_MGR_REG_SPUM_OPEN_CLKGATE_SPUM_OPEN_HYST_EN_SHIFT),
   ccu_kps_spum_open_clkgate_hyst_val               = NFIX(KPS_CLK_MGR_REG_SPUM_OPEN_CLKGATE_SPUM_OPEN_HYST_VAL_SHIFT),
   ccu_kps_spum_open_clkgate_axi_stprsts            = NFIX(KPS_CLK_MGR_REG_SPUM_OPEN_CLKGATE_SPUM_OPEN_AXI_STPRSTS_SHIFT),
   ccu_kps_spum_open_clkgate_stprsts                = NFIX(KPS_CLK_MGR_REG_SPUM_OPEN_CLKGATE_SPUM_OPEN_STPRSTS_SHIFT)

} ccu_kps_spum_open_clkgate_e;

typedef enum
{
   ccu_kps_mphi_clkgate_ahb_clk_en            = NFIX(KPS_CLK_MGR_REG_MPHI_CLKGATE_MPHI_AHB_CLK_EN_SHIFT),
   ccu_kps_mphi_clkgate_ahb_hw_sw_gating_sel  = NFIX(KPS_CLK_MGR_REG_MPHI_CLKGATE_MPHI_AHB_HW_SW_GATING_SEL_SHIFT),
   ccu_kps_mphi_clkgate_ahb_stprsts           = NFIX(KPS_CLK_MGR_REG_MPHI_CLKGATE_MPHI_AHB_STPRSTS_SHIFT),
   ccu_kps_mphi_clkgate_voltage_level         = NFIX(KPS_CLK_MGR_REG_MPHI_CLKGATE_MPHI_VOLTAGE_LEVEL_SHIFT)

} ccu_kps_mphi_clkgate_e;

typedef enum
{
   ccu_kps_uart_clkgate_apb_clk_en            = NFIX(KPS_CLK_MGR_REG_UARTB_CLKGATE_UARTB_APB_CLK_EN_SHIFT),
   ccu_kps_uart_clkgate_apb_hw_sw_gating_sel  = NFIX(KPS_CLK_MGR_REG_UARTB_CLKGATE_UARTB_APB_HW_SW_GATING_SEL_SHIFT),
   ccu_kps_uart_clkgate_clk_en                = NFIX(KPS_CLK_MGR_REG_UARTB_CLKGATE_UARTB_CLK_EN_SHIFT),
   ccu_kps_uart_clkgate_hw_sw_gating_sel      = NFIX(KPS_CLK_MGR_REG_UARTB_CLKGATE_UARTB_HW_SW_GATING_SEL_SHIFT),
   ccu_kps_uart_clkgate_apb_stprsts           = NFIX(KPS_CLK_MGR_REG_UARTB_CLKGATE_UARTB_APB_STPRSTS_SHIFT),
   ccu_kps_uart_clkgate_stprsts               = NFIX(KPS_CLK_MGR_REG_UARTB_CLKGATE_UARTB_STPRSTS_SHIFT),
   ccu_kps_uart_clkgate_voltage_level         = NFIX(KPS_CLK_MGR_REG_UARTB_CLKGATE_UARTB_VOLTAGE_LEVEL_SHIFT)

} ccu_kps_uart_clkgate_e;

typedef enum
{
   ccu_kps_ssp_clkgate_apb_clk_en             = NFIX(KPS_CLK_MGR_REG_SSP0_CLKGATE_SSP0_APB_CLK_EN_SHIFT),
   ccu_kps_ssp_clkgate_apb_hw_sw_gating_sel   = NFIX(KPS_CLK_MGR_REG_SSP0_CLKGATE_SSP0_APB_HW_SW_GATING_SEL_SHIFT),
   ccu_kps_ssp_clkgate_clk_en                 = NFIX(KPS_CLK_MGR_REG_SSP0_CLKGATE_SSP0_CLK_EN_SHIFT),
   ccu_kps_ssp_clkgate_hw_sw_gating_sel       = NFIX(KPS_CLK_MGR_REG_SSP0_CLKGATE_SSP0_HW_SW_GATING_SEL_SHIFT),
   ccu_kps_ssp_clkgate_audio_clk_en           = NFIX(KPS_CLK_MGR_REG_SSP0_CLKGATE_SSP0_AUDIO_CLK_EN_SHIFT),
   ccu_kps_ssp_clkgate_audio_hw_sw_gating_sel = NFIX(KPS_CLK_MGR_REG_SSP0_CLKGATE_SSP0_AUDIO_HW_SW_GATING_SEL_SHIFT),
   ccu_kps_ssp_clkgate_audio_hyst_en          = NFIX(KPS_CLK_MGR_REG_SSP0_CLKGATE_SSP0_AUDIO_HYST_EN_SHIFT),
   ccu_kps_ssp_clkgate_audio_hyst_val         = NFIX(KPS_CLK_MGR_REG_SSP0_CLKGATE_SSP0_AUDIO_HYST_VAL_SHIFT),
   ccu_kps_ssp_clkgate_apb_stprsts            = NFIX(KPS_CLK_MGR_REG_SSP0_CLKGATE_SSP0_APB_STPRSTS_SHIFT),
   ccu_kps_ssp_clkgate_stprsts                = NFIX(KPS_CLK_MGR_REG_SSP0_CLKGATE_SSP0_STPRSTS_SHIFT),
   ccu_kps_ssp_clkgate_audio_stprsts          = NFIX(KPS_CLK_MGR_REG_SSP0_CLKGATE_SSP0_AUDIO_STPRSTS_SHIFT),
   ccu_kps_ssp_clkgate_voltage_level          = NFIX(KPS_CLK_MGR_REG_SSP0_CLKGATE_SSP0_VOLTAGE_LEVEL_SHIFT)

} ccu_kps_ssp_clkgate_e;

typedef enum
{
   ccu_kps_timers_clkgate_apb_clk_en             = NFIX(KPS_CLK_MGR_REG_TIMERS_CLKGATE_TIMERS_APB_CLK_EN_SHIFT),
   ccu_kps_timers_clkgate_apb_hw_sw_gating_sel   = NFIX(KPS_CLK_MGR_REG_TIMERS_CLKGATE_TIMERS_APB_HW_SW_GATING_SEL_SHIFT),
   ccu_kps_timers_clkgate_clk_en                 = NFIX(KPS_CLK_MGR_REG_TIMERS_CLKGATE_TIMERS_CLK_EN_SHIFT),
   ccu_kps_timers_clkgate_hw_sw_gating_sel       = NFIX(KPS_CLK_MGR_REG_TIMERS_CLKGATE_TIMERS_HW_SW_GATING_SEL_SHIFT),
   ccu_kps_timers_clkgate_apb_stprsts            = NFIX(KPS_CLK_MGR_REG_TIMERS_CLKGATE_TIMERS_APB_STPRSTS_SHIFT),
   ccu_kps_timers_clkgate_stprsts                = NFIX(KPS_CLK_MGR_REG_TIMERS_CLKGATE_TIMERS_STPRSTS_SHIFT),
   ccu_kps_timers_clkgate_voltage_level          = NFIX(KPS_CLK_MGR_REG_TIMERS_CLKGATE_TIMERS_VOLTAGE_LEVEL_SHIFT)

} ccu_kps_timers_clkgate_e;

typedef enum
{
   ccu_kps_dmac_mux_clkgate_apb_clk_en           = NFIX(KPS_CLK_MGR_REG_DMAC_MUX_CLKGATE_DMAC_MUX_APB_CLK_EN_SHIFT),
   ccu_kps_dmac_mux_clkgate_apb_hw_sw_gating_sel = NFIX(KPS_CLK_MGR_REG_DMAC_MUX_CLKGATE_DMAC_MUX_APB_HW_SW_GATING_SEL_SHIFT),
   ccu_kps_dmac_mux_clkgate_apb_stprsts          = NFIX(KPS_CLK_MGR_REG_DMAC_MUX_CLKGATE_DMAC_MUX_APB_STPRSTS_SHIFT),
   ccu_kps_dmac_mux_clkgate_voltage_level        = NFIX(KPS_CLK_MGR_REG_DMAC_MUX_CLKGATE_DMAC_MUX_VOLTAGE_LEVEL_SHIFT)

} ccu_kps_dmac_mux_clkgate_e;

typedef enum
{
   ccu_kps_bsc_clkgate_apb_clk_en             = NFIX(KPS_CLK_MGR_REG_BSC1_CLKGATE_BSC1_APB_CLK_EN_SHIFT),
   ccu_kps_bsc_clkgate_apb_hw_sw_gating_sel   = NFIX(KPS_CLK_MGR_REG_BSC1_CLKGATE_BSC1_APB_HW_SW_GATING_SEL_SHIFT),
   ccu_kps_bsc_clkgate_clk_en                 = NFIX(KPS_CLK_MGR_REG_BSC1_CLKGATE_BSC1_CLK_EN_SHIFT),
   ccu_kps_bsc_clkgate_hw_sw_gating_sel       = NFIX(KPS_CLK_MGR_REG_BSC1_CLKGATE_BSC1_HW_SW_GATING_SEL_SHIFT),
   ccu_kps_bsc_clkgate_apb_hyst_en            = NFIX(KPS_CLK_MGR_REG_BSC1_CLKGATE_BSC1_APB_HYST_EN_SHIFT),
   ccu_kps_bsc_clkgate_apb_hyst_val           = NFIX(KPS_CLK_MGR_REG_BSC1_CLKGATE_BSC1_APB_HYST_VAL_SHIFT),
   ccu_kps_bsc_clkgate_apb_stprsts            = NFIX(KPS_CLK_MGR_REG_BSC1_CLKGATE_BSC1_APB_STPRSTS_SHIFT),
   ccu_kps_bsc_clkgate_stprsts                = NFIX(KPS_CLK_MGR_REG_BSC1_CLKGATE_BSC1_STPRSTS_SHIFT),
   ccu_kps_bsc_clkgate_voltage_level          = NFIX(KPS_CLK_MGR_REG_BSC1_CLKGATE_BSC1_VOLTAGE_LEVEL_SHIFT)

} ccu_kps_bsc_clkgate_e;

typedef enum
{
   ccu_kps_pwm_clkgate_apb_clk_en             = NFIX(KPS_CLK_MGR_REG_PWM_CLKGATE_PWM_APB_CLK_EN_SHIFT),
   ccu_kps_pwm_clkgate_apb_hw_sw_gating_sel   = NFIX(KPS_CLK_MGR_REG_PWM_CLKGATE_PWM_APB_HW_SW_GATING_SEL_SHIFT),
   ccu_kps_pwm_clkgate_clk_en                 = NFIX(KPS_CLK_MGR_REG_PWM_CLKGATE_PWM_CLK_EN_SHIFT),
   ccu_kps_pwm_clkgate_hw_sw_gating_sel       = NFIX(KPS_CLK_MGR_REG_PWM_CLKGATE_PWM_HW_SW_GATING_SEL_SHIFT),
   ccu_kps_pwm_clkgate_apb_hyst_en            = NFIX(KPS_CLK_MGR_REG_PWM_CLKGATE_PWM_APB_HYST_EN_SHIFT),
   ccu_kps_pwm_clkgate_apb_hyst_val           = NFIX(KPS_CLK_MGR_REG_PWM_CLKGATE_PWM_APB_HYST_VAL_SHIFT),
   ccu_kps_pwm_clkgate_apb_stprsts            = NFIX(KPS_CLK_MGR_REG_PWM_CLKGATE_PWM_APB_STPRSTS_SHIFT),
   ccu_kps_pwm_clkgate_stprsts                = NFIX(KPS_CLK_MGR_REG_PWM_CLKGATE_PWM_STPRSTS_SHIFT),
   ccu_kps_pwm_clkgate_voltage_level          = NFIX(KPS_CLK_MGR_REG_PWM_CLKGATE_PWM_VOLTAGE_LEVEL_SHIFT)

} ccu_kps_pwm_clkgate_e;

typedef enum
{
   ccu_kps_bbl_clkgate_apb_clk_en             = NFIX(KPS_CLK_MGR_REG_BBL_CLKGATE_BBL_REG_APB_CLK_EN_SHIFT),
   ccu_kps_bbl_clkgate_apb_hw_sw_gating_sel   = NFIX(KPS_CLK_MGR_REG_BBL_CLKGATE_BBL_REG_APB_HW_SW_GATING_SEL_SHIFT),
   ccu_kps_bbl_clkgate_apb_hyst_en            = NFIX(KPS_CLK_MGR_REG_BBL_CLKGATE_BBL_REG_APB_HYST_EN_SHIFT),
   ccu_kps_bbl_clkgate_apb_hyst_val           = NFIX(KPS_CLK_MGR_REG_BBL_CLKGATE_BBL_REG_APB_HYST_VAL_SHIFT),
   ccu_kps_bbl_clkgate_apb_stprsts            = NFIX(KPS_CLK_MGR_REG_BBL_CLKGATE_BBL_REG_APB_STPRSTS_SHIFT),
   ccu_kps_bbl_clkgate_voltage_level          = NFIX(KPS_CLK_MGR_REG_BBL_CLKGATE_BBL_REG_VOLTAGE_LEVEL_SHIFT)

} ccu_kps_bbl_clkgate_e;

typedef enum
{
   ccu_kps_dap_clkgate_clk_en           = NFIX(KPS_CLK_MGR_REG_DAP_CLKGATE_DAP_CLK_EN_SHIFT),
   ccu_kps_dap_clkgate_hw_sw_gating_sel = NFIX(KPS_CLK_MGR_REG_DAP_CLKGATE_DAP_HW_SW_GATING_SEL_SHIFT),
   ccu_kps_dap_clkgate_hyst_val         = NFIX(KPS_CLK_MGR_REG_DAP_CLKGATE_DAP_HYST_VAL_SHIFT),
   ccu_kps_dap_clkgate_hyst_en          = NFIX(KPS_CLK_MGR_REG_DAP_CLKGATE_DAP_HYST_EN_SHIFT),
   ccu_kps_dap_clkgate_stprsts          = NFIX(KPS_CLK_MGR_REG_DAP_CLKGATE_DAP_STPRSTS_SHIFT)

} ccu_kps_dap_clkgate_e;

typedef enum
{
   ccu_kps_cir_clkgate_apb_clk_en           = NFIX(KPS_CLK_MGR_REG_CIR_CLKGATE_CIR_APB_CLK_EN_SHIFT),
   ccu_kps_cir_clkgate_apb_hw_sw_gating_sel = NFIX(KPS_CLK_MGR_REG_CIR_CLKGATE_CIR_APB_HW_SW_GATING_SEL_SHIFT),
   ccu_kps_cir_clkgate_apb_stprsts          = NFIX(KPS_CLK_MGR_REG_CIR_CLKGATE_CIR_APB_STPRSTS_SHIFT),
   ccu_kps_cir_clkgate_voltage_level        = NFIX(KPS_CLK_MGR_REG_CIR_CLKGATE_CIR_VOLTAGE_LEVEL_SHIFT)
	
} ccu_kps_cir_clkgate_e;

typedef enum
{
   ccu_kps_ledm_clkgate_apb_clk_en           = NFIX(KPS_CLK_MGR_REG_LEDM_CLKGATE_LEDM_APB_CLK_EN_SHIFT),
   ccu_kps_ledm_clkgate_apb_hw_sw_gating_sel = NFIX(KPS_CLK_MGR_REG_LEDM_CLKGATE_LEDM_APB_HW_SW_GATING_SEL_SHIFT),
   ccu_kps_ledm_clkgate_apb_hyst_val         = NFIX(KPS_CLK_MGR_REG_LEDM_CLKGATE_LEDM_APB_HYST_VAL_SHIFT),
   ccu_kps_ledm_clkgate_apb_hyst_en          = NFIX(KPS_CLK_MGR_REG_LEDM_CLKGATE_LEDM_APB_HYST_EN_SHIFT),
   ccu_kps_ledm_clkgate_apb_stprsts          = NFIX(KPS_CLK_MGR_REG_LEDM_CLKGATE_LEDM_APB_STPRSTS_SHIFT),
   ccu_kps_ledm_clkgate_voltage_level        = NFIX(KPS_CLK_MGR_REG_LEDM_CLKGATE_LEDM_VOLTAGE_LEVEL_SHIFT)
	
} ccu_kps_ledm_clkgate_e;

typedef enum
{
   ccu_kps_spum_sec_div_pll_select = NFIX(KPS_CLK_MGR_REG_SPUM_SEC_DIV_SPUM_SEC_PLL_SELECT_SHIFT),
   ccu_kps_spum_sec_div_div        = NFIX(KPS_CLK_MGR_REG_SPUM_SEC_DIV_SPUM_SEC_DIV_SHIFT)

} ccu_kps_spum_sec_div_e;

typedef enum
{
   ccu_kps_spum_open_div_pll_select = NFIX(KPS_CLK_MGR_REG_SPUM_OPEN_DIV_SPUM_OPEN_PLL_SELECT_SHIFT),
   ccu_kps_spum_open_div_div        = NFIX(KPS_CLK_MGR_REG_SPUM_OPEN_DIV_SPUM_OPEN_DIV_SHIFT)

} ccu_kps_spum_open_div_e;

typedef enum
{
   ccu_ksp_apb7_clkgate_clk_en           = NFIX(KPS_CLK_MGR_REG_APB7_CLKGATE_APB7_CLK_EN_SHIFT),
   ccu_ksp_apb7_clkgate_hw_sw_gating_sel = NFIX(KPS_CLK_MGR_REG_APB7_CLKGATE_APB7_HW_SW_GATING_SEL_SHIFT),
   ccu_ksp_apb7_clkgate_hyst_val         = NFIX(KPS_CLK_MGR_REG_APB7_CLKGATE_APB7_HYST_VAL_SHIFT),
   ccu_ksp_apb7_clkgate_hyst_en          = NFIX(KPS_CLK_MGR_REG_APB7_CLKGATE_APB7_HYST_EN_SHIFT),
   ccu_ksp_apb7_clkgate_stprsts          = NFIX(KPS_CLK_MGR_REG_APB7_CLKGATE_APB7_STPRSTS_SHIFT)
	
} ccu_kps_apb7_clkgate_e;

typedef enum
{
   ccu_kps_apb_spum_sec_clkgate_clk_en           = NFIX(KPS_CLK_MGR_REG_SPUM_SEC_APB_CLKGATE_SPUM_SEC_APB_CLK_EN_SHIFT),
   ccu_kps_apb_spum_sec_clkgate_hw_sw_gating_sel = NFIX(KPS_CLK_MGR_REG_SPUM_SEC_APB_CLKGATE_SPUM_SEC_APB_HW_SW_GATING_SEL_SHIFT),
   ccu_kps_apb_spum_sec_clkgate_stprsts          = NFIX(KPS_CLK_MGR_REG_SPUM_SEC_APB_CLKGATE_SPUM_SEC_APB_STPRSTS_SHIFT),
   ccu_kps_apb_spum_sec_clkgate_voltage_level    = NFIX(KPS_CLK_MGR_REG_SPUM_OPEN_APB_CLKGATE_SPUM_OPEN_VOLTAGE_LEVEL_SHIFT)

} ccu_kps_apb_spum_sec_clkgate_e;

typedef enum
{
   ccu_kps_apb_spum_open_clkgate_clk_en             = NFIX(KPS_CLK_MGR_REG_SPUM_OPEN_APB_CLKGATE_SPUM_OPEN_APB_CLK_EN_SHIFT),
   ccu_kps_apb_spum_open_clkgate_hw_sw_gating_sel   = NFIX(KPS_CLK_MGR_REG_SPUM_OPEN_APB_CLKGATE_SPUM_OPEN_APB_HW_SW_GATING_SEL_SHIFT),
   ccu_kps_apb_spum_open_clkgate_stprsts            = NFIX(KPS_CLK_MGR_REG_SPUM_OPEN_APB_CLKGATE_SPUM_OPEN_APB_STPRSTS_SHIFT),
   ccu_kps_apb_spum_open_clkgate_voltage_level      = NFIX(KPS_CLK_MGR_REG_SPUM_OPEN_APB_CLKGATE_SPUM_OPEN_VOLTAGE_LEVEL_SHIFT)

} ccu_kps_apb_spum_open_clkgate_e;

typedef enum
{
   ccu_kps_axi_div_switch_axi_pll_select         = NFIX(KPS_CLK_MGR_REG_AXI_DIV_SWITCH_AXI_PLL_SELECT_SHIFT),
   ccu_kps_axi_div_switch_axi_div                = NFIX(KPS_CLK_MGR_REG_AXI_DIV_SWITCH_AXI_DIV_SHIFT),
   ccu_kps_axi_div_apb1_div                      = NFIX(KPS_CLK_MGR_REG_AXI_DIV_APB1_DIV_SHIFT),
   ccu_kps_axi_div_apb2_free_div                 = NFIX(KPS_CLK_MGR_REG_AXI_DIV_APB2_FREE_DIV_SHIFT),
   ccu_kps_axi_div_apb3_div                      = NFIX(KPS_CLK_MGR_REG_AXI_DIV_APB3_DIV_SHIFT),
   ccu_kps_axi_div_apb7_div                      = NFIX(KPS_CLK_MGR_REG_AXI_DIV_APB7_DIV_SHIFT),
   ccu_kps_axi_div_switch_axi_trigger            = NFIX(KPS_CLK_MGR_REG_AXI_DIV_SWITCH_AXI_TRIGGER_SHIFT),
   ccu_kps_axi_div_switch_axi_trigger_override   = NFIX(KPS_CLK_MGR_REG_AXI_DIV_SWITCH_AXI_TRIGGER_OVERRIDE_SHIFT),
   ccu_kps_axi_div_switch_axi_pll_sel_override   = NFIX(KPS_CLK_MGR_REG_AXI_DIV_SWITCH_AXI_PLL_SELECT_OVERRIDE_SHIFT),
   ccu_kps_axi_div_switch_axi_div_override       = NFIX(KPS_CLK_MGR_REG_AXI_DIV_SWITCH_AXI_DIV_OVERRIDE_SHIFT),   
   ccu_kps_axi_div_apb1_div_override             = NFIX(KPS_CLK_MGR_REG_AXI_DIV_APB1_DIV_OVERRIDE_SHIFT),
   ccu_kps_axi_div_apb2_free_div_override        = NFIX(KPS_CLK_MGR_REG_AXI_DIV_APB2_FREE_DIV_OVERRIDE_SHIFT),
   ccu_kps_axi_div_apb3_div_override             = NFIX(KPS_CLK_MGR_REG_AXI_DIV_APB3_DIV_OVERRIDE_SHIFT),
   ccu_kps_axi_div_apb7_div_override             = NFIX(KPS_CLK_MGR_REG_AXI_DIV_APB7_DIV_OVERRIDE_SHIFT),

} ccu_kps_axi_div_e;

typedef enum
{
   ccu_kps_uart_div_pll_select = NFIX(KPS_CLK_MGR_REG_UARTB_DIV_UARTB_PLL_SELECT_SHIFT),
   ccu_kps_uart_div_div        = NFIX(KPS_CLK_MGR_REG_UARTB_DIV_UARTB_DIV_SHIFT)

} ccu_kps_uart_div_e;

typedef enum
{
   ccu_kps_ssp_div_pll_select  = NFIX(KPS_CLK_MGR_REG_SSP0_DIV_SSP0_PLL_SELECT_SHIFT),
   ccu_kps_ssp_div             = NFIX(KPS_CLK_MGR_REG_SSP0_DIV_SSP0_DIV_SHIFT)

} ccu_kps_ssp_div_e;

typedef enum
{
   ccu_kps_ssp_audio_div_pre_pll_sel = NFIX(KPS_CLK_MGR_REG_SSP0_AUDIO_DIV_SSP0_AUDIO_PRE_PLL_SELECT_SHIFT),
   ccu_kps_ssp_audio_div_pre_div     = NFIX(KPS_CLK_MGR_REG_SSP0_AUDIO_DIV_SSP0_AUDIO_PRE_DIV_SHIFT),
   ccu_kps_ssp_audio_div             = NFIX(KPS_CLK_MGR_REG_SSP0_AUDIO_DIV_SSP0_AUDIO_DIV_SHIFT)

} ccu_kps_ssp_audio_div_e;

typedef enum
{
   ccu_kps_bsc_div_pll_sel  = NFIX(KPS_CLK_MGR_REG_BSC1_DIV_BSC1_PLL_SELECT_SHIFT),

} ccu_kps_bsc_div_e;

typedef enum
{
   ccu_kps_pwm_div_pll_sel = NFIX(KPS_CLK_MGR_REG_PWM_DIV_PWM_PLL_SELECT_SHIFT),
   ccu_kps_pwm_div         = NFIX(KPS_CLK_MGR_REG_PWM_DIV_PWM_DIV_SHIFT)
	
} ccu_kps_pwm_div_e;

typedef enum
{
   ccu_kps_timers_div_pll_sel  = NFIX(KPS_CLK_MGR_REG_TIMERS_DIV_TIMERS_PLL_SELECT_SHIFT)

} ccu_kps_timers_div_e;

typedef enum
{
   ccu_kps_div_trig_switch_axi_trig     = NFIX(KPS_CLK_MGR_REG_AXI_DIV_SWITCH_AXI_TRIGGER_OVERRIDE_SHIFT),
   ccu_kps_div_trig_uartb_trig          = NFIX(KPS_CLK_MGR_REG_DIV_TRIG_UARTB_TRIGGER_SHIFT),
   ccu_kps_div_trig_uartb2_trig         = NFIX(KPS_CLK_MGR_REG_DIV_TRIG_UARTB2_TRIGGER_SHIFT),
   ccu_kps_div_trig_uartb3_trig         = NFIX(KPS_CLK_MGR_REG_DIV_TRIG_UARTB3_TRIGGER_SHIFT),
   ccu_kps_div_trig_uartb4_trig         = NFIX(KPS_CLK_MGR_REG_DIV_TRIG_UARTB4_TRIGGER_SHIFT),
   ccu_kps_div_trig_ssp0_trig           = NFIX(KPS_CLK_MGR_REG_DIV_TRIG_SSP0_TRIGGER_SHIFT),
   ccu_kps_div_trig_ssp2_trig           = NFIX(KPS_CLK_MGR_REG_DIV_TRIG_SSP2_TRIGGER_SHIFT),
   ccu_kps_div_trig_ssp0_audio_pre_trig = NFIX(KPS_CLK_MGR_REG_DIV_TRIG_SSP0_AUDIO_PRE_TRIGGER_SHIFT),
   ccu_kps_div_trig_ssp2_audio_pre_trig = NFIX(KPS_CLK_MGR_REG_DIV_TRIG_SSP2_AUDIO_PRE_TRIGGER_SHIFT),
   ccu_kps_div_trig_pwm_trig            = NFIX(KPS_CLK_MGR_REG_DIV_TRIG_PWM_TRIGGER_SHIFT),
   ccu_kps_div_trig_timers_trig         = NFIX(KPS_CLK_MGR_REG_DIV_TRIG_TIMERS_TRIGGER_SHIFT),
   ccu_kps_div_trig_bsc1_trig           = NFIX(KPS_CLK_MGR_REG_DIV_TRIG_BSC1_TRIGGER_SHIFT),
   ccu_kps_div_trig_bsc2_trig           = NFIX(KPS_CLK_MGR_REG_DIV_TRIG_BSC2_TRIGGER_SHIFT),
   ccu_kps_div_trig_spum_sec_trig       = NFIX(KPS_CLK_MGR_REG_DIV_TRIG_SPUM_SEC_TRIGGER_SHIFT),
   ccu_kps_div_trig_spum_open_trig      = NFIX(KPS_CLK_MGR_REG_DIV_TRIG_SPUM_OPEN_TRIGGER_SHIFT),
   
   ccu_kps_div_trig_uartb5_trig         = 32 + NFIX(KPS_CLK_MGR_REG_DIV_TRIG2_UARTB5_TRIGGER_SHIFT),
   ccu_kps_div_trig_uartb6_trig         = 32 + NFIX(KPS_CLK_MGR_REG_DIV_TRIG2_UARTB6_TRIGGER_SHIFT),
   ccu_kps_div_trig_bsc3_trig           = 32 + NFIX(KPS_CLK_MGR_REG_DIV_TRIG2_BSC3_TRIGGER_SHIFT)

} ccu_kps_div_trig_e;


typedef enum
{
   ccu_kps_clkmon_sel = NFIX(KPS_CLK_MGR_REG_CLKMON_CLKMON_SEL_SHIFT),
   ccu_kps_clkmon_ctl = NFIX(KPS_CLK_MGR_REG_CLKMON_CLKMON_CTL_SHIFT)

} ccu_kps_clkmon_e;




/* Function Prototypes */
static inline void      ccu_set_kps_peri_volt( ccu_kps_peri_volt_e field, uint32_t val );
static inline uint32_t  ccu_get_kps_peri_volt( ccu_kps_peri_volt_e field );
static inline void      ccu_set_kps_lvm( ccu_kps_lvm_e field, uint32_t val );
static inline uint32_t  ccu_get_kps_lvm( ccu_kps_lvm_e field );
static inline void      ccu_set_kps_vlt( ccu_kps_vlt_e field, uint32_t val );
static inline uint32_t  ccu_get_kps_vlt( ccu_kps_vlt_e field );
static inline void      ccu_set_kps_bus_quiesc( ccu_kps_bus_quiesc_e field, uint32_t val );
static inline uint32_t  ccu_get_kps_bus_quiesc( ccu_kps_bus_quiesc_e field );
static inline void      ccu_set_kps_axi_switch_clkgate( ccu_kps_axi_switch_clkgate_e field, uint32_t val );
static inline uint32_t  ccu_get_kps_axi_switch_clkgate( ccu_kps_axi_switch_clkgate_e field );
static inline void      ccu_set_kps_axi_ext_clkgate( ccu_kps_axi_ext_clkgate_e field, uint32_t val );
static inline uint32_t  ccu_get_kps_axi_ext_clkgate( ccu_kps_axi_ext_clkgate_e field );
static inline void      ccu_set_kps_ahb1_clkgate( ccu_kps_ahb1_clkgate_e field, uint32_t val );
static inline uint32_t  ccu_get_kps_ahb1_clkgate( ccu_kps_ahb1_clkgate_e field );
static inline void      ccu_set_kps_apb1_clkgate( ccu_kps_apb1_clkgate_e field, uint32_t val );
static inline uint32_t  ccu_get_kps_apb1_clkgate( ccu_kps_apb1_clkgate_e field );
static inline void      ccu_set_kps_apb2_clkgate( ccu_kps_apb2_clkgate_e field, uint32_t val );
static inline uint32_t  ccu_get_kps_apb2_clkgate( ccu_kps_apb2_clkgate_e field );
static inline void      ccu_set_kps_apb3_clkgate( ccu_kps_apb3_clkgate_e field, uint32_t val );
static inline uint32_t  ccu_get_kps_apb3_clkgate( ccu_kps_apb3_clkgate_e field );
static inline void      ccu_set_kps_apb2reg_clkgate( ccu_kps_apb2reg_clkgate_e field, uint32_t val );
static inline uint32_t  ccu_get_kps_apb2reg_clkgate( ccu_kps_apb2reg_clkgate_e field );
static inline void      ccu_set_kps_spum_sec_clkgate( ccu_kps_spum_sec_clkgate_e field, uint32_t val );
static inline uint32_t  ccu_get_kps_spum_sec_clkgate( ccu_kps_spum_sec_clkgate_e field );
static inline void      ccu_set_kps_spum_open_clkgate( ccu_kps_spum_open_clkgate_e field, uint32_t val );
static inline uint32_t  ccu_get_kps_spum_open_clkgate( ccu_kps_spum_open_clkgate_e field );
static inline void      ccu_set_kps_mphi_clkgate( ccu_kps_mphi_clkgate_e field, uint32_t val );
static inline uint32_t  ccu_get_kps_mphi_clkgate( ccu_kps_mphi_clkgate_e field );
static inline void      ccu_set_kps_uartb_clkgate( ccu_kps_uart_clkgate_e field, uint32_t val );
static inline uint32_t  ccu_get_kps_uartb_clkgate( ccu_kps_uart_clkgate_e field );
static inline void      ccu_set_kps_uartb2_clkgate( ccu_kps_uart_clkgate_e field, uint32_t val );
static inline uint32_t  ccu_get_kps_uartb2_clkgate( ccu_kps_uart_clkgate_e field );
static inline void      ccu_set_kps_uartb3_clkgate( ccu_kps_uart_clkgate_e field, uint32_t val );
static inline uint32_t  ccu_get_kps_uartb3_clkgate( ccu_kps_uart_clkgate_e field );
static inline void      ccu_set_kps_uartb4_clkgate( ccu_kps_uart_clkgate_e field, uint32_t val );
static inline uint32_t  ccu_get_kps_uartb4_clkgate( ccu_kps_uart_clkgate_e field );
static inline void      ccu_set_kps_uartb5_clkgate( ccu_kps_uart_clkgate_e field, uint32_t val );
static inline uint32_t  ccu_get_kps_uartb5_clkgate( ccu_kps_uart_clkgate_e field );
static inline void      ccu_set_kps_uartb6_clkgate( ccu_kps_uart_clkgate_e field, uint32_t val );
static inline uint32_t  ccu_get_kps_uartb6_clkgate( ccu_kps_uart_clkgate_e field );
static inline void      ccu_set_kps_ssp0_clkgate( ccu_kps_ssp_clkgate_e field, uint32_t val );
static inline uint32_t  ccu_get_kps_ssp0_clkgate( ccu_kps_ssp_clkgate_e field );
static inline void      ccu_set_kps_ssp2_clkgate( ccu_kps_ssp_clkgate_e field, uint32_t val );
static inline uint32_t  ccu_get_kps_ssp2_clkgate( ccu_kps_ssp_clkgate_e field );
static inline void      ccu_set_kps_timers_clkgate( ccu_kps_timers_clkgate_e field, uint32_t val );
static inline uint32_t  ccu_get_kps_timers_clkgate( ccu_kps_timers_clkgate_e field );
static inline void      ccu_set_kps_dmac_mux_clkgate( ccu_kps_dmac_mux_clkgate_e field, uint32_t val );
static inline uint32_t  ccu_get_kps_dmac_mux_clkgate( ccu_kps_dmac_mux_clkgate_e field );
static inline void      ccu_set_kps_bsc1_clkgate( ccu_kps_bsc_clkgate_e field, uint32_t val );
static inline uint32_t  ccu_get_kps_bsc1_clkgate( ccu_kps_bsc_clkgate_e field );
static inline void      ccu_set_kps_bsc2_clkgate( ccu_kps_bsc_clkgate_e field, uint32_t val );
static inline uint32_t  ccu_get_kps_bsc2_clkgate( ccu_kps_bsc_clkgate_e field );
static inline void      ccu_set_kps_bsc3_clkgate( ccu_kps_bsc_clkgate_e field, uint32_t val );
static inline uint32_t  ccu_get_kps_bsc3_clkgate( ccu_kps_bsc_clkgate_e field );
static inline void      ccu_set_kps_pwm_clkgate( ccu_kps_pwm_clkgate_e field, uint32_t val );
static inline uint32_t  ccu_get_kps_pwm_clkgate( ccu_kps_pwm_clkgate_e field );
static inline void      ccu_set_kps_bbl_clkgate( ccu_kps_bbl_clkgate_e field, uint32_t val );
static inline uint32_t  ccu_get_kps_bbl_clkgate( ccu_kps_bbl_clkgate_e field );
static inline void      ccu_set_kps_dap_clkgate( ccu_kps_dap_clkgate_e field, uint32_t val );
static inline uint32_t  ccu_get_kps_dap_clkgate( ccu_kps_dap_clkgate_e field );
static inline void      ccu_set_kps_cir_clkgate( ccu_kps_cir_clkgate_e field, uint32_t val );
static inline uint32_t  ccu_get_kps_cir_clkgate( ccu_kps_cir_clkgate_e field );
static inline void      ccu_set_kps_ledm_clkgate( ccu_kps_ledm_clkgate_e field, uint32_t val );
static inline uint32_t  ccu_get_kps_ledm_clkgate( ccu_kps_ledm_clkgate_e field );
static inline void      ccu_set_kps_spum_sec_div( ccu_kps_spum_sec_div_e field, uint32_t val );
static inline uint32_t  ccu_get_kps_spum_sec_div( ccu_kps_spum_sec_div_e field );
static inline void      ccu_set_kps_spum_open_div( ccu_kps_spum_open_div_e field, uint32_t val );
static inline uint32_t  ccu_get_kps_spum_open_div( ccu_kps_spum_open_div_e field );
static inline void      ccu_set_kps_apb7_clkgate( ccu_kps_apb7_clkgate_e field, uint32_t val );
static inline uint32_t  ccu_get_kps_apb7_clkgate( ccu_kps_apb7_clkgate_e field );
static inline void      ccu_set_kps_apb_spum_sec_clkgate( ccu_kps_apb_spum_sec_clkgate_e field, uint32_t val );
static inline uint32_t  ccu_get_kps_apb_spum_sec_clkgate( ccu_kps_apb_spum_sec_clkgate_e field );
static inline void      ccu_set_kps_apb_spum_open_clkgate( ccu_kps_apb_spum_open_clkgate_e field, uint32_t val );
static inline uint32_t  ccu_get_kps_apb_spum_open_clkgate( ccu_kps_apb_spum_open_clkgate_e field );
static inline void      ccu_set_kps_axi_div( ccu_kps_axi_div_e field, uint32_t val );
static inline uint32_t  ccu_get_kps_axi_div( ccu_kps_axi_div_e field );
static inline void      ccu_set_kps_uartb_div( ccu_kps_uart_div_e field, uint32_t val );
static inline uint32_t  ccu_get_kps_uartb_div( ccu_kps_uart_div_e field );
static inline void      ccu_set_kps_uartb2_div( ccu_kps_uart_div_e field, uint32_t val );
static inline uint32_t  ccu_get_kps_uartb2_div( ccu_kps_uart_div_e field );
static inline void      ccu_set_kps_uartb3_div( ccu_kps_uart_div_e field, uint32_t val );
static inline uint32_t  ccu_get_kps_uartb3_div( ccu_kps_uart_div_e field );
static inline void      ccu_set_kps_uartb4_div( ccu_kps_uart_div_e field, uint32_t val );
static inline uint32_t  ccu_get_kps_uartb4_div( ccu_kps_uart_div_e field );
static inline void      ccu_set_kps_ssp0_div( ccu_kps_ssp_div_e field, uint32_t val );
static inline uint32_t  ccu_get_kps_ssp0_div( ccu_kps_ssp_div_e field );
static inline void      ccu_set_kps_ssp2_div( ccu_kps_ssp_div_e field, uint32_t val );
static inline uint32_t  ccu_get_kps_ssp2_div( ccu_kps_ssp_div_e field );
static inline void      ccu_set_kps_ssp0_audio_div( ccu_kps_ssp_audio_div_e field, uint32_t val );
static inline uint32_t  ccu_get_kps_ssp0_audio_div( ccu_kps_ssp_audio_div_e field );
static inline void      ccu_set_kps_ssp2_audio_div( ccu_kps_ssp_audio_div_e field, uint32_t val );
static inline uint32_t  ccu_get_kps_ssp2_audio_div( ccu_kps_ssp_audio_div_e field );
static inline void      ccu_set_kps_bsc1_div( ccu_kps_bsc_div_e field, uint32_t val );
static inline uint32_t  ccu_get_kps_bsc1_div( ccu_kps_bsc_div_e field );
static inline void      ccu_set_kps_bsc2_div( ccu_kps_bsc_div_e field, uint32_t val );
static inline uint32_t  ccu_get_kps_bsc2_div( ccu_kps_bsc_div_e field );
static inline void      ccu_set_kps_bsc3_div( ccu_kps_bsc_div_e field, uint32_t val );
static inline uint32_t  ccu_get_kps_bsc3_div( ccu_kps_bsc_div_e field );
static inline void      ccu_set_kps_pwm_div( ccu_kps_pwm_div_e field, uint32_t val );
static inline uint32_t  ccu_get_kps_pwm_div( ccu_kps_pwm_div_e field );
static inline void      ccu_set_kps_timers_div( ccu_kps_timers_div_e field, uint32_t val );
static inline uint32_t  ccu_get_kps_timers_div( ccu_kps_timers_div_e field );
static inline void      ccu_set_kps_div_trig( ccu_kps_div_trig_e field, uint32_t val );
static inline uint32_t  ccu_get_kps_div_trig( ccu_kps_div_trig_e field );
static inline void      ccu_set_kps_clkmon( ccu_kps_clkmon_e field, uint32_t val );
static inline uint32_t  ccu_get_kps_clkmon( ccu_kps_clkmon_e field );





/* Local macros */
#define ccu_set_kps_bit(offset, field, val)  \
            ccu_set_bit( (MM_IO_BASE_SLV_CLK + (offset)), (field), (val) )

#define ccu_get_kps_bit(offset, field)       \
            ccu_get_bit( (MM_IO_BASE_SLV_CLK + (offset)), (field) )

#define ccu_set_kps_reg_field(offset, mask, shift, val)  \
            ccu_set_reg_field( (MM_IO_BASE_SLV_CLK + (offset)), (mask), (shift), (val) );

#define ccu_get_kps_reg_field(offset, mask, shift)       \
            ccu_get_reg_field( (MM_IO_BASE_SLV_CLK + (offset)), (mask), (shift) )



/* Functions */

static inline void ccu_set_kps_peri_volt( ccu_kps_peri_volt_e field, uint32_t val )
{
   uint32_t mask;

   if (field == ccu_kps_peri_volt_normal)
   {
      mask = NFIX(KPS_CLK_MGR_REG_VLT_PERI_VLT_NORMAL_PERI_MASK);
   }
   else if (field == ccu_kps_peri_volt_high)
   {
      mask = NFIX(KPS_CLK_MGR_REG_VLT_PERI_VLT_HIGH_PERI_MASK);
   }
   else
   {
      mask = 1 << field;
   }
   ccu_set_kps_reg_field( NFIX(KPS_CLK_MGR_REG_VLT_PERI_OFFSET), mask, field, val );
}

static inline uint32_t ccu_get_kps_peri_volt( ccu_kps_peri_volt_e field )
{
   uint32_t mask;

   if (field == ccu_kps_peri_volt_normal)
   {
      mask = NFIX(KPS_CLK_MGR_REG_VLT_PERI_VLT_NORMAL_PERI_MASK);
   }
   else if (field == ccu_kps_peri_volt_high)
   {
      mask = NFIX(KPS_CLK_MGR_REG_VLT_PERI_VLT_HIGH_PERI_MASK);
   }
   else
   {
      mask = 1 << field;
   }
   return ccu_get_kps_reg_field( NFIX(KPS_CLK_MGR_REG_VLT_PERI_OFFSET), mask, field );
}

static inline void ccu_set_kps_lvm( ccu_kps_lvm_e field, uint32_t val )
{
   uint32_t shift;
   uint32_t mask;
   uint32_t reg_offset;

   if (field < ccu_kps_lvm_4 )
   {
      reg_offset = NFIX(KPS_CLK_MGR_REG_LVM0_3_OFFSET);
   }
   else
   {
      reg_offset = NFIX(KPS_CLK_MGR_REG_LVM4_7_OFFSET);
      field = field - ccu_kps_lvm_4;
   }

   shift = field * 4;
   mask  = NFIX(KPS_CLK_MGR_REG_LVM0_3_LVM0_3_MD_00_MASK) << (shift);

   ccu_set_kps_reg_field( reg_offset, mask, shift, val );
}

static inline uint32_t ccu_get_kps_lvm( ccu_kps_lvm_e field )
{
   uint32_t shift;
   uint32_t mask;
   uint32_t reg_offset;

   if (field < ccu_kps_lvm_4 )
   {
      reg_offset = NFIX(KPS_CLK_MGR_REG_LVM0_3_OFFSET);
   }
   else
   {
      reg_offset = NFIX(KPS_CLK_MGR_REG_LVM4_7_OFFSET);
      field = field - ccu_kps_lvm_4;
   }

   shift = field * 4;
   mask  = NFIX(KPS_CLK_MGR_REG_LVM0_3_LVM0_3_MD_00_MASK) << (shift);

   return ccu_get_kps_reg_field( reg_offset, mask, shift );
}

static inline void ccu_set_kps_vlt( ccu_kps_vlt_e field, uint32_t val )
{
   uint32_t shift;
   uint32_t mask;
   uint32_t reg_offset;

   if (field < ccu_kps_vlt_4 )
   {
      reg_offset = NFIX(KPS_CLK_MGR_REG_VLT0_3_OFFSET);
   }
   else
   {
      reg_offset = NFIX(KPS_CLK_MGR_REG_VLT4_7_OFFSET);
      field = field - ccu_kps_vlt_4;
   }

   shift = field * 8;
   mask  = NFIX(KPS_CLK_MGR_REG_VLT0_3_VLT0_3_VV_00_MASK) << (shift);

   ccu_set_kps_reg_field( reg_offset, mask, shift, val );
}

static inline uint32_t ccu_get_kps_vlt( ccu_kps_vlt_e field )
{
   uint32_t shift;
   uint32_t mask;
   uint32_t reg_offset;

   if (field < ccu_kps_vlt_4 )
   {
      reg_offset = NFIX(KPS_CLK_MGR_REG_VLT0_3_OFFSET);
   }
   else
   {
      reg_offset = NFIX(KPS_CLK_MGR_REG_VLT4_7_OFFSET);
      field = field - ccu_kps_vlt_4;
   }

   shift = field * 8;
   mask  = NFIX(KPS_CLK_MGR_REG_VLT0_3_VLT0_3_VV_00_MASK) << (shift);

   return ccu_get_kps_reg_field( reg_offset, mask, shift );
}

static inline void ccu_set_kps_bus_quiesc( ccu_kps_bus_quiesc_e field, uint32_t val )
{
   ccu_set_kps_bit( NFIX(KPS_CLK_MGR_REG_BUS_QUIESC_OFFSET), field, val );
}

static inline uint32_t ccu_get_kps_bus_quiesc( ccu_kps_bus_quiesc_e field )
{
   return ccu_get_kps_bit( NFIX(KPS_CLK_MGR_REG_BUS_QUIESC_OFFSET), field );
}

static inline void ccu_set_kps_axi_switch_clkgate( ccu_kps_axi_switch_clkgate_e field, uint32_t val )
{
   ccu_set_kps_bit( NFIX(KPS_CLK_MGR_REG_AXI_SWITCH_CLKGATE_OFFSET), field, val );
}

static inline uint32_t ccu_get_kps_axi_switch_clkgate( ccu_kps_axi_switch_clkgate_e field )
{
   return ccu_get_kps_bit( NFIX(KPS_CLK_MGR_REG_AXI_SWITCH_CLKGATE_OFFSET), field );
}

static inline void ccu_set_kps_axi_ext_clkgate( ccu_kps_axi_ext_clkgate_e field, uint32_t val )
{
   ccu_set_kps_bit( NFIX(KPS_CLK_MGR_REG_AXI_EXT_CLKGATE_OFFSET), field, val );
}

static inline uint32_t ccu_get_kps_axi_ext_clkgate( ccu_kps_axi_ext_clkgate_e field )
{
   return ccu_get_kps_bit( NFIX(KPS_CLK_MGR_REG_AXI_EXT_CLKGATE_OFFSET), field );
}

static inline void ccu_set_kps_ahb1_clkgate( ccu_kps_ahb1_clkgate_e field, uint32_t val )
{
   ccu_set_kps_bit( NFIX(KPS_CLK_MGR_REG_AHB1_CLKGATE_OFFSET), field, val );
}

static inline uint32_t ccu_get_kps_ahb1_clkgate( ccu_kps_ahb1_clkgate_e field )
{
   return ccu_get_kps_bit( NFIX(KPS_CLK_MGR_REG_AHB1_CLKGATE_OFFSET), field );
}

static inline void ccu_set_kps_apb1_clkgate( ccu_kps_apb1_clkgate_e field, uint32_t val )
{
   ccu_set_kps_bit( NFIX(KPS_CLK_MGR_REG_APB1_CLKGATE_OFFSET), field, val );
}

static inline uint32_t ccu_get_kps_apb1_clkgate( ccu_kps_apb1_clkgate_e field )
{
   return ccu_get_kps_bit( NFIX(KPS_CLK_MGR_REG_APB1_CLKGATE_OFFSET), field );
}

static inline void ccu_set_kps_apb2_clkgate( ccu_kps_apb2_clkgate_e field, uint32_t val )
{
   ccu_set_kps_bit( NFIX(KPS_CLK_MGR_REG_APB2_CLKGATE_OFFSET), field, val );
}

static inline uint32_t ccu_get_kps_apb2_clkgate( ccu_kps_apb2_clkgate_e field )
{
   return ccu_get_kps_bit( NFIX(KPS_CLK_MGR_REG_APB2_CLKGATE_OFFSET), field );
}

static inline void ccu_set_kps_apb3_clkgate( ccu_kps_apb3_clkgate_e field, uint32_t val )
{
   ccu_set_kps_bit( NFIX(KPS_CLK_MGR_REG_APB3_CLKGATE_OFFSET), field, val );
}

static inline uint32_t ccu_get_kps_apb3_clkgate( ccu_kps_apb3_clkgate_e field )
{
   return ccu_get_kps_bit( NFIX(KPS_CLK_MGR_REG_APB3_CLKGATE_OFFSET), field );
}

static inline void ccu_set_kps_apb2reg_clkgate( ccu_kps_apb2reg_clkgate_e field, uint32_t val )
{
   ccu_set_kps_bit( NFIX(KPS_CLK_MGR_REG_APB2_REG_CLKGATE_OFFSET), field, val );
}

static inline uint32_t ccu_get_kps_apb2reg_clkgate( ccu_kps_apb2reg_clkgate_e field )
{
   return ccu_get_kps_bit( NFIX(KPS_CLK_MGR_REG_APB2_REG_CLKGATE_OFFSET), field );
}

static inline void ccu_set_kps_spum_sec_clkgate( ccu_kps_spum_sec_clkgate_e field, uint32_t val )
{
   ccu_set_kps_bit( NFIX(KPS_CLK_MGR_REG_SPUM_SEC_CLKGATE_OFFSET), field, val );
}

static inline uint32_t ccu_get_kps_spum_sec_clkgate( ccu_kps_spum_sec_clkgate_e field )
{
   return ccu_get_kps_bit( NFIX(KPS_CLK_MGR_REG_SPUM_SEC_CLKGATE_OFFSET), field );
}

static inline void ccu_set_kps_spum_open_clkgate( ccu_kps_spum_open_clkgate_e field, uint32_t val )
{
   ccu_set_kps_bit( NFIX(KPS_CLK_MGR_REG_SPUM_OPEN_CLKGATE_OFFSET), field, val );
}

static inline uint32_t ccu_get_kps_spum_open_clkgate( ccu_kps_spum_open_clkgate_e field )
{
   return ccu_get_kps_bit( NFIX(KPS_CLK_MGR_REG_SPUM_OPEN_CLKGATE_OFFSET), field );
}

static inline void ccu_set_kps_mphi_clkgate( ccu_kps_mphi_clkgate_e field, uint32_t val )
{
   ccu_set_kps_bit( NFIX(KPS_CLK_MGR_REG_MPHI_CLKGATE_OFFSET), field, val );
}

static inline uint32_t ccu_get_kps_mphi_clkgate( ccu_kps_mphi_clkgate_e field )
{
   return ccu_get_kps_bit( NFIX(KPS_CLK_MGR_REG_MPHI_CLKGATE_OFFSET), field );
}

static inline void ccu_set_kps_uartb_clkgate( ccu_kps_uart_clkgate_e field, uint32_t val )
{
   ccu_set_kps_bit( NFIX(KPS_CLK_MGR_REG_UARTB_CLKGATE_OFFSET), field, val );
}

static inline uint32_t ccu_get_kps_uartb_clkgate( ccu_kps_uart_clkgate_e field )
{
   return ccu_get_kps_bit( NFIX(KPS_CLK_MGR_REG_UARTB_CLKGATE_OFFSET), field );
}

static inline void ccu_set_kps_uartb2_clkgate( ccu_kps_uart_clkgate_e field, uint32_t val )
{
   ccu_set_kps_bit( NFIX(KPS_CLK_MGR_REG_UARTB2_CLKGATE_OFFSET), field, val );
}

static inline uint32_t ccu_get_kps_uartb2_clkgate( ccu_kps_uart_clkgate_e field )
{
   return ccu_get_kps_bit( NFIX(KPS_CLK_MGR_REG_UARTB2_CLKGATE_OFFSET), field );
}

static inline void ccu_set_kps_uartb3_clkgate( ccu_kps_uart_clkgate_e field, uint32_t val )
{
   ccu_set_kps_bit( NFIX(KPS_CLK_MGR_REG_UARTB3_CLKGATE_OFFSET), field, val );
}

static inline uint32_t ccu_get_kps_uartb3_clkgate( ccu_kps_uart_clkgate_e field )
{
   return ccu_get_kps_bit( NFIX(KPS_CLK_MGR_REG_UARTB3_CLKGATE_OFFSET), field );
}

static inline void ccu_set_kps_uartb4_clkgate( ccu_kps_uart_clkgate_e field, uint32_t val )
{
   ccu_set_kps_bit( NFIX(KPS_CLK_MGR_REG_UARTB4_CLKGATE_OFFSET), field, val );
}

static inline uint32_t ccu_get_kps_uartb4_clkgate( ccu_kps_uart_clkgate_e field )
{
   return ccu_get_kps_bit( NFIX(KPS_CLK_MGR_REG_UARTB4_CLKGATE_OFFSET), field );
}

static inline void ccu_set_kps_uartb5_clkgate( ccu_kps_uart_clkgate_e field, uint32_t val )
{
   ccu_set_kps_bit( NFIX(KPS_CLK_MGR_REG_UARTB5_CLKGATE_OFFSET), field, val );
}

static inline uint32_t ccu_get_kps_uartb5_clkgate( ccu_kps_uart_clkgate_e field )
{
   return ccu_get_kps_bit( NFIX(KPS_CLK_MGR_REG_UARTB5_CLKGATE_OFFSET), field );
}

static inline void ccu_set_kps_uartb6_clkgate( ccu_kps_uart_clkgate_e field, uint32_t val )
{
   ccu_set_kps_bit( NFIX(KPS_CLK_MGR_REG_UARTB6_CLKGATE_OFFSET), field, val );
}

static inline uint32_t ccu_get_kps_uartb6_clkgate( ccu_kps_uart_clkgate_e field )
{
   return ccu_get_kps_bit( NFIX(KPS_CLK_MGR_REG_UARTB6_CLKGATE_OFFSET), field );
}

static inline void ccu_set_kps_ssp0_clkgate( ccu_kps_ssp_clkgate_e field, uint32_t val )
{
   ccu_set_kps_bit( NFIX(KPS_CLK_MGR_REG_SSP0_CLKGATE_OFFSET), field, val );
}

static inline uint32_t ccu_get_kps_ssp0_clkgate( ccu_kps_ssp_clkgate_e field )
{
   return ccu_get_kps_bit( NFIX(KPS_CLK_MGR_REG_SSP0_CLKGATE_OFFSET), field );
}

static inline void ccu_set_kps_ssp2_clkgate( ccu_kps_ssp_clkgate_e field, uint32_t val )
{
   ccu_set_kps_bit( NFIX(KPS_CLK_MGR_REG_SSP2_CLKGATE_OFFSET), field, val );
}

static inline uint32_t ccu_get_kps_ssp2_clkgate( ccu_kps_ssp_clkgate_e field )
{
   return ccu_get_kps_bit( NFIX(KPS_CLK_MGR_REG_SSP2_CLKGATE_OFFSET), field );
}

static inline void ccu_set_kps_timers_clkgate( ccu_kps_timers_clkgate_e field, uint32_t val )
{
   ccu_set_kps_bit( NFIX(KPS_CLK_MGR_REG_TIMERS_CLKGATE_OFFSET), field, val );
}

static inline uint32_t ccu_get_kps_timers_clkgate( ccu_kps_timers_clkgate_e field )
{
   return ccu_get_kps_bit( NFIX(KPS_CLK_MGR_REG_TIMERS_CLKGATE_OFFSET), field );
}

static inline void ccu_set_kps_dmac_mux_clkgate( ccu_kps_dmac_mux_clkgate_e field, uint32_t val )
{
   ccu_set_kps_bit( NFIX(KPS_CLK_MGR_REG_DMAC_MUX_CLKGATE_OFFSET), field, val );
}

static inline uint32_t ccu_get_kps_dmac_mux_clkgate( ccu_kps_dmac_mux_clkgate_e field )
{
   return ccu_get_kps_bit( NFIX(KPS_CLK_MGR_REG_DMAC_MUX_CLKGATE_OFFSET), field );
}

static inline void ccu_set_kps_bsc1_clkgate( ccu_kps_bsc_clkgate_e field, uint32_t val )
{
   ccu_set_kps_bit( NFIX(KPS_CLK_MGR_REG_BSC1_CLKGATE_OFFSET), field, val );
}

static inline uint32_t ccu_get_kps_bsc1_clkgate( ccu_kps_bsc_clkgate_e field )
{
   return ccu_get_kps_bit( NFIX(KPS_CLK_MGR_REG_BSC1_CLKGATE_OFFSET), field );
}

static inline void ccu_set_kps_bsc2_clkgate( ccu_kps_bsc_clkgate_e field, uint32_t val )
{
   ccu_set_kps_bit( NFIX(KPS_CLK_MGR_REG_BSC2_CLKGATE_OFFSET), field, val );
}

static inline uint32_t ccu_get_kps_bsc2_clkgate( ccu_kps_bsc_clkgate_e field )
{
   return ccu_get_kps_bit( NFIX(KPS_CLK_MGR_REG_BSC2_CLKGATE_OFFSET), field );
}

static inline void ccu_set_kps_bsc3_clkgate( ccu_kps_bsc_clkgate_e field, uint32_t val )
{
   ccu_set_kps_bit( NFIX(KPS_CLK_MGR_REG_BSC3_CLKGATE_OFFSET), field, val );
}

static inline uint32_t ccu_get_kps_bsc3_clkgate( ccu_kps_bsc_clkgate_e field )
{
   return ccu_get_kps_bit( NFIX(KPS_CLK_MGR_REG_BSC3_CLKGATE_OFFSET), field );
}

static inline void ccu_set_kps_pwm_clkgate( ccu_kps_pwm_clkgate_e field, uint32_t val )
{
   ccu_set_kps_bit( NFIX(KPS_CLK_MGR_REG_PWM_CLKGATE_OFFSET), field, val );
}

static inline uint32_t ccu_get_kps_pwm_clkgate( ccu_kps_pwm_clkgate_e field )
{
   return ccu_get_kps_bit( NFIX(KPS_CLK_MGR_REG_PWM_CLKGATE_OFFSET), field );
}

static inline void ccu_set_kps_bbl_clkgate( ccu_kps_bbl_clkgate_e field, uint32_t val )
{
   ccu_set_kps_bit( NFIX(KPS_CLK_MGR_REG_BBL_CLKGATE_OFFSET), field, val );
}

static inline uint32_t ccu_get_kps_bbl_clkgate( ccu_kps_bbl_clkgate_e field )
{
   return ccu_get_kps_bit( NFIX(KPS_CLK_MGR_REG_BBL_CLKGATE_OFFSET), field );
}

static inline void ccu_set_kps_dap_clkgate( ccu_kps_dap_clkgate_e field, uint32_t val )
{
   ccu_set_kps_bit( NFIX(KPS_CLK_MGR_REG_DAP_CLKGATE_OFFSET), field, val );
}

static inline uint32_t ccu_get_kps_dap_clkgate( ccu_kps_dap_clkgate_e field )
{
   return ccu_get_kps_bit( NFIX(KPS_CLK_MGR_REG_DAP_CLKGATE_OFFSET), field );
}

static inline void ccu_set_kps_cir_clkgate( ccu_kps_cir_clkgate_e field, uint32_t val )
{
   ccu_set_kps_bit( NFIX(KPS_CLK_MGR_REG_CIR_CLKGATE_OFFSET), field, val );
}

static inline uint32_t ccu_get_kps_cir_clkgate( ccu_kps_cir_clkgate_e field )
{
   return ccu_get_kps_bit( NFIX(KPS_CLK_MGR_REG_CIR_CLKGATE_OFFSET), field );
}

static inline void ccu_set_kps_ledm_clkgate( ccu_kps_ledm_clkgate_e field, uint32_t val )
{
   ccu_set_kps_bit( NFIX(KPS_CLK_MGR_REG_LEDM_CLKGATE_OFFSET), field, val );
}

static inline uint32_t ccu_get_kps_ledm_clkgate( ccu_kps_ledm_clkgate_e field )
{
   return ccu_get_kps_bit( NFIX(KPS_CLK_MGR_REG_LEDM_CLKGATE_OFFSET), field );
}

static inline void ccu_set_kps_spum_sec_div( ccu_kps_spum_sec_div_e field, uint32_t val )
{
   uint32_t mask;

   if (field == ccu_kps_spum_sec_div_pll_select)
   {
      mask = NFIX(KPS_CLK_MGR_REG_SPUM_SEC_DIV_SPUM_SEC_PLL_SELECT_MASK);
   }
   else if (field == ccu_kps_spum_sec_div_div)
   {
      mask = NFIX(KPS_CLK_MGR_REG_SPUM_SEC_DIV_SPUM_SEC_DIV_MASK);
   }
   else
   {
      mask = 1 << field;
   }
   ccu_set_kps_reg_field( NFIX(KPS_CLK_MGR_REG_SPUM_SEC_DIV_OFFSET), mask, field, val );
}

static inline uint32_t ccu_get_kps_spum_sec_div( ccu_kps_spum_sec_div_e field )
{
   uint32_t mask;

   if (field == ccu_kps_spum_sec_div_pll_select)
   {
      mask = NFIX(KPS_CLK_MGR_REG_SPUM_SEC_DIV_SPUM_SEC_PLL_SELECT_MASK);
   }
   else if (field == ccu_kps_spum_sec_div_div)
   {
      mask = NFIX(KPS_CLK_MGR_REG_SPUM_SEC_DIV_SPUM_SEC_DIV_MASK);
   }
   else
   {
      mask = 1 << field;
   }
   return ccu_get_kps_reg_field( NFIX(KPS_CLK_MGR_REG_SPUM_SEC_DIV_OFFSET), mask, field );
}

static inline void ccu_set_kps_spum_open_div( ccu_kps_spum_open_div_e field, uint32_t val )
{
   uint32_t mask;

   if (field == ccu_kps_spum_open_div_pll_select)
   {
      mask = NFIX(KPS_CLK_MGR_REG_SPUM_OPEN_DIV_SPUM_OPEN_PLL_SELECT_MASK);
   }
   else if (field == ccu_kps_spum_open_div_div)
   {
      mask = NFIX(KPS_CLK_MGR_REG_SPUM_OPEN_DIV_SPUM_OPEN_DIV_MASK);
   }
   else
   {
      mask = 1 << field;
   }
   ccu_set_kps_reg_field( NFIX(KPS_CLK_MGR_REG_SPUM_OPEN_DIV_OFFSET), mask, field, val );
}

static inline uint32_t ccu_get_kps_spum_open_div( ccu_kps_spum_open_div_e field )
{
   uint32_t mask;

   if (field == ccu_kps_spum_open_div_pll_select)
   {
      mask = NFIX(KPS_CLK_MGR_REG_SPUM_OPEN_DIV_SPUM_OPEN_PLL_SELECT_MASK);
   }
   else if (field == ccu_kps_spum_open_div_div)
   {
      mask = NFIX(KPS_CLK_MGR_REG_SPUM_OPEN_DIV_SPUM_OPEN_DIV_MASK);
   }
   else
   {
      mask = 1 << field;
   }
   return ccu_get_kps_reg_field( NFIX(KPS_CLK_MGR_REG_SPUM_OPEN_DIV_OFFSET), mask, field );
}

static inline void ccu_set_kps_apb7_clkgate( ccu_kps_apb7_clkgate_e field, uint32_t val )
{
   ccu_set_kps_bit( NFIX(KPS_CLK_MGR_REG_APB7_CLKGATE_OFFSET), field, val );
}

static inline uint32_t ccu_get_kps_apb7_clkgate( ccu_kps_apb7_clkgate_e field )
{
   return ccu_get_kps_bit( NFIX(KPS_CLK_MGR_REG_APB7_CLKGATE_OFFSET), field );
}

static inline void ccu_set_kps_apb_spum_sec_clkgate( ccu_kps_apb_spum_sec_clkgate_e field, uint32_t val )
{
   ccu_set_kps_bit( NFIX(KPS_CLK_MGR_REG_SPUM_SEC_APB_CLKGATE_OFFSET), field, val );
}

static inline uint32_t ccu_get_kps_apb_spum_sec_clkgate( ccu_kps_apb_spum_sec_clkgate_e field )
{
   return ccu_get_kps_bit( NFIX(KPS_CLK_MGR_REG_SPUM_SEC_APB_CLKGATE_OFFSET), field );
}

static inline void ccu_set_kps_apb_spum_open_clkgate( ccu_kps_apb_spum_open_clkgate_e field, uint32_t val )
{
   ccu_set_kps_bit( NFIX(KPS_CLK_MGR_REG_SPUM_OPEN_APB_CLKGATE_OFFSET), field, val );
}

static inline uint32_t ccu_get_kps_apb_spum_open_clkgate( ccu_kps_apb_spum_open_clkgate_e field )
{
   return ccu_get_kps_bit( NFIX(KPS_CLK_MGR_REG_SPUM_OPEN_APB_CLKGATE_OFFSET), field );
}

static inline void ccu_set_kps_axi_div( ccu_kps_axi_div_e field, uint32_t val )
{
   uint32_t mask;

   switch( (uint32_t)field )
   {
      case ccu_kps_axi_div_switch_axi_pll_select:
         mask = NFIX(KPS_CLK_MGR_REG_AXI_DIV_SWITCH_AXI_PLL_SELECT_MASK);
         break;

      case ccu_kps_axi_div_switch_axi_div:
         mask = NFIX(KPS_CLK_MGR_REG_AXI_DIV_SWITCH_AXI_DIV_MASK);
         break;

      case ccu_kps_axi_div_apb1_div:
         mask = NFIX(KPS_CLK_MGR_REG_AXI_DIV_APB1_DIV_MASK);
         break;

      case ccu_kps_axi_div_apb2_free_div:
         mask = NFIX(KPS_CLK_MGR_REG_AXI_DIV_APB2_FREE_DIV_MASK);
         break;

      case ccu_kps_axi_div_apb3_div:
         mask = NFIX(KPS_CLK_MGR_REG_AXI_DIV_APB3_DIV_MASK);
         break;
         
      case ccu_kps_axi_div_apb7_div:
      	 mask = NFIX(KPS_CLK_MGR_REG_AXI_DIV_APB7_DIV_MASK);
      	 break;

      default:
         mask = 1 << field;
         break;
   }
   ccu_set_kps_reg_field( NFIX(KPS_CLK_MGR_REG_AXI_DIV_OFFSET), mask, field, val );
}

static inline uint32_t ccu_get_kps_axi_div( ccu_kps_axi_div_e field )
{
   uint32_t mask;

   switch( (uint32_t)field )
   {
      case ccu_kps_axi_div_switch_axi_pll_select:
         mask = NFIX(KPS_CLK_MGR_REG_AXI_DIV_SWITCH_AXI_PLL_SELECT_MASK);
         break;

      case ccu_kps_axi_div_switch_axi_div:
         mask = NFIX(KPS_CLK_MGR_REG_AXI_DIV_SWITCH_AXI_DIV_MASK);
         break;

      case ccu_kps_axi_div_apb1_div:
         mask = NFIX(KPS_CLK_MGR_REG_AXI_DIV_APB1_DIV_MASK);
         break;

      case ccu_kps_axi_div_apb2_free_div:
         mask = NFIX(KPS_CLK_MGR_REG_AXI_DIV_APB2_FREE_DIV_MASK);
         break;

      case ccu_kps_axi_div_apb3_div:
         mask = NFIX(KPS_CLK_MGR_REG_AXI_DIV_APB3_DIV_MASK);
         break;

      case ccu_kps_axi_div_apb7_div:
      	 mask = NFIX(KPS_CLK_MGR_REG_AXI_DIV_APB7_DIV_MASK);
      	 break;

      default:
         mask = 1 << field;
         break;
   }
   return ccu_get_kps_reg_field( NFIX(KPS_CLK_MGR_REG_AXI_DIV_OFFSET), mask, field );
}

static inline void ccu_set_kps_uartb_div( ccu_kps_uart_div_e field, uint32_t val )
{
   uint32_t mask;

   switch( (uint32_t)field )
   {
      case ccu_kps_uart_div_pll_select:
         mask = NFIX(KPS_CLK_MGR_REG_UARTB_DIV_UARTB_PLL_SELECT_MASK);
         break;

      case ccu_kps_uart_div_div:
         mask = NFIX(KPS_CLK_MGR_REG_UARTB_DIV_UARTB_DIV_MASK);
         break;

      default:
         mask = 1 << field;
         break;
   }
   ccu_set_kps_reg_field( NFIX(KPS_CLK_MGR_REG_UARTB_DIV_OFFSET), mask, field, val );
}

static inline uint32_t ccu_get_kps_uartb_div( ccu_kps_uart_div_e field )
{
   uint32_t mask;

   switch( (uint32_t)field )
   {
      case ccu_kps_uart_div_pll_select:
         mask = NFIX(KPS_CLK_MGR_REG_UARTB_DIV_UARTB_PLL_SELECT_MASK);
         break;

      case ccu_kps_uart_div_div:
         mask = NFIX(KPS_CLK_MGR_REG_UARTB_DIV_UARTB_DIV_MASK);
         break;

      default:
         mask = 1 << field;
         break;
   }
   return ccu_get_kps_reg_field( NFIX(KPS_CLK_MGR_REG_UARTB_DIV_OFFSET), mask, field );
}

static inline void ccu_set_kps_uartb2_div( ccu_kps_uart_div_e field, uint32_t val )
{
   uint32_t mask;

   switch( (uint32_t)field )
   {
      case ccu_kps_uart_div_pll_select:
         mask = NFIX(KPS_CLK_MGR_REG_UARTB_DIV_UARTB_PLL_SELECT_MASK);
         break;

      case ccu_kps_uart_div_div:
         mask = NFIX(KPS_CLK_MGR_REG_UARTB_DIV_UARTB_DIV_MASK);
         break;

      default:
         mask = 1 << field;
         break;
   }
   ccu_set_kps_reg_field( NFIX(KPS_CLK_MGR_REG_UARTB2_DIV_OFFSET), mask, field, val );
}

static inline uint32_t ccu_get_kps_uartb2_div( ccu_kps_uart_div_e field )
{
   uint32_t mask;

   switch( (uint32_t)field )
   {
      case ccu_kps_uart_div_pll_select:
         mask = NFIX(KPS_CLK_MGR_REG_UARTB_DIV_UARTB_PLL_SELECT_MASK);
         break;

      case ccu_kps_uart_div_div:
         mask = NFIX(KPS_CLK_MGR_REG_UARTB_DIV_UARTB_DIV_MASK);
         break;

      default:
         mask = 1 << field;
         break;
   }
   return ccu_get_kps_reg_field( NFIX(KPS_CLK_MGR_REG_UARTB2_DIV_OFFSET), mask, field );
}

static inline void ccu_set_kps_uartb3_div( ccu_kps_uart_div_e field, uint32_t val )
{
   uint32_t mask;

   switch( (uint32_t)field )
   {
      case ccu_kps_uart_div_pll_select:
         mask = NFIX(KPS_CLK_MGR_REG_UARTB_DIV_UARTB_PLL_SELECT_MASK);
         break;

      case ccu_kps_uart_div_div:
         mask = NFIX(KPS_CLK_MGR_REG_UARTB_DIV_UARTB_DIV_MASK);
         break;

      default:
         mask = 1 << field;
         break;
   }
   ccu_set_kps_reg_field( NFIX(KPS_CLK_MGR_REG_UARTB3_DIV_OFFSET), mask, field, val );
}

static inline uint32_t ccu_get_kps_uartb3_div( ccu_kps_uart_div_e field )
{
   uint32_t mask;

   switch( (uint32_t)field )
   {
      case ccu_kps_uart_div_pll_select:
         mask = NFIX(KPS_CLK_MGR_REG_UARTB_DIV_UARTB_PLL_SELECT_MASK);
         break;

      case ccu_kps_uart_div_div:
         mask = NFIX(KPS_CLK_MGR_REG_UARTB_DIV_UARTB_DIV_MASK);
         break;

      default:
         mask = 1 << field;
         break;
   }
   return ccu_get_kps_reg_field( NFIX(KPS_CLK_MGR_REG_UARTB3_DIV_OFFSET), mask, field );
}

static inline void ccu_set_kps_uartb4_div( ccu_kps_uart_div_e field, uint32_t val )
{
   uint32_t mask;

   switch( (uint32_t)field )
   {
      case ccu_kps_uart_div_pll_select:
         mask = NFIX(KPS_CLK_MGR_REG_UARTB_DIV_UARTB_PLL_SELECT_MASK);
         break;

      case ccu_kps_uart_div_div:
         mask = NFIX(KPS_CLK_MGR_REG_UARTB_DIV_UARTB_DIV_MASK);
         break;

      default:
         mask = 1 << field;
         break;
   }
   ccu_set_kps_reg_field( NFIX(KPS_CLK_MGR_REG_UARTB4_DIV_OFFSET), mask, field, val );
}

static inline uint32_t ccu_get_kps_uartb4_div( ccu_kps_uart_div_e field )
{
   uint32_t mask;

   switch( (uint32_t)field )
   {
      case ccu_kps_uart_div_pll_select:
         mask = NFIX(KPS_CLK_MGR_REG_UARTB_DIV_UARTB_PLL_SELECT_MASK);
         break;

      case ccu_kps_uart_div_div:
         mask = NFIX(KPS_CLK_MGR_REG_UARTB_DIV_UARTB_DIV_MASK);
         break;

      default:
         mask = 1 << field;
         break;
   }
   return ccu_get_kps_reg_field( NFIX(KPS_CLK_MGR_REG_UARTB4_DIV_OFFSET), mask, field );
}

static inline void ccu_set_kps_ssp0_div( ccu_kps_ssp_div_e field, uint32_t val )
{
   uint32_t mask;

   switch( (uint32_t)field )
   {
      case ccu_kps_ssp_div_pll_select:
         mask = NFIX(KPS_CLK_MGR_REG_SSP0_DIV_SSP0_PLL_SELECT_MASK);
         break;

      case ccu_kps_ssp_div:
         mask = NFIX(KPS_CLK_MGR_REG_SSP0_DIV_SSP0_DIV_MASK);
         break;

      default:
         mask = 1 << field;
         break;
   }
   ccu_set_kps_reg_field( NFIX(KPS_CLK_MGR_REG_SSP0_DIV_OFFSET), mask, field, val );
}

static inline uint32_t ccu_get_kps_ssp0_div( ccu_kps_ssp_div_e field )
{
   uint32_t mask;

   switch( (uint32_t)field )
   {
      case ccu_kps_ssp_div_pll_select:
         mask = NFIX(KPS_CLK_MGR_REG_SSP0_DIV_SSP0_PLL_SELECT_MASK);
         break;

      case ccu_kps_ssp_div:
         mask = NFIX(KPS_CLK_MGR_REG_SSP0_DIV_SSP0_DIV_MASK);
         break;

      default:
         mask = 1 << field;
         break;
   }
   return ccu_get_kps_reg_field( NFIX(KPS_CLK_MGR_REG_SSP0_DIV_OFFSET), mask, field );
}

static inline void ccu_set_kps_ssp2_div( ccu_kps_ssp_div_e field, uint32_t val )
{
   uint32_t mask;

   switch( (uint32_t)field )
   {
      case ccu_kps_ssp_div_pll_select:
         mask = NFIX(KPS_CLK_MGR_REG_SSP0_DIV_SSP0_PLL_SELECT_MASK);
         break;

      case ccu_kps_ssp_div:
         mask = NFIX(KPS_CLK_MGR_REG_SSP0_DIV_SSP0_DIV_MASK);
         break;

      default:
         mask = 1 << field;
         break;
   }
   ccu_set_kps_reg_field( NFIX(KPS_CLK_MGR_REG_SSP2_DIV_OFFSET), mask, field, val );
}

static inline uint32_t ccu_get_kps_ssp2_div( ccu_kps_ssp_div_e field )
{
   uint32_t mask;

   switch( (uint32_t)field )
   {
      case ccu_kps_ssp_div_pll_select:
         mask = NFIX(KPS_CLK_MGR_REG_SSP0_DIV_SSP0_PLL_SELECT_MASK);
         break;

      case ccu_kps_ssp_div:
         mask = NFIX(KPS_CLK_MGR_REG_SSP0_DIV_SSP0_DIV_MASK);
         break;

      default:
         mask = 1 << field;
         break;
   }
   return ccu_get_kps_reg_field( NFIX(KPS_CLK_MGR_REG_SSP2_DIV_OFFSET), mask, field );
}

static inline void ccu_set_kps_ssp0_audio_div( ccu_kps_ssp_audio_div_e field, uint32_t val )
{
   uint32_t mask;

   switch( (uint32_t)field )
   {
      case ccu_kps_ssp_audio_div_pre_pll_sel:
         mask = NFIX(KPS_CLK_MGR_REG_SSP0_AUDIO_DIV_SSP0_AUDIO_PRE_PLL_SELECT_MASK);
         break;

      case ccu_kps_ssp_audio_div_pre_div:
         mask = NFIX(KPS_CLK_MGR_REG_SSP0_AUDIO_DIV_SSP0_AUDIO_PRE_DIV_MASK);
         break;

      case ccu_kps_ssp_audio_div:
         mask = NFIX(KPS_CLK_MGR_REG_SSP0_AUDIO_DIV_SSP0_AUDIO_DIV_MASK);
         break;

      default:
         mask = 1 << field;
         break;
   }
   ccu_set_kps_reg_field( NFIX(KPS_CLK_MGR_REG_SSP0_AUDIO_DIV_OFFSET), mask, field, val );
}

static inline uint32_t ccu_get_kps_ssp0_audio_div( ccu_kps_ssp_audio_div_e field )
{
   uint32_t mask;

   switch( (uint32_t)field )
   {
      case ccu_kps_ssp_audio_div_pre_pll_sel:
         mask = NFIX(KPS_CLK_MGR_REG_SSP0_AUDIO_DIV_SSP0_AUDIO_PRE_PLL_SELECT_MASK);
         break;

      case ccu_kps_ssp_audio_div_pre_div:
         mask = NFIX(KPS_CLK_MGR_REG_SSP0_AUDIO_DIV_SSP0_AUDIO_PRE_DIV_MASK);
         break;

      case ccu_kps_ssp_audio_div:
         mask = NFIX(KPS_CLK_MGR_REG_SSP0_AUDIO_DIV_SSP0_AUDIO_DIV_MASK);
         break;

      default:
         mask = 1 << field;
         break;
   }
   return ccu_get_kps_reg_field( NFIX(KPS_CLK_MGR_REG_SSP0_AUDIO_DIV_OFFSET), mask, field );
}

static inline void ccu_set_kps_ssp2_audio_div( ccu_kps_ssp_audio_div_e field, uint32_t val )
{
   uint32_t mask;

   switch( (uint32_t)field )
   {
      case ccu_kps_ssp_audio_div_pre_pll_sel:
         mask = NFIX(KPS_CLK_MGR_REG_SSP0_AUDIO_DIV_SSP0_AUDIO_PRE_PLL_SELECT_MASK);
         break;

      case ccu_kps_ssp_audio_div_pre_div:
         mask = NFIX(KPS_CLK_MGR_REG_SSP0_AUDIO_DIV_SSP0_AUDIO_PRE_DIV_MASK);
         break;

      case ccu_kps_ssp_audio_div:
         mask = NFIX(KPS_CLK_MGR_REG_SSP0_AUDIO_DIV_SSP0_AUDIO_DIV_MASK);
         break;

      default:
         mask = 1 << field;
         break;
   }
   ccu_set_kps_reg_field( NFIX(KPS_CLK_MGR_REG_SSP2_AUDIO_DIV_OFFSET), mask, field, val );
}

static inline uint32_t ccu_get_kps_ssp2_audio_div( ccu_kps_ssp_audio_div_e field )
{
   uint32_t mask;

   switch( (uint32_t)field )
   {
      case ccu_kps_ssp_audio_div_pre_pll_sel:
         mask = NFIX(KPS_CLK_MGR_REG_SSP0_AUDIO_DIV_SSP0_AUDIO_PRE_PLL_SELECT_MASK);
         break;

      case ccu_kps_ssp_audio_div_pre_div:
         mask = NFIX(KPS_CLK_MGR_REG_SSP0_AUDIO_DIV_SSP0_AUDIO_PRE_DIV_MASK);
         break;

      case ccu_kps_ssp_audio_div:
         mask = NFIX(KPS_CLK_MGR_REG_SSP0_AUDIO_DIV_SSP0_AUDIO_DIV_MASK);
         break;

      default:
         mask = 1 << field;
         break;
   }
   return ccu_get_kps_reg_field( NFIX(KPS_CLK_MGR_REG_SSP2_AUDIO_DIV_OFFSET), mask, field );
}

static inline void ccu_set_kps_bsc1_div( ccu_kps_bsc_div_e field, uint32_t val )
{
   uint32_t mask;

   if (field == ccu_kps_bsc_div_pll_sel)
   {
      mask = NFIX(KPS_CLK_MGR_REG_BSC1_DIV_BSC1_PLL_SELECT_MASK);
   }
   else
   {
      mask = 1 << field;
   }
   ccu_set_kps_reg_field( NFIX(KPS_CLK_MGR_REG_BSC1_DIV_OFFSET), mask, field, val );
}

static inline uint32_t ccu_get_kps_bsc1_div( ccu_kps_bsc_div_e field )
{
   uint32_t mask;

   if (field == ccu_kps_bsc_div_pll_sel)
   {
      mask = NFIX(KPS_CLK_MGR_REG_BSC1_DIV_BSC1_PLL_SELECT_MASK);
   }
   else
   {
      mask = 1 << field;
   }
   return ccu_get_kps_reg_field( NFIX(KPS_CLK_MGR_REG_BSC1_DIV_OFFSET), mask, field );
}

static inline void ccu_set_kps_bsc2_div( ccu_kps_bsc_div_e field, uint32_t val )
{
   uint32_t mask;

   if (field == ccu_kps_bsc_div_pll_sel)
   {
      mask = NFIX(KPS_CLK_MGR_REG_BSC1_DIV_BSC1_PLL_SELECT_MASK);
   }
   else
   {
      mask = 1 << field;
   }
   ccu_set_kps_reg_field( NFIX(KPS_CLK_MGR_REG_BSC2_DIV_OFFSET), mask, field, val );
}

static inline uint32_t ccu_get_kps_bsc2_div( ccu_kps_bsc_div_e field )
{
   uint32_t mask;

   if (field == ccu_kps_bsc_div_pll_sel)
   {
      mask = NFIX(KPS_CLK_MGR_REG_BSC1_DIV_BSC1_PLL_SELECT_MASK);
   }
   else
   {
      mask = 1 << field;
   }
   return ccu_get_kps_reg_field( NFIX(KPS_CLK_MGR_REG_BSC2_DIV_OFFSET), mask, field );
}

static inline void ccu_set_kps_bsc3_div( ccu_kps_bsc_div_e field, uint32_t val )
{
   uint32_t mask;

   if (field == ccu_kps_bsc_div_pll_sel)
   {
      mask = NFIX(KPS_CLK_MGR_REG_BSC3_DIV_BSC3_PLL_SELECT_MASK);
   }
   else
   {
      mask = 1 << field;
   }
   ccu_set_kps_reg_field( NFIX(KPS_CLK_MGR_REG_BSC3_DIV_OFFSET), mask, field, val );
}

static inline uint32_t ccu_get_kps_bsc3_div( ccu_kps_bsc_div_e field )
{
   uint32_t mask;

   if (field == ccu_kps_bsc_div_pll_sel)
   {
      mask = NFIX(KPS_CLK_MGR_REG_BSC3_DIV_BSC3_PLL_SELECT_MASK);
   }
   else
   {
      mask = 1 << field;
   }
   return ccu_get_kps_reg_field( NFIX(KPS_CLK_MGR_REG_BSC3_DIV_OFFSET), mask, field );
}

static inline void ccu_set_kps_pwm_div( ccu_kps_pwm_div_e field, uint32_t val )
{
   uint32_t mask;

   if (field == ccu_kps_pwm_div_pll_sel)
   {
      mask = NFIX(KPS_CLK_MGR_REG_PWM_DIV_PWM_PLL_SELECT_MASK);
   }
   else if (field == ccu_kps_pwm_div)
   {
      mask = NFIX(KPS_CLK_MGR_REG_PWM_DIV_PWM_DIV_MASK);
   }
   else
   {
      mask = 1 << field;
   }
   ccu_set_kps_reg_field( NFIX(KPS_CLK_MGR_REG_PWM_DIV_OFFSET), mask, field, val );
}

static inline uint32_t ccu_get_kps_pwm_div( ccu_kps_pwm_div_e field )
{
   uint32_t mask;

   if (field == ccu_kps_pwm_div_pll_sel)
   {
      mask = NFIX(KPS_CLK_MGR_REG_PWM_DIV_PWM_PLL_SELECT_MASK);
   }
   else if (field == ccu_kps_pwm_div)
   {
      mask = NFIX(KPS_CLK_MGR_REG_PWM_DIV_PWM_DIV_MASK);
   }
   else
   {
      mask = 1 << field;
   }
   return ccu_get_kps_reg_field( NFIX(KPS_CLK_MGR_REG_PWM_DIV_OFFSET), mask, field );
}

static inline void ccu_set_kps_timers_div( ccu_kps_timers_div_e field, uint32_t val )
{
   uint32_t mask;

   if (field == ccu_kps_timers_div_pll_sel)
   {
      mask = NFIX(KPS_CLK_MGR_REG_TIMERS_DIV_TIMERS_PLL_SELECT_MASK);
   }
   else
   {
      mask = 1 << field;
   }
   ccu_set_kps_reg_field( NFIX(KPS_CLK_MGR_REG_TIMERS_DIV_OFFSET), mask, field, val );
}

static inline uint32_t ccu_get_kps_timers_div( ccu_kps_timers_div_e field )
{
   uint32_t mask;

   if (field == ccu_kps_timers_div_pll_sel)
   {
      mask = NFIX(KPS_CLK_MGR_REG_TIMERS_DIV_TIMERS_PLL_SELECT_MASK);
   }
   else
   {
      mask = 1 << field;
   }
   return ccu_get_kps_reg_field( NFIX(KPS_CLK_MGR_REG_TIMERS_DIV_OFFSET), mask, field );
}

static inline void ccu_set_kps_div_trig( ccu_kps_div_trig_e field, uint32_t val )
{
   if (field == ccu_kps_div_trig_switch_axi_trig)
   {
      ccu_set_kps_bit( NFIX(KPS_CLK_MGR_REG_AXI_DIV_OFFSET), field, val ); 
   }
   else
   {
      ccu_set_kps_bit( NFIX(KPS_CLK_MGR_REG_DIV_TRIG_OFFSET), field, val ); 
   }
}

static inline uint32_t ccu_get_kps_div_trig( ccu_kps_div_trig_e field )
{
   if (field == ccu_kps_div_trig_switch_axi_trig)
   {
      return ccu_get_kps_bit( NFIX(KPS_CLK_MGR_REG_AXI_DIV_OFFSET), field ); 
   }
   else
   {
      return ccu_get_kps_bit( NFIX(KPS_CLK_MGR_REG_DIV_TRIG_OFFSET), field ); 
   }
}

static inline void ccu_set_kps_clkmon( ccu_kps_clkmon_e field, uint32_t val )
{
   uint32_t mask;

   if (field == ccu_kps_clkmon_sel)
   {
      mask = NFIX(KPS_CLK_MGR_REG_CLKMON_CLKMON_SEL_MASK);
   }
   else if (field == ccu_kps_clkmon_ctl)
   {
      mask = NFIX(KPS_CLK_MGR_REG_CLKMON_CLKMON_CTL_MASK);
   }
   else
   {
      mask = 1 << field;
   }
   ccu_set_kps_reg_field( NFIX(KPS_CLK_MGR_REG_CLKMON_OFFSET), mask, field, val );
}

static inline uint32_t ccu_get_kps_clkmon( ccu_kps_clkmon_e field )
{
   uint32_t mask;

   if (field == ccu_kps_clkmon_sel)
   {
      mask = NFIX(KPS_CLK_MGR_REG_CLKMON_CLKMON_SEL_MASK);
   }
   else if (field == ccu_kps_clkmon_ctl)
   {
      mask = NFIX(KPS_CLK_MGR_REG_CLKMON_CLKMON_CTL_MASK);
   }
   else
   {
      mask = 1 << field;
   }
   return ccu_get_kps_reg_field( NFIX(KPS_CLK_MGR_REG_CLKMON_OFFSET), mask, field );
}






#ifdef __cplusplus
}
#endif

#endif /* _CCU_KPS_INLINE_H_*/

