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
* @file  ccu_esub_inline.c
*
* @brief Ethernet Subsystem Clock Control Unit inline functions
*
* @note
*
*******************************************************************************/
#ifndef _CCU_ESUB_INLINE_H_
#define _CCU_ESUB_INLINE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <asm/arch/ccu_inline.h>
#include <asm/arch/ccu_util_inline.h>
#include <asm/arch/brcm_rdb_esub_clk_mgr_reg.h>


/* type definitions */

typedef enum
{
   ccu_esub_peri_volt_normal = ESUB_CLK_MGR_REG_VLT_PERI_VLT_NORMAL_PERI_SHIFT,
   ccu_esub_peri_volt_high   = ESUB_CLK_MGR_REG_VLT_PERI_VLT_HIGH_PERI_SHIFT

} ccu_esub_peri_volt_e;

typedef enum
{
   ccu_esub_lvm_0 = 0,
   ccu_esub_lvm_1,
   ccu_esub_lvm_2,
   ccu_esub_lvm_3,
   ccu_esub_lvm_4,
   ccu_esub_lvm_5,
   ccu_esub_lvm_6,
   ccu_esub_lvm_7

} ccu_esub_lvm_e;

typedef enum
{
   ccu_esub_vlt_0 = 0,
   ccu_esub_vlt_1,
   ccu_esub_vlt_2,
   ccu_esub_vlt_3,
   ccu_esub_vlt_4,
   ccu_esub_vlt_5,
   ccu_esub_vlt_6,
   ccu_esub_vlt_7

} ccu_esub_vlt_e;

typedef enum
{
   ccu_esub_axi_clkgate_esub_axi_clk_en            = ESUB_CLK_MGR_REG_ESUB_AXI_CLKGATE_ESUB_AXI_CLK_EN_SHIFT,
   ccu_esub_axi_clkgate_esub_axi_hw_sw_gating_sel  = ESUB_CLK_MGR_REG_ESUB_AXI_CLKGATE_ESUB_AXI_HW_SW_GATING_SEL_SHIFT,
   ccu_esub_axi_clkgate_esub_axi_hyst_val          = ESUB_CLK_MGR_REG_ESUB_AXI_CLKGATE_ESUB_AXI_HYST_VAL_SHIFT,
   ccu_esub_axi_clkgate_esub_axi_hyst_en           = ESUB_CLK_MGR_REG_ESUB_AXI_CLKGATE_ESUB_AXI_HYST_EN_SHIFT,
   ccu_esub_axi_clkgate_esub_axi_stprsts           = ESUB_CLK_MGR_REG_ESUB_AXI_CLKGATE_ESUB_AXI_STPRSTS_SHIFT,
   ccu_esub_axi_clkgate_esub_voltage_level         = ESUB_CLK_MGR_REG_ESUB_AXI_CLKGATE_ESUB_VOLTAGE_LEVEL_SHIFT

} ccu_esub_axi_clkgate_e;

typedef enum
{
   ccu_esub_apb_clkgate_esub_apb_clk_en            = ESUB_CLK_MGR_REG_ESUB_APB_CLKGATE_ESUB_APB_CLK_EN_SHIFT,
   ccu_esub_apb_clkgate_esub_apb_hw_sw_gating_sel  = ESUB_CLK_MGR_REG_ESUB_APB_CLKGATE_ESUB_APB_HW_SW_GATING_SEL_SHIFT,
   ccu_esub_apb_clkgate_esub_apb_hyst_val          = ESUB_CLK_MGR_REG_ESUB_APB_CLKGATE_ESUB_APB_HYST_VAL_SHIFT,
   ccu_esub_apb_clkgate_esub_apb_hyst_en           = ESUB_CLK_MGR_REG_ESUB_APB_CLKGATE_ESUB_APB_HYST_EN_SHIFT,
   ccu_esub_apb_clkgate_esub_apb_stprsts           = ESUB_CLK_MGR_REG_ESUB_APB_CLKGATE_ESUB_APB_STPRSTS_SHIFT

} ccu_esub_apb_clkgate_e;

