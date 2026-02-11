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
* @file  ccu_sdio_inline.c
*
* @brief Clock Control Unit inline functions
*
* @note
*
*******************************************************************************/
#ifndef _CCU_SDIO_INLINE_H_
#define _CCU_SDIO_INLINE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <asm/arch/ccu_inline.h>
#include <asm/arch/ccu_kpm_inline.h>


#define CCU_SDIO_PLL_SELECT_REF_CRYSTAL_CLK          0x0
#define CCU_SDIO_PLL_SELECT_VAR_52M_CLK              0x1
#define CCU_SDIO_PLL_SELECT_REF_52M_CLK              0x2
#define CCU_SDIO_PLL_SELECT_VAR_96M_CLK              0x3
#define CCU_SDIO_PLL_SELECT_REF_96M_CLK              0x4

#define CCU_SDIO_POLICY_SET_POLICY_0                 (0x1 << ccu_policy_0)
#define CCU_SDIO_POLICY_SET_POLICY_1                 (0x1 << ccu_policy_1)
#define CCU_SDIO_POLICY_SET_POLICY_2                 (0x1 << ccu_policy_2)
#define CCU_SDIO_POLICY_SET_POLICY_3                 (0x1 << ccu_policy_3)
#define CCU_SDIO_POLICY_SET_POLICY_ALL               (CCU_SDIO_POLICY_SET_POLICY_0 | \
                                                           CCU_SDIO_POLICY_SET_POLICY_1 | \
                                                           CCU_SDIO_POLICY_SET_POLICY_2 | \
                                                           CCU_SDIO_POLICY_SET_POLICY_3 )


