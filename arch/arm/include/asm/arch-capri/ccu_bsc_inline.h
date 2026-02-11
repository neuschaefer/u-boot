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
* @file  ccu_bsc_inline.c
*
* @brief Clock Control Unit inline functions
*
* @note
*
*******************************************************************************/
#ifndef _CCU_BSC_INLINE_H_
#define _CCU_BSC_INLINE_H_

#ifdef __cplusplus
extern "C" {
#endif

//#include <mach/chal/reg.h>
//#include <mach/csp/delay.h>
#include <asm/arch/ccu_khubaon_inline.h>
#include <asm/arch/ccu_kps_inline.h>
#include <asm/arch/brcm_rdb_i2c_mm_hs.h>

#define  CCU_BSC_WAIT_FOR_BIT_TIMEOUT   200


/* BSC DIV */
typedef enum
{
   ccu_bsc_pll_sel_ref_xtal_clk = 0,
   ccu_bsc_pll_sel_var_104m_clk,
   ccu_bsc_pll_sel_ref_104m_clk,
   ccu_bsc_pll_sel_var_13m_clk,
   ccu_bsc_pll_sel_ref_13m_clk,
   ccu_bsc_pll_sel_not_supported
} ccu_bsc_pll_sel_e;

typedef enum
{
   ccu_pmu_bsc_pll_sel_xtal_clk = 0,
   ccu_pmu_bsc_pll_sel_var_clk,
   ccu_pmu_bsc_pll_sel_bbl_32k_clk,
   ccu_pmu_bsc_pll_sel_not_supported
} ccu_pmu_bsc_pll_sel_e;

typedef enum
{
   ccu_bsc_1 = 0,
   ccu_bsc_2,
   ccu_bsc_3,
   ccu_bsc_pmu,
   ccu_bsc_not_supported
} ccu_bsc_e;

typedef enum
{
   ccu_bsc_speed_sel_13m,
   ccu_bsc_speed_sel_104m,
   ccu_bsc_speed_sel_not_supported
} ccu_bsc_speed_sel_e;

/* Function prototypes */
static inline void   ccu_bsc_init( ccu_bsc_e instance );
static inline void   ccu_bsc_set_speed( ccu_bsc_e instance, ccu_bsc_speed_sel_e speed );

/* Functions */

static inline void   ccu_bsc_init( ccu_bsc_e instance )
{
   uint32_t kps_access;
   uint32_t khubaon_access;

   /* Unlock clock manager registers */
   kps_access = ccu_unlock_kps_clk_mgr();

   switch( instance )
   {
      case ccu_bsc_1:

         /* Configure policy mask */
         ccu_set_kps_policy_mask( ccu_policy_0, ccu_kps_policy_mask_bsc1 );
         ccu_set_kps_policy_mask( ccu_policy_1, ccu_kps_policy_mask_bsc1 );
         ccu_set_kps_policy_mask( ccu_policy_2, ccu_kps_policy_mask_bsc1 );
         ccu_set_kps_policy_mask( ccu_policy_3, ccu_kps_policy_mask_bsc1 );

         /* Configure clock gate for BSC1 */
         ccu_set_kps_bsc1_clkgate( ccu_kps_bsc_clkgate_apb_hw_sw_gating_sel, 1 );
         ccu_set_kps_bsc1_clkgate( ccu_kps_bsc_clkgate_apb_clk_en, 1 );

         /* HWCAPRI-1103.  SW workaround would be to first write CLKEN[0] in BSC block,
            and then enable BSC clock in CCU - this will ensure that the clockgate on i2c_clk
            does not see any timing violation */
         CHAL_REG_SETBIT32( MM_IO_BASE_BSC1, 1 << I2C_MM_HS_CLKEN_CLKEN_SHIFT );

         ccu_set_kps_bsc1_clkgate( ccu_kps_bsc_clkgate_apb_hyst_val, 1 );
         ccu_set_kps_bsc1_clkgate( ccu_kps_bsc_clkgate_apb_hyst_en, 1 );
         ccu_set_kps_bsc1_clkgate( ccu_kps_bsc_clkgate_hw_sw_gating_sel, 1 );
         ccu_set_kps_bsc1_clkgate( ccu_kps_bsc_clkgate_clk_en, 1 );
         break;

      case ccu_bsc_2:
         /* Configure policy mask */
         ccu_set_kps_policy_mask( ccu_policy_0, ccu_kps_policy_mask_bsc2 );
         ccu_set_kps_policy_mask( ccu_policy_1, ccu_kps_policy_mask_bsc2 );
         ccu_set_kps_policy_mask( ccu_policy_2, ccu_kps_policy_mask_bsc2 );
         ccu_set_kps_policy_mask( ccu_policy_3, ccu_kps_policy_mask_bsc2 );

         /* Configure clock gate for BSC2 */
         ccu_set_kps_bsc2_clkgate( ccu_kps_bsc_clkgate_apb_hw_sw_gating_sel, 1 );
         ccu_set_kps_bsc2_clkgate( ccu_kps_bsc_clkgate_apb_clk_en, 1 );

         /* HWCAPRI-1103.  SW workaround would be to first write CLKEN[0] in BSC block,
            and then enable BSC clock in CCU - this will ensure that the clockgate on i2c_clk
            does not see any timing violation */
         CHAL_REG_SETBIT32( MM_IO_BASE_BSC2, 1 << I2C_MM_HS_CLKEN_CLKEN_SHIFT );

         ccu_set_kps_bsc2_clkgate( ccu_kps_bsc_clkgate_apb_hyst_val, 1 );
         ccu_set_kps_bsc2_clkgate( ccu_kps_bsc_clkgate_apb_hyst_en, 1 );
         ccu_set_kps_bsc2_clkgate( ccu_kps_bsc_clkgate_hw_sw_gating_sel, 1 );
         ccu_set_kps_bsc2_clkgate( ccu_kps_bsc_clkgate_clk_en, 1 );
         break;

      case ccu_bsc_3:
         /* Configure policy mask */
         ccu_set_kps_policy_mask( ccu_policy_0, ccu_kps_policy_mask_bsc3 );
         ccu_set_kps_policy_mask( ccu_policy_1, ccu_kps_policy_mask_bsc3 );
         ccu_set_kps_policy_mask( ccu_policy_2, ccu_kps_policy_mask_bsc3 );
         ccu_set_kps_policy_mask( ccu_policy_3, ccu_kps_policy_mask_bsc3 );

         /* Configure clock gate for BSC3 */
         ccu_set_kps_bsc3_clkgate( ccu_kps_bsc_clkgate_apb_hw_sw_gating_sel, 1 );
         ccu_set_kps_bsc3_clkgate( ccu_kps_bsc_clkgate_apb_clk_en, 1 );

         /* HWCAPRI-1103.  SW workaround would be to first write CLKEN[0] in BSC block,
            and then enable BSC clock in CCU - this will ensure that the clockgate on i2c_clk
            does not see any timing violation */
         CHAL_REG_SETBIT32( MM_IO_BASE_BSC3, 1 << I2C_MM_HS_CLKEN_CLKEN_SHIFT );

         ccu_set_kps_bsc3_clkgate( ccu_kps_bsc_clkgate_apb_hyst_val, 1 );
         ccu_set_kps_bsc3_clkgate( ccu_kps_bsc_clkgate_apb_hyst_en, 1 );
         ccu_set_kps_bsc3_clkgate( ccu_kps_bsc_clkgate_hw_sw_gating_sel, 1 );
         ccu_set_kps_bsc3_clkgate( ccu_kps_bsc_clkgate_clk_en, 1 );
         break;
         
      case ccu_bsc_pmu:
         /* Configure policy mask */
         ccu_set_khubaon_policy_mask( ccu_policy_0, ccu_khubaon_policy_mask_pmu_bsc );
         ccu_set_khubaon_policy_mask( ccu_policy_1, ccu_khubaon_policy_mask_pmu_bsc );
         ccu_set_khubaon_policy_mask( ccu_policy_2, ccu_khubaon_policy_mask_pmu_bsc );
         ccu_set_khubaon_policy_mask( ccu_policy_3, ccu_khubaon_policy_mask_pmu_bsc );

         /* Unlock CCU registers in the khubaon_clk_mgr_reg block */
         khubaon_access = ccu_unlock_khubaon_clk_mgr();

         /* Configure clock gate for BSC3 */
         ccu_set_khubaon_pmubsc_clkgate( ccu_khubaon_hubtmr_clkgate_apb_hyst_en, 1 );
         ccu_set_khubaon_pmubsc_clkgate( ccu_khubaon_hubtmr_clkgate_apb_hyst_val, 1 );
         ccu_set_khubaon_pmubsc_clkgate( ccu_khubaon_hubtmr_clkgate_hyst_en, 1 );
         ccu_set_khubaon_pmubsc_clkgate( ccu_khubaon_hubtmr_clkgate_hyst_val, 1 );
         ccu_set_khubaon_pmubsc_clkgate( ccu_khubaon_hubtmr_clkgate_apb_hw_sw_gating_sel, 1 );
         ccu_set_khubaon_pmubsc_clkgate( ccu_khubaon_hubtmr_clkgate_apb_clk_en, 1 );
         ccu_set_khubaon_pmubsc_clkgate( ccu_khubaon_hubtmr_clkgate_hw_sw_gating_sel, 1 );
         ccu_set_khubaon_pmubsc_clkgate( ccu_khubaon_hubtmr_clkgate_clk_en, 1 );

         /* Restore the register block access enable */
         ccu_restore_khubaon_clk_mgr(khubaon_access);
         break;

      default:
         /* NOT SUPPORTED */
         break;
   }

   /* Restore previous kps_clk_mgr_reg block access enable */
   ccu_restore_kps_clk_mgr(kps_access);
}

static inline void   ccu_bsc_set_speed( ccu_bsc_e instance, ccu_bsc_speed_sel_e speed )
{
   uint32_t khubaon_access;
   uint32_t kps_access;
   ccu_bsc_pll_sel_e pll_sel;

   if ( speed == ccu_bsc_speed_sel_13m )
   {
      pll_sel = ccu_bsc_pll_sel_var_13m_clk;
   }
   else if ( speed == ccu_bsc_speed_sel_104m )
   {
      pll_sel = ccu_bsc_pll_sel_var_104m_clk;
   }
   else
   {
      return;
   }

   /* Unlock clock manager registers */
   kps_access = ccu_unlock_kps_clk_mgr();

   switch( instance )
   {
      case ccu_bsc_1:
         /* Set incoming clock speed */
         ccu_set_kps_bsc1_div( ccu_kps_bsc_div_pll_sel, pll_sel );

         /* Trigger the new clock frequency */
         ccu_set_kps_div_trig( ccu_kps_div_trig_bsc1_trig, 1 );
         break;

      case ccu_bsc_2:
         /* Set incoming clock speed */
         ccu_set_kps_bsc2_div( ccu_kps_bsc_div_pll_sel, pll_sel );

         /* Trigger the new clock frequency */
         ccu_set_kps_div_trig( ccu_kps_div_trig_bsc2_trig, 1 );
         break;

      case ccu_bsc_3:
         /* Set incoming clock speed */
         ccu_set_kps_bsc3_div( ccu_kps_bsc_div_pll_sel, pll_sel );

         /* Trigger the new clock frequency */
         ccu_set_kps_div_trig( ccu_kps_div_trig_bsc3_trig, 1 );
         break;
         
      case ccu_bsc_pmu:
         /* Unlock clock manager registers */
         khubaon_access = ccu_unlock_khubaon_clk_mgr();

         /* Configure pmu_bsc_clk */
         ccu_set_khubaon_pmubsc_div( ccu_khubaon_pmubsc_div_pll_select,
                                          KHUBAON_CLK_MGR_REG_PMU_BSC_DIV_PMU_BSC_PLL_SELECT_CMD_PMU_BSC_VAR_CLK );
         ccu_set_khubaon_pmubsc_div( ccu_khubaon_pmubsc_div_div, 0 );

         /* Trigger the new clock frequency */
         ccu_set_khubaon_seg_trigger( ccu_khubaon_segtrg_pmubsc_trigger, 1 );

         /* Configure pmu_bsc_var_clk */
         ccu_set_khubaon_async_prediv( ccu_khubaon_async_prediv_pll_select,
                                            KHUBAON_CLK_MGR_REG_ASYNC_PRE_DIV_ASYNC_PRE_PLL_SELECT_CMD_VAR_312M_CLK );

         if ( speed == ccu_bsc_speed_sel_104m )
         {
            /* Set incoming clock to 104MHz */
            ccu_set_khubaon_async_prediv( ccu_khubaon_async_prediv_div, 2 );
         }
         else
         {
            /* Set incoming clock to 13MHz */
            ccu_set_khubaon_async_prediv( ccu_khubaon_async_prediv_div, 23 );
         }

         /* Trigger the new clock frequency */
         ccu_set_khubaon_seg_trigger( ccu_khubaon_segtrg_async_pre_trigger, 1 );

         /* Restore clock manager registers in the khubaon_clk_mgr_reg block (BSC3) */
         ccu_restore_khubaon_clk_mgr(khubaon_access);
         break;

      default:
         /* NOT SUPPORTED */
         break;
   }

   /* Restore clock manager registers access mode in the kps_clk_mgr_reg block */
   ccu_restore_kps_clk_mgr(kps_access);
}

#ifdef __cplusplus
}
#endif

#endif /*_CCU_BSC_INLINE_H_*/