typedef enum
{
   ccu_esub_esw_sys_125m_clkgate_esw_sys_125m_clk_en            = ESUB_CLK_MGR_REG_ESW_SYS_125M_CLKGATE_ESW_SYS_125M_CLK_EN_SHIFT,
   ccu_esub_esw_sys_125m_clkgate_esw_sys_125m_hw_sw_gating_sel	= ESUB_CLK_MGR_REG_ESW_SYS_125M_CLKGATE_ESW_SYS_125M_HW_SW_GATING_SEL_SHIFT,
   ccu_esub_esw_sys_125m_clkgate_esw_sys_125m_hyst_val          = ESUB_CLK_MGR_REG_ESW_SYS_125M_CLKGATE_ESW_SYS_125M_HYST_VAL_SHIFT,
   ccu_esub_esw_sys_125m_clkgate_esw_sys_125m_hyst_en           = ESUB_CLK_MGR_REG_ESW_SYS_125M_CLKGATE_ESW_SYS_125M_HYST_EN_SHIFT,
   ccu_esub_esw_sys_125m_clkgate_esw_sys_125m_stprsts           = ESUB_CLK_MGR_REG_ESW_SYS_125M_CLKGATE_ESW_SYS_125M_STPRSTS_SHIFT,
   ccu_esub_esw_sys_125m_clkgate_esw_sys_125m_voltage_level     = ESUB_CLK_MGR_REG_ESW_SYS_125M_CLKGATE_ESW_VOLTAGE_LEVEL_SHIFT

} ccu_esub_esw_sys_125m_clkgate_e;

typedef enum
{
   ccu_esub_esw_25m_clkgate_esw_25m_clk_en            = ESUB_CLK_MGR_REG_ESW_25M_CLKGATE_ESW_25M_CLK_EN_SHIFT,
   ccu_esub_esw_25m_clkgate_esw_25m_hw_sw_gating_sel  = ESUB_CLK_MGR_REG_ESW_25M_CLKGATE_ESW_25M_HW_SW_GATING_SEL_SHIFT,
   ccu_esub_esw_25m_clkgate_esw_25m_hyst_val          = ESUB_CLK_MGR_REG_ESW_25M_CLKGATE_ESW_25M_HYST_VAL_SHIFT,
   ccu_esub_esw_25m_clkgate_esw_25m_hyst_en           = ESUB_CLK_MGR_REG_ESW_25M_CLKGATE_ESW_25M_HYST_EN_SHIFT,
   ccu_esub_esw_25m_clkgate_esw_25m_stprsts           = ESUB_CLK_MGR_REG_ESW_25M_CLKGATE_ESW_25M_STPRSTS_SHIFT

} ccu_esub_esw_25m_clkgate_e;

typedef enum
{
   ccu_esub_esw_sys_clkgate_esw_sys_clk_en            = ESUB_CLK_MGR_REG_ESW_SYS_CLKGATE_ESW_SYS_CLK_EN_SHIFT,
   ccu_esub_esw_sys_clkgate_esw_sys_hw_sw_gating_sel  = ESUB_CLK_MGR_REG_ESW_SYS_CLKGATE_ESW_SYS_HW_SW_GATING_SEL_SHIFT,
   ccu_esub_esw_sys_clkgate_esw_sys_hyst_val          = ESUB_CLK_MGR_REG_ESW_SYS_CLKGATE_ESW_SYS_HYST_VAL_SHIFT,
   ccu_esub_esw_sys_clkgate_esw_sys_hyst_en           = ESUB_CLK_MGR_REG_ESW_SYS_CLKGATE_ESW_SYS_HYST_EN_SHIFT,
   ccu_esub_esw_sys_clkgate_esw_sys_stprsts           = ESUB_CLK_MGR_REG_ESW_SYS_CLKGATE_ESW_SYS_STPRSTS_SHIFT,
   ccu_esub_esw_sys_clkgate_esw_sys_voltage_level     = ESUB_CLK_MGR_REG_ESW_SYS_CLKGATE_ESW_SYS_VOLTAGE_LEVEL_SHIFT

} ccu_esub_esw_sys_clkgate_e;