/******************************************************************************
* 
*  Function Name:
*  uint32_t ccu_sdio_init(uint32_t instance,
*                              uint32_t policy_set,
*                              uint32_t pll_select,
*                              uint32_t divider)
* 
*  Description: This function configures clock settings for the SDIO
*               host controller.
* 
*  Parameters:
*               host controller instance
*               set of policies to be enabled
*               clock source
*               clock divider
* 
*  Return:      status
* 
******************************************************************************/
static inline uint32_t ccu_sdio_init(uint32_t instance,
                                          uint32_t policy_set,
                                          uint32_t pll_select,
                                          uint32_t divider)
{
   uint32_t res;
   uint32_t access;

   res = CCU_INLINE_RC_OK;

   /* Enable write access */
   access = ccu_unlock_kpm_clk_mgr();

   /* Clock init for SDIO1  */
   if (instance == 0)
   {
      ccu_cfg_kpm_policy_mask(0, ccu_kpm_policy_mask_sdio1, policy_set & CCU_SDIO_POLICY_SET_POLICY_0);
      ccu_cfg_kpm_policy_mask(1, ccu_kpm_policy_mask_sdio1, policy_set & CCU_SDIO_POLICY_SET_POLICY_1);
      ccu_cfg_kpm_policy_mask(2, ccu_kpm_policy_mask_sdio1, policy_set & CCU_SDIO_POLICY_SET_POLICY_2);
      ccu_cfg_kpm_policy_mask(3, ccu_kpm_policy_mask_sdio1, policy_set & CCU_SDIO_POLICY_SET_POLICY_3);

      /* Set clock source and divider */
      ccu_set_kpm_sdio1_div(ccu_kpm_sdio_div_pll_sel, pll_select);      
      ccu_set_kpm_sdio1_div(ccu_kpm_sdio_div, divider);      

      /* Enable clocks */
      ccu_set_kpm_sdio1_clkgate(ccu_kpm_sdio_clkgate_ahb_hw_sw_gating_sel, 1);
      ccu_set_kpm_sdio1_clkgate(ccu_kpm_sdio_clkgate_hw_sw_gating_sel, 1);
      ccu_set_kpm_sdio1_clkgate(ccu_kpm_sdio_clkgate_ahb_clk_en, 1);
      ccu_set_kpm_sdio1_clkgate(ccu_kpm_sdio_clkgate_clk_en, 1);
      ccu_set_kpm_sdio1_clkgate(ccu_kpm_sdio_clkgate_sleep_clk_en, 1);
      ccu_set_kpm_sdio1_clkgate(ccu_kpm_sdio_clkgate_voltage_level, 0);

      /* Trigger */
      ccu_set_kpm_div_trig(ccu_kpm_div_trig_sdio1_trig, 1);

      /* Wait for clock source switch */
      while (ccu_get_kpm_div_trig(ccu_kpm_div_trig_sdio1_trig) != 0);

      /* Wait for clocks to be turned on */
      while ((ccu_get_kpm_sdio1_clkgate(ccu_kpm_sdio_clkgate_ahb_stprsts) == 0) ||
             (ccu_get_kpm_sdio1_clkgate(ccu_kpm_sdio_clkgate_stprsts) == 0) ||
             (ccu_get_kpm_sdio1_clkgate(ccu_kpm_sdio_clkgate_sleep_stprsts) == 0));
   }

   /* Clock init for SDIO2  */
   else if (instance == 1)
   {
      ccu_cfg_kpm_policy_mask(0, ccu_kpm_policy_mask_sdio2, policy_set & CCU_SDIO_POLICY_SET_POLICY_0);
      ccu_cfg_kpm_policy_mask(1, ccu_kpm_policy_mask_sdio2, policy_set & CCU_SDIO_POLICY_SET_POLICY_1);
      ccu_cfg_kpm_policy_mask(2, ccu_kpm_policy_mask_sdio2, policy_set & CCU_SDIO_POLICY_SET_POLICY_2);
      ccu_cfg_kpm_policy_mask(3, ccu_kpm_policy_mask_sdio2, policy_set & CCU_SDIO_POLICY_SET_POLICY_3);

      /* Set clock source and divider */
      ccu_set_kpm_sdio2_div(ccu_kpm_sdio_div_pll_sel, pll_select);      
      ccu_set_kpm_sdio2_div(ccu_kpm_sdio_div, divider);      

      /* Enable clocks */
      ccu_set_kpm_sdio2_clkgate(ccu_kpm_sdio_clkgate_ahb_hw_sw_gating_sel, 1);
      ccu_set_kpm_sdio2_clkgate(ccu_kpm_sdio_clkgate_hw_sw_gating_sel, 1);
      ccu_set_kpm_sdio2_clkgate(ccu_kpm_sdio_clkgate_ahb_clk_en, 1);
      ccu_set_kpm_sdio2_clkgate(ccu_kpm_sdio_clkgate_clk_en, 1);
      ccu_set_kpm_sdio2_clkgate(ccu_kpm_sdio_clkgate_sleep_clk_en, 1);
      ccu_set_kpm_sdio2_clkgate(ccu_kpm_sdio_clkgate_voltage_level, 0);

      /* Trigger */
      ccu_set_kpm_div_trig(ccu_kpm_div_trig_sdio2_trig, 1);

      /* Wait for clock source switch */
      while (ccu_get_kpm_div_trig(ccu_kpm_div_trig_sdio2_trig) != 0);

      /* Wait for clocks to be turned on */
      while ((ccu_get_kpm_sdio2_clkgate(ccu_kpm_sdio_clkgate_ahb_stprsts) == 0) ||
             (ccu_get_kpm_sdio2_clkgate(ccu_kpm_sdio_clkgate_stprsts) == 0) ||
             (ccu_get_kpm_sdio2_clkgate(ccu_kpm_sdio_clkgate_sleep_stprsts) == 0));
   }

   /* Clock init for SDIO3  */
   else if (instance == 2)
   {
      ccu_cfg_kpm_policy_mask(0, ccu_kpm_policy_mask_sdio3, policy_set & CCU_SDIO_POLICY_SET_POLICY_0);
      ccu_cfg_kpm_policy_mask(1, ccu_kpm_policy_mask_sdio3, policy_set & CCU_SDIO_POLICY_SET_POLICY_1);
      ccu_cfg_kpm_policy_mask(2, ccu_kpm_policy_mask_sdio3, policy_set & CCU_SDIO_POLICY_SET_POLICY_2);
      ccu_cfg_kpm_policy_mask(3, ccu_kpm_policy_mask_sdio3, policy_set & CCU_SDIO_POLICY_SET_POLICY_3);

      /* Set clock source and divider */
      ccu_set_kpm_sdio3_div(ccu_kpm_sdio_div_pll_sel, pll_select);      
      ccu_set_kpm_sdio3_div(ccu_kpm_sdio_div, divider);      

      /* Enable clocks */
      ccu_set_kpm_sdio3_clkgate(ccu_kpm_sdio_clkgate_ahb_hw_sw_gating_sel, 1);
      ccu_set_kpm_sdio3_clkgate(ccu_kpm_sdio_clkgate_hw_sw_gating_sel, 1);
      ccu_set_kpm_sdio3_clkgate(ccu_kpm_sdio_clkgate_ahb_clk_en, 1);
      ccu_set_kpm_sdio3_clkgate(ccu_kpm_sdio_clkgate_clk_en, 1);
      ccu_set_kpm_sdio3_clkgate(ccu_kpm_sdio_clkgate_sleep_clk_en, 1);
      ccu_set_kpm_sdio3_clkgate(ccu_kpm_sdio_clkgate_voltage_level, 0);

      /* Trigger */
      ccu_set_kpm_div_trig(ccu_kpm_div_trig_sdio3_trig, 1);

      /* Wait for clock source switch */
      while (ccu_get_kpm_div_trig(ccu_kpm_div_trig_sdio3_trig) != 0);

      /* Wait for clocks to be turned on */
      while ((ccu_get_kpm_sdio3_clkgate(ccu_kpm_sdio_clkgate_ahb_stprsts) == 0) ||
             (ccu_get_kpm_sdio3_clkgate(ccu_kpm_sdio_clkgate_stprsts) == 0) ||
             (ccu_get_kpm_sdio3_clkgate(ccu_kpm_sdio_clkgate_sleep_stprsts) == 0));
   }

   /* Clock init for SDIO4  */
   else if (instance == 3)
   {
      ccu_cfg_kpm_policy_mask(0, ccu_kpm_policy_mask_sdio4, policy_set & CCU_SDIO_POLICY_SET_POLICY_0);
      ccu_cfg_kpm_policy_mask(1, ccu_kpm_policy_mask_sdio4, policy_set & CCU_SDIO_POLICY_SET_POLICY_1);
      ccu_cfg_kpm_policy_mask(2, ccu_kpm_policy_mask_sdio4, policy_set & CCU_SDIO_POLICY_SET_POLICY_2);
      ccu_cfg_kpm_policy_mask(3, ccu_kpm_policy_mask_sdio4, policy_set & CCU_SDIO_POLICY_SET_POLICY_3);

      /* Set clock source and divider */
      ccu_set_kpm_sdio4_div(ccu_kpm_sdio_div_pll_sel, pll_select);      
      ccu_set_kpm_sdio4_div(ccu_kpm_sdio_div, divider);      

      /* Enable clocks */
      ccu_set_kpm_sdio4_clkgate(ccu_kpm_sdio_clkgate_ahb_hw_sw_gating_sel, 1);
      ccu_set_kpm_sdio4_clkgate(ccu_kpm_sdio_clkgate_hw_sw_gating_sel, 1);
      ccu_set_kpm_sdio4_clkgate(ccu_kpm_sdio_clkgate_ahb_clk_en, 1);
      ccu_set_kpm_sdio4_clkgate(ccu_kpm_sdio_clkgate_clk_en, 1);
      ccu_set_kpm_sdio4_clkgate(ccu_kpm_sdio_clkgate_sleep_clk_en, 1);
      ccu_set_kpm_sdio4_clkgate(ccu_kpm_sdio_clkgate_voltage_level, 0);

      /* Trigger */
      ccu_set_kpm_div_trig(ccu_kpm_div_trig_sdio4_trig, 1);

      /* Wait for clock source switch */
      while (ccu_get_kpm_div_trig(ccu_kpm_div_trig_sdio4_trig) != 0);

      /* Wait for clocks to be turned on */
      while ((ccu_get_kpm_sdio4_clkgate(ccu_kpm_sdio_clkgate_ahb_stprsts) == 0) ||
             (ccu_get_kpm_sdio4_clkgate(ccu_kpm_sdio_clkgate_stprsts) == 0) ||
             (ccu_get_kpm_sdio4_clkgate(ccu_kpm_sdio_clkgate_sleep_stprsts) == 0));
   }
   else
   {
      /* wrong instance */
      res = CCU_INLINE_RC_FAIL;
   }

   /* Restore write access status*/
   ccu_restore_kpm_clk_mgr(access);

   return res;
}

#ifdef __cplusplus
}
#endif

#endif /*_CCU_SDIO_INLINE_H_*/