typedef enum
{
   ccu_esub_eav_clkgate_axi_clk_en            = ESUB_CLK_MGR_REG_EAV_CLKGATE_EAV_AXI_CLK_EN_SHIFT,
   ccu_esub_eav_clkgate_axi_hw_sw_gating_sel  = ESUB_CLK_MGR_REG_EAV_CLKGATE_EAV_AXI_HW_SW_GATING_SEL_SHIFT,
   ccu_esub_eav_clkgate_apb_clk_en            = ESUB_CLK_MGR_REG_EAV_CLKGATE_EAV_APB_CLK_EN_SHIFT,
   ccu_esub_eav_clkgate_apb_hw_sw_gating_sel  = ESUB_CLK_MGR_REG_EAV_CLKGATE_EAV_APB_HW_SW_GATING_SEL_SHIFT,
   ccu_esub_eav_clkgate_sys_clk_en            = ESUB_CLK_MGR_REG_EAV_CLKGATE_EAV_SYS_CLK_EN_SHIFT,
   ccu_esub_eav_clkgate_sys_hw_sw_gating_sel  = ESUB_CLK_MGR_REG_EAV_CLKGATE_EAV_SYS_HW_SW_GATING_SEL_SHIFT,
   ccu_esub_eav_clkgate_125m_clk_en           = ESUB_CLK_MGR_REG_EAV_CLKGATE_EAV_125M_CLK_EN_SHIFT,
   ccu_esub_eav_clkgate_125m_hw_sw_gating_sel = ESUB_CLK_MGR_REG_EAV_CLKGATE_EAV_125M_HW_SW_GATING_SEL_SHIFT,
   ccu_esub_eav_clkgate_axi_hyst_val          = ESUB_CLK_MGR_REG_EAV_CLKGATE_EAV_AXI_HYST_VAL_SHIFT,
   ccu_esub_eav_clkgate_axi_hyst_en           = ESUB_CLK_MGR_REG_EAV_CLKGATE_EAV_AXI_HYST_EN_SHIFT,
   ccu_esub_eav_clkgate_apb_hyst_val          = ESUB_CLK_MGR_REG_EAV_CLKGATE_EAV_APB_HYST_VAL_SHIFT,
   ccu_esub_eav_clkgate_apb_hyst_en           = ESUB_CLK_MGR_REG_EAV_CLKGATE_EAV_APB_HYST_EN_SHIFT,
   ccu_esub_eav_clkgate_sys_hyst_val          = ESUB_CLK_MGR_REG_EAV_CLKGATE_EAV_SYS_HYST_VAL_SHIFT,
   ccu_esub_eav_clkgate_sys_hyst_en           = ESUB_CLK_MGR_REG_EAV_CLKGATE_EAV_SYS_HYST_EN_SHIFT,
   ccu_esub_eav_clkgate_125m_hyst_val         = ESUB_CLK_MGR_REG_EAV_CLKGATE_EAV_125M_HYST_VAL_SHIFT,
   ccu_esub_eav_clkgate_125m_hyst_en          = ESUB_CLK_MGR_REG_EAV_CLKGATE_EAV_125M_HYST_EN_SHIFT,
   ccu_esub_eav_clkgate_voltage_level         = ESUB_CLK_MGR_REG_EAV_CLKGATE_EAV_VOLTAGE_LEVEL_SHIFT

} ccu_esub_eav_clkgate_e;


typedef enum
{
   ccu_esub_esw_sys_div_override = ESUB_CLK_MGR_REG_ESW_SYS_DIV_ESW_SYS_TRIGGER_SHIFT,
   ccu_esub_esw_sys_div          = ESUB_CLK_MGR_REG_ESW_SYS_DIV_ESW_SYS_DIV_SHIFT,
   ccu_esub_esw_sys_pll_sel      = ESUB_CLK_MGR_REG_ESW_SYS_DIV_ESW_SYS_PLL_SELECT_SHIFT

} ccu_esub_esw_sys_div_e;

/* Function Prototypes */

static inline void      ccu_set_esub_peri_volt( ccu_esub_peri_volt_e field, uint32_t val );
static inline uint32_t  ccu_get_esub_peri_volt( ccu_esub_peri_volt_e field );
static inline void      ccu_set_esub_lvm( ccu_esub_lvm_e field, uint32_t val );
static inline uint32_t  ccu_get_esub_lvm( ccu_esub_lvm_e field );
static inline void      ccu_set_esub_vlt( ccu_esub_vlt_e field, uint32_t val );
static inline uint32_t  ccu_get_esub_vlt( ccu_esub_vlt_e field );

static inline void      ccu_set_esub_axi_clkgate( ccu_esub_axi_clkgate_e field, uint32_t val );
static inline uint32_t  ccu_get_esub_axi_clkgate( ccu_esub_axi_clkgate_e field );
static inline void      ccu_set_esub_apb_clkgate( ccu_esub_apb_clkgate_e field, uint32_t val );
static inline uint32_t  ccu_get_esub_apb_clkgate( ccu_esub_apb_clkgate_e field );
static inline void      ccu_set_esub_esw_sys_125m_clkgate( ccu_esub_esw_sys_125m_clkgate_e field, uint32_t val );
static inline uint32_t  ccu_get_esub_esw_sys_125m_clkgate( ccu_esub_esw_sys_125m_clkgate_e field );
static inline void      ccu_set_esub_esw_25m_clkgate( ccu_esub_esw_25m_clkgate_e field, uint32_t val );
static inline uint32_t  ccu_get_esub_esw_25m_clkgate( ccu_esub_esw_25m_clkgate_e field );
static inline void      ccu_set_esub_esw_sys_clkgate( ccu_esub_esw_sys_clkgate_e field, uint32_t val );
static inline uint32_t  ccu_get_esub_esw_sys_clkgate( ccu_esub_esw_sys_clkgate_e field );

static inline void      ccu_set_esub_eav_clkgate( ccu_esub_eav_clkgate_e field, uint32_t val );
static inline uint32_t  ccu_get_esub_eav_clkgate( ccu_esub_eav_clkgate_e field );

static inline void      ccu_set_esub_esw_sys_div( ccu_esub_esw_sys_div_e field, uint32_t val );
static inline uint32_t  ccu_get_esub_esw_sys_div( ccu_esub_esw_sys_div_e field );

static inline void      ccu_esub_pll_init(void);
static inline void      ccu_esub_system_init(void);

/* Local macros */

#define ccu_set_esub_bit(offset, field, val)  \
            ccu_set_bit( (MM_IO_BASE_ESUB_CLK + (offset)), (field), (val) )

#define ccu_get_esub_bit(offset, field)       \
            ccu_get_bit( (MM_IO_BASE_ESUB_CLK + (offset)), (field) )

#define ccu_set_esub_reg_field(offset, mask, shift, val)  \
            ccu_set_reg_field( (MM_IO_BASE_ESUB_CLK + (offset)), (mask), (shift), (val) );

#define ccu_get_esub_reg_field(offset, mask, shift)       \
            ccu_get_reg_field( (MM_IO_BASE_ESUB_CLK + (offset)), (mask), (shift) )

/* Functions */

static inline void ccu_set_esub_peri_volt( ccu_esub_peri_volt_e field, uint32_t val )
{
   uint32_t mask;

   if (field == ccu_esub_peri_volt_normal)
   {
      mask = ESUB_CLK_MGR_REG_VLT_PERI_VLT_NORMAL_PERI_MASK;
   }
   else if (field == ccu_esub_peri_volt_high)
   {
      mask = ESUB_CLK_MGR_REG_VLT_PERI_VLT_HIGH_PERI_MASK;
   }
   else
   {
      mask = 1 << field;
   }
   ccu_set_esub_reg_field( ESUB_CLK_MGR_REG_VLT_PERI_OFFSET, mask, field, val );
}

static inline uint32_t ccu_get_esub_peri_volt( ccu_esub_peri_volt_e field )
{
   uint32_t mask;

   if (field == ccu_esub_peri_volt_normal)
   {
      mask = ESUB_CLK_MGR_REG_VLT_PERI_VLT_NORMAL_PERI_MASK;
   }
   else if (field == ccu_esub_peri_volt_high)
   {
      mask = ESUB_CLK_MGR_REG_VLT_PERI_VLT_HIGH_PERI_MASK;
   }
   else
   {
      mask = 1 << field;
   }
   return ccu_get_esub_reg_field( ESUB_CLK_MGR_REG_VLT_PERI_OFFSET, mask, field );
}

static inline void ccu_set_esub_lvm( ccu_esub_lvm_e field, uint32_t val )
{
   uint32_t shift;
   uint32_t mask;
   uint32_t reg_offset;

   if (field < ccu_esub_lvm_4 )
   {
      reg_offset = ESUB_CLK_MGR_REG_LVM0_3_OFFSET;
   }
   else
   {
      reg_offset = ESUB_CLK_MGR_REG_LVM4_7_OFFSET;
      field = field - ccu_esub_lvm_4;
   }

   shift = field * 4;
   mask  = ESUB_CLK_MGR_REG_LVM0_3_LVM0_3_MD_00_MASK << (shift);

   ccu_set_esub_reg_field( reg_offset, mask, shift, val );
}

static inline uint32_t ccu_get_esub_lvm( ccu_esub_lvm_e field )
{
   uint32_t shift;
   uint32_t mask;
   uint32_t reg_offset;

   if (field < ccu_esub_lvm_4 )
   {
      reg_offset = ESUB_CLK_MGR_REG_LVM0_3_OFFSET;
   }
   else
   {
      reg_offset = ESUB_CLK_MGR_REG_LVM4_7_OFFSET;
      field = field - ccu_esub_lvm_4;
   }

   shift = field * 4;
   mask  = ESUB_CLK_MGR_REG_LVM0_3_LVM0_3_MD_00_MASK << (shift);

   return ccu_get_esub_reg_field( reg_offset, mask, shift );
}

static inline void ccu_set_esub_vlt( ccu_esub_vlt_e field, uint32_t val )
{
   uint32_t shift;
   uint32_t mask;
   uint32_t reg_offset;

   if (field < ccu_esub_vlt_4 )
   {
      reg_offset = ESUB_CLK_MGR_REG_VLT0_3_OFFSET;
   }
   else
   {
      reg_offset = ESUB_CLK_MGR_REG_VLT4_7_OFFSET;
      field = field - ccu_esub_vlt_4;
   }

   shift = field * 8;
   mask  = ESUB_CLK_MGR_REG_VLT0_3_VLT0_3_VV_00_MASK << (shift);

   ccu_set_esub_reg_field( reg_offset, mask, shift, val );
}

static inline uint32_t ccu_get_esub_vlt( ccu_esub_vlt_e field )
{
   uint32_t shift;
   uint32_t mask;
   uint32_t reg_offset;

   if (field < ccu_esub_vlt_4 )
   {
      reg_offset = ESUB_CLK_MGR_REG_VLT0_3_OFFSET;
   }
   else
   {
      reg_offset = ESUB_CLK_MGR_REG_VLT4_7_OFFSET;
      field = field - ccu_esub_vlt_4;
   }

   shift = field * 8;
   mask  = ESUB_CLK_MGR_REG_VLT0_3_VLT0_3_VV_00_MASK << (shift);

   return ccu_get_esub_reg_field( reg_offset, mask, shift );
}

static inline void ccu_set_esub_axi_clkgate( ccu_esub_axi_clkgate_e field, uint32_t val )
{
   ccu_set_esub_bit( ESUB_CLK_MGR_REG_ESUB_AXI_CLKGATE_OFFSET, field, val );
}

static inline uint32_t ccu_get_esub_axi_clkgate( ccu_esub_axi_clkgate_e field )
{
   return ccu_get_esub_bit( ESUB_CLK_MGR_REG_ESUB_AXI_CLKGATE_OFFSET, field );
}

static inline void ccu_set_esub_apb_clkgate( ccu_esub_apb_clkgate_e field, uint32_t val )
{
   ccu_set_esub_bit( ESUB_CLK_MGR_REG_ESUB_APB_CLKGATE_OFFSET, field, val );
}
static inline uint32_t ccu_get_esub_apb_clkgate( ccu_esub_apb_clkgate_e field )
{
   return ccu_get_esub_bit( ESUB_CLK_MGR_REG_ESUB_APB_CLKGATE_OFFSET, field );
}

static inline void ccu_set_esub_esw_sys_125m_clkgate( ccu_esub_esw_sys_125m_clkgate_e field, uint32_t val )
{
   ccu_set_esub_bit( ESUB_CLK_MGR_REG_ESW_SYS_125M_CLKGATE_OFFSET, field, val );
}
static inline uint32_t ccu_get_esub_esw_sys_125m_clkgate( ccu_esub_esw_sys_125m_clkgate_e field )
{
   return ccu_get_esub_bit( ESUB_CLK_MGR_REG_ESW_SYS_125M_CLKGATE_OFFSET, field );
}

static inline void ccu_set_esub_esw_25m_clkgate( ccu_esub_esw_25m_clkgate_e field, uint32_t val )
{
   ccu_set_esub_bit( ESUB_CLK_MGR_REG_ESW_25M_CLKGATE_OFFSET, field, val );
}
static inline uint32_t ccu_get_esub_esw_25m_clkgate( ccu_esub_esw_25m_clkgate_e field )
{
   return ccu_get_esub_bit( ESUB_CLK_MGR_REG_ESW_25M_CLKGATE_OFFSET, field );
}

static inline void ccu_set_esub_esw_sys_clkgate( ccu_esub_esw_sys_clkgate_e field, uint32_t val )
{
   ccu_set_esub_bit( ESUB_CLK_MGR_REG_ESW_SYS_CLKGATE_OFFSET, field, val );
}
static inline uint32_t ccu_get_esub_esw_sys_clkgate( ccu_esub_esw_sys_clkgate_e field )
{
   return ccu_get_esub_bit( ESUB_CLK_MGR_REG_ESW_SYS_CLKGATE_OFFSET, field );
}

static inline void      ccu_set_esub_eav_clkgate( ccu_esub_eav_clkgate_e field, uint32_t val )
{
   ccu_set_esub_bit( ESUB_CLK_MGR_REG_EAV_CLKGATE_OFFSET, field, val );
}
static inline uint32_t  ccu_get_esub_eav_clkgate( ccu_esub_eav_clkgate_e field )
{
   return ccu_get_esub_bit( ESUB_CLK_MGR_REG_EAV_CLKGATE_OFFSET, field );
}

static inline void ccu_set_esub_esw_sys_div( ccu_esub_esw_sys_div_e field, uint32_t val )
{
   uint32_t mask;

   if (field == ccu_esub_esw_sys_div_override)
   {
      mask = ESUB_CLK_MGR_REG_ESW_SYS_DIV_ESW_SYS_TRIGGER_MASK;
   }
   else if (field == ccu_esub_esw_sys_div)
   {
      mask = ESUB_CLK_MGR_REG_ESW_SYS_DIV_ESW_SYS_DIV_MASK;
   }
   else if (field == ccu_esub_esw_sys_pll_sel)
   {
      mask = ESUB_CLK_MGR_REG_ESW_SYS_DIV_ESW_SYS_PLL_SELECT_MASK;
   }
   else
   {
      mask = 1 << field;
   }
   ccu_set_esub_reg_field(ESUB_CLK_MGR_REG_ESW_SYS_DIV_OFFSET, mask, field, val);
}

static inline uint32_t ccu_get_esub_esw_sys_div( ccu_esub_esw_sys_div_e field )
{
   uint32_t mask;

   if (field == ccu_esub_esw_sys_div_override)
   {
      mask = ESUB_CLK_MGR_REG_ESW_SYS_DIV_ESW_SYS_TRIGGER_MASK;
   }
   else if (field == ccu_esub_esw_sys_div)
   {
      mask = ESUB_CLK_MGR_REG_ESW_SYS_DIV_ESW_SYS_DIV_MASK;
   }
   else if (field == ccu_esub_esw_sys_pll_sel)
   {
      mask = ESUB_CLK_MGR_REG_ESW_SYS_DIV_ESW_SYS_PLL_SELECT_MASK;
   }
   else
   {
      mask = 1 << field;
   }
   return ccu_get_esub_reg_field( ESUB_CLK_MGR_REG_ESW_SYS_DIV_OFFSET, mask, field );
}

static inline void ccu_esub_pll_init(void)
{
   /* enable access */
   ccu_unlock_esub_clk_mgr();
     
   ccu_set_bit(MM_IO_BASE_ESUB_CLK + ESUB_CLK_MGR_REG_PLLE_POST_RESETB_OFFSET, ESUB_CLK_MGR_REG_PLLE_POST_RESETB_I_POST_RESETB_PLLE_SHIFT, 0);

   /* take PLL out of reset and put into normal mode */
   ccu_set_bit(MM_IO_BASE_ESUB_CLK + ESUB_CLK_MGR_REG_PLLE_RESETB_OFFSET, ESUB_CLK_MGR_REG_PLLE_RESETB_I_PLL_RESETB_PLLE_SHIFT, 1);

   /* wait for PLL lock */
   while( !ccu_get_bit(MM_IO_BASE_ESUB_CLK + ESUB_CLK_MGR_REG_PLL_LOCK_OFFSET, ESUB_CLK_MGR_REG_PLL_LOCK_PLL_LOCK_PLLE_SHIFT) );

   ccu_set_bit(MM_IO_BASE_ESUB_CLK + ESUB_CLK_MGR_REG_PLLE_POST_RESETB_OFFSET, ESUB_CLK_MGR_REG_PLLE_POST_RESETB_I_POST_RESETB_PLLE_SHIFT, 1);
   
   /* Lock clock manager registers */
   ccu_lock_esub_clk_mgr();
}

static inline void ccu_esub_sys_clk_init(void)
{
   /* enable access */
   ccu_unlock_esub_clk_mgr();

   /* select 208Mhz clock */
   ccu_set_esub_esw_sys_div( ccu_esub_esw_sys_pll_sel, ESUB_CLK_MGR_REG_ESW_SYS_DIV_ESW_SYS_PLL_SELECT_CMD_VAR_208M_CLK );

   /* set divider to 2 for 104Mhz output */
   ccu_set_esub_esw_sys_div( ccu_esub_esw_sys_div, 1 ); 

   /* trigger */
   ccu_set_esub_esw_sys_div( ccu_esub_esw_sys_div_override, 1 );

   /* Lock clock manager registers */
   ccu_lock_esub_clk_mgr();
}

static inline void ccu_esub_system_init(void)
{
   /* initialize the entire esub system (i.e. DMA, ESW, VPM) */

   /* Configure policy mask */
   ccu_set_esub_policy_mask( ccu_policy_0, ccu_esub_policy_mask_esub );
   ccu_set_esub_policy_mask( ccu_policy_1, ccu_esub_policy_mask_esub );
   ccu_set_esub_policy_mask( ccu_policy_2, ccu_esub_policy_mask_esub );
   ccu_set_esub_policy_mask( ccu_policy_3, ccu_esub_policy_mask_esub );

   ccu_set_esub_policy_mask( ccu_policy_0, ccu_esub_policy_mask_esw_sys );
   ccu_set_esub_policy_mask( ccu_policy_1, ccu_esub_policy_mask_esw_sys );
   ccu_set_esub_policy_mask( ccu_policy_2, ccu_esub_policy_mask_esw_sys );
   ccu_set_esub_policy_mask( ccu_policy_3, ccu_esub_policy_mask_esw_sys );

   ccu_set_esub_policy_mask( ccu_policy_0, ccu_esub_policy_mask_eav );
   ccu_set_esub_policy_mask( ccu_policy_1, ccu_esub_policy_mask_eav );
   ccu_set_esub_policy_mask( ccu_policy_2, ccu_esub_policy_mask_eav );
   ccu_set_esub_policy_mask( ccu_policy_3, ccu_esub_policy_mask_eav );

   ccu_set_esub_policy_mask( ccu_policy_0, ccu_esub_policy_mask_esw );
   ccu_set_esub_policy_mask( ccu_policy_1, ccu_esub_policy_mask_esw );
   ccu_set_esub_policy_mask( ccu_policy_2, ccu_esub_policy_mask_esw );
   ccu_set_esub_policy_mask( ccu_policy_3, ccu_esub_policy_mask_esw );

   /* enable access */
   ccu_unlock_esub_clk_mgr();

   /* Configure clock gates */
   ccu_set_esub_axi_clkgate( ccu_esub_axi_clkgate_esub_axi_clk_en, 1 );
   ccu_set_esub_axi_clkgate( ccu_esub_axi_clkgate_esub_axi_hw_sw_gating_sel, 1 );
   ccu_set_esub_axi_clkgate( ccu_esub_axi_clkgate_esub_axi_hyst_val, 1 );
   ccu_set_esub_axi_clkgate( ccu_esub_axi_clkgate_esub_axi_hyst_en, 1 );

   ccu_set_esub_apb_clkgate( ccu_esub_apb_clkgate_esub_apb_clk_en, 1 );
   ccu_set_esub_apb_clkgate( ccu_esub_apb_clkgate_esub_apb_hw_sw_gating_sel, 1 );
   ccu_set_esub_apb_clkgate( ccu_esub_apb_clkgate_esub_apb_hyst_val, 1 );
   ccu_set_esub_apb_clkgate( ccu_esub_apb_clkgate_esub_apb_hyst_en, 1 );

   ccu_set_esub_esw_sys_125m_clkgate( ccu_esub_esw_sys_125m_clkgate_esw_sys_125m_clk_en, 1 );
   ccu_set_esub_esw_sys_125m_clkgate( ccu_esub_esw_sys_125m_clkgate_esw_sys_125m_hw_sw_gating_sel, 1 );
   ccu_set_esub_esw_sys_125m_clkgate( ccu_esub_esw_sys_125m_clkgate_esw_sys_125m_hyst_val, 1 );
   ccu_set_esub_esw_sys_125m_clkgate( ccu_esub_esw_sys_125m_clkgate_esw_sys_125m_hyst_en, 1 );

   ccu_set_esub_esw_25m_clkgate( ccu_esub_esw_25m_clkgate_esw_25m_clk_en, 1 );
   ccu_set_esub_esw_25m_clkgate( ccu_esub_esw_25m_clkgate_esw_25m_hw_sw_gating_sel, 1 );
   ccu_set_esub_esw_25m_clkgate( ccu_esub_esw_25m_clkgate_esw_25m_hyst_val, 1 );
   ccu_set_esub_esw_25m_clkgate( ccu_esub_esw_25m_clkgate_esw_25m_hyst_en, 1 );

   ccu_set_esub_esw_sys_clkgate( ccu_esub_esw_sys_clkgate_esw_sys_clk_en, 1 );
   ccu_set_esub_esw_sys_clkgate( ccu_esub_esw_sys_clkgate_esw_sys_hw_sw_gating_sel, 1 );
   ccu_set_esub_esw_sys_clkgate( ccu_esub_esw_sys_clkgate_esw_sys_hyst_val, 1 );
   ccu_set_esub_esw_sys_clkgate( ccu_esub_esw_sys_clkgate_esw_sys_hyst_en, 1 );

   /* Lock clock manager registers */
   ccu_lock_esub_clk_mgr();
}



#ifdef __cplusplus
}
#endif

#endif /* _CCU_ESUB_INLINE_H_*/

