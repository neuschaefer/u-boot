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
* @file  chal_ccu_inline.c
*
* @brief Clock Control Unit inline functions
*
* @note
*
*******************************************************************************/
#ifndef _CCU_INLINE_H_
#define _CCU_INLINE_H_

#ifdef __cplusplus
extern "C" {
#endif

//#include <asm/arch/chipregHw_inline.h>
#include <asm/arch/mm_io.h>

#include <asm/arch/brcm_rdb_kpm_clk_mgr_reg.h>
#include <asm/arch/brcm_rdb_kps_clk_mgr_reg.h>
#include <asm/arch/brcm_rdb_khubaon_clk_mgr_reg.h>
#include <asm/arch/brcm_rdb_khub_clk_mgr_reg.h>
#include <asm/arch/brcm_rdb_kproc_clk_mgr_reg.h>
#include <asm/arch/brcm_rdb_esub_clk_mgr_reg.h>

#include "ccu_util_inline.h"
#include <asm/arch/chal_reg.h>

#define  CCU_WR_ACCESS_MODE_MASK        0x80000000
#define  CCU_WR_ACCESS_PASSWORD         0x00A5A500
#define  CCU_WR_ACCESS_ENABLE           0x00000001

#define  CCU_POLICY_CONFIG_EN_MASK      0x00000001

/* Maximum AXI frequency */
#define CCU_MAX_A9_AXI_FREQ		       300000000

/* FPGA A9 core frequency */
#define  CCU_FPGA_A9_FREQ               5000000


/* I-prefix fix up */
#define I_FIX(label)	label

/* Name fix-up */
#define NFIX(label)		I_FIX(label)



/* Active/Target policy types */
typedef enum
{
   ccu_tgt_policy = 0,   /* target policy  */
   ccu_act_policy        /* active policy  */
   
} ccu_policy_freq_type_e;

/* Generic policy numbers */
typedef enum
{
   ccu_policy_0 = 0,
   ccu_policy_1,
   ccu_policy_2,
   ccu_policy_3   
} ccu_policy_num_e;

/* Generic frequency IDs */
typedef enum
{
   ccu_freq_id_0 = 0,
   ccu_freq_id_1,
   ccu_freq_id_2,
   ccu_freq_id_3,
   ccu_freq_id_4,
   ccu_freq_id_5,
   ccu_freq_id_6,
   ccu_freq_id_7
} ccu_freq_id_e;


/* POLICY FREQ */
typedef enum
{
   /* AXI/AHB/APB4 frequencies */
   ccu_kpm_policy_freq_xtal = 0,
   ccu_kpm_policy_freq_52_52_52,
   ccu_kpm_policy_freq_104_52_26,
   ccu_kpm_policy_freq_156_156_26,
   ccu_kpm_policy_freq_156_78_39,
   ccu_kpm_policy_freq_208_104_52,
   ccu_kpm_policy_freq_312_104_52,
   ccu_kpm_policy_freq_312_156_78,
   ccu_kpm_policy_freq_not_supported
} ccu_kpm_policy_freq_e;

typedef enum
{
   /* AXI/AP1=APB2=APB3=APB_HSM frequencies */
   ccu_kps_policy_freq_xtal = 0,
   ccu_kps_policy_freq_52_26,
   ccu_kps_policy_freq_78_39,
   ccu_kps_policy_freq_104_52,
   ccu_kps_policy_freq_156_52,
   ccu_kps_policy_freq_156_78,
   ccu_kps_policy_freq_not_supported
} ccu_kps_policy_freq_e;

typedef enum
{
   /* AXI/APB frequencies */
   ccu_khubaon_policy_freq_xtal = 0,
   ccu_khubaon_policy_freq_52_52,
   ccu_khubaon_policy_freq_78_78,
   ccu_khubaon_policy_freq_104_52,
   ccu_khubaon_policy_freq_156_78,
   ccu_khubaon_policy_freq_not_supported
} ccu_khubaon_policy_freq_e;

typedef enum
{
   /* AXI/NOR/BROM/APB frequencies */
   ccu_khub_policy_freq_xtal = 0,
   ccu_khub_policy_freq_52_52_52_52,
   ccu_khub_policy_freq_104_104_52_52,
   ccu_khub_policy_freq_156_156_78_78,
   ccu_khub_policy_freq_104_104_104_104,
   ccu_khub_policy_freq_208_104_104_104,
   ccu_khub_policy_freq_312_156_156_156,
   ccu_khub_policy_freq_not_supported
} ccu_khub_policy_freq_e;

typedef enum
{
   ccu_esub_policy_freq_xtal = 0,
   ccu_esub_policy_freq_25_0_0_0_0,
   ccu_esub_policy_freq_25_125_100_375_300,
   ccu_esub_policy_freq_25_125_100_0_0,
   ccu_esub_policy_freq_0_0_0_424_212,
   ccu_esub_policy_freq_0_0_0_318_212,
   ccu_esub_policy_freq_0_0_0_0_0,
   ccu_esub_policy_freq_not_supported
} ccu_esub_policy_freq_e;

/* POLICY MASK */
typedef enum
{
   ccu_kpm_policy_mask_sys_switch = 0,
   ccu_kpm_policy_mask_dma,
   ccu_kpm_policy_mask_master_switch,
   ccu_kpm_policy_mask_sdio3,
   ccu_kpm_policy_mask_sdio4,
   ccu_kpm_policy_mask_sdio2,
   ccu_kpm_policy_mask_sdio1,
   ccu_kpm_policy_mask_nand,
   ccu_kpm_policy_mask_reserved8,
   ccu_kpm_policy_mask_usb_ic,
   ccu_kpm_policy_mask_reserved10,
   ccu_kpm_policy_mask_usb_otg,
   ccu_kpm_policy_mask_armcore,
   ccu_kpm_policy_mask_apb8,
   ccu_kpm_policy_mask_reserved14,
   ccu_kpm_policy_mask_reserved15,
   ccu_kpm_policy_mask_hsic2,
   ccu_kpm_policy_mask_usbh2,
   ccu_kpm_policy_mask_spi_slave,
   ccu_kpm_policy_mask_not_supported
} ccu_kpm_policy_mask_e;

typedef enum
{
   ccu_kps_policy_mask_switch = 0,
   ccu_kps_policy_mask_ext,
   ccu_kps_policy_mask_spum_open,
   ccu_kps_policy_mask_spum_sec,
   ccu_kps_policy_mask_reserved4,
   ccu_kps_policy_mask_reserved5,
   ccu_kps_policy_mask_reserved6,
   ccu_kps_policy_mask_timers,
   ccu_kps_policy_mask_dmac_mux,
   ccu_kps_policy_mask_reserved9,
   ccu_kps_policy_mask_reserved10,
   ccu_kps_policy_mask_reserved11,
   ccu_kps_policy_mask_reserved12,
   ccu_kps_policy_mask_reserved13,
   ccu_kps_policy_mask_ssp2,
   ccu_kps_policy_mask_reserved15,
   ccu_kps_policy_mask_ssp0,
   ccu_kps_policy_mask_uartb4,
   ccu_kps_policy_mask_uartb3,
   ccu_kps_policy_mask_uartb2,
   ccu_kps_policy_mask_uartb,
   ccu_kps_policy_mask_pwm,
   ccu_kps_policy_mask_reserved22,
   ccu_kps_policy_mask_bsc2,
   ccu_kps_policy_mask_bsc1,
   ccu_kps_policy_mask_reserved25,
   ccu_kps_policy_mask_reserved26,
   ccu_kps_policy_mask_mphi,
   ccu_kps_policy_mask_bbl,
   ccu_kps_policy_mask_dap,

   ccu_kps_policy_mask_uartb5 = 32,
   ccu_kps_policy_mask_uartb6,
   ccu_kps_policy_mask_cir,
   ccu_kps_policy_mask_ledm,
   ccu_kps_policy_mask_bsc3,
   ccu_kps_policy_mask_not_supported
} ccu_kps_policy_mask_e;

typedef enum
{
   ccu_khubaon_policy_mask_reserved0 = 0,
   ccu_khubaon_policy_mask_reserved1,
   ccu_khubaon_policy_mask_reserved2,
   ccu_khubaon_policy_mask_gpiokp,
   ccu_khubaon_policy_mask_reserved4,
   ccu_khubaon_policy_mask_hubaon,
   ccu_khubaon_policy_mask_hub_timer,
   ccu_khubaon_policy_mask_reserved7,
   ccu_khubaon_policy_mask_pmu_bsc,
   ccu_khubaon_policy_mask_reserved9,
   ccu_khubaon_policy_mask_reserved10,
   ccu_khubaon_policy_mask_sysemi,
   ccu_khubaon_policy_mask_reserved12,
   ccu_khubaon_policy_mask_reserved13,
   ccu_khubaon_policy_mask_vcemi,
   ccu_khubaon_policy_mask_fmon,
   ccu_khubaon_policy_mask_sec_wd,
   ccu_khubaon_policy_mask_spm,
   ccu_khubaon_policy_mask_aci,
   ccu_khubaon_policy_mask_sim,
   ccu_khubaon_policy_mask_sim2,
   ccu_khubaon_policy_mask_dap,
   ccu_khubaon_policy_mask_pscs,
   ccu_khubaon_policy_mask_not_supported
} ccu_khubaon_policy_mask_e;

typedef enum
{
   ccu_khub_policy_mask_brom = 0,
   ccu_khub_policy_mask_cti,
   ccu_khub_policy_mask_funnel,
   ccu_khub_policy_mask_reserved3,
   ccu_khub_policy_mask_reserved4,
   ccu_khub_policy_mask_hub,
   ccu_khub_policy_mask_reserved6,
   ccu_khub_policy_mask_nor,
   ccu_khub_policy_mask_reserved8,
   ccu_khub_policy_mask_sec_viol_trap4,
   ccu_khub_policy_mask_sec_viol_trap7,
   ccu_khub_policy_mask_reserved11,
   ccu_khub_policy_mask_tpiu,
   ccu_khub_policy_mask_vc_itm,
   ccu_khub_policy_mask_reserved14,
   ccu_khub_policy_mask_hsi,
   ccu_khub_policy_mask_axi_trace_19,
   ccu_khub_policy_mask_axi_trace_11,
   ccu_khub_policy_mask_axi_trace_12,
   ccu_khub_policy_mask_axi_trace_13,
   ccu_khub_policy_mask_etb,
   ccu_khub_policy_mask_final_funnel,
   ccu_khub_policy_mask_mdiomaster,
   ccu_khub_policy_mask_caph,
   ccu_khub_policy_mask_ssp4,
   ccu_khub_policy_mask_ssp5,
   ccu_khub_policy_mask_ssp6,

   ccu_khub_policy_mask_reserved32 = 32,
   ccu_khub_policy_mask_atb_filter,
   ccu_khub_policy_mask_reserved34,
   ccu_khub_policy_mask_reserved35,

   ccu_khub_policy_mask_etb2axi,
   ccu_khub_policy_mask_reserved37,
   ccu_khub_policy_mask_ssp3,
   ccu_khub_policy_mask_reserved39,
   ccu_khub_policy_mask_reserved40,
   ccu_khub_policy_mask_reserved41,
   ccu_khub_policy_mask_reserved42,
   ccu_khub_policy_mask_reserved43,
   ccu_khub_policy_mask_reserved44,
   ccu_khub_policy_mask_reserved45,
   ccu_khub_policy_mask_reserved46,
   ccu_khub_policy_mask_reserved47,
   ccu_khub_policy_mask_reserved48,
   ccu_khub_policy_mask_reserved49,
   ccu_khub_policy_mask_reserved50,
   ccu_khub_policy_mask_reserved51,
   ccu_khub_policy_mask_reserved52,
   ccu_khub_policy_mask_reserved53,
   ccu_khub_policy_mask_reserved54,
   ccu_khub_policy_mask_audioh,
   ccu_khub_policy_mask_sec_viol_trap_5,
   ccu_khub_policy_mask_tmon,
   ccu_khub_policy_mask_var_spm,
   ccu_khub_policy_mask_dap_switch,
   ccu_khub_policy_mask_sec_viol_trap_6,
   ccu_khub_policy_mask_not_supported
} ccu_khub_policy_mask_e;

typedef enum
{
   /* A9/AXI/APB0 frequencies, in MHz */
   ccu_kproc_policy_freq_xtal = 0,
   ccu_kproc_policy_freq_52_52_52,
   ccu_kproc_policy_freq_156_156_78,
   ccu_kproc_policy_freq_156_156_78_0V9,            /* 0.9V target */
   ccu_kproc_policy_freq_312_156_104,
   ccu_kproc_policy_freq_312_156_104_1V0,           /* 1.0V target */
   ccu_kproc_policy_freq_aclk_aclkdiv2_aclkdiv3,    /* 1.1V target */
   ccu_kproc_policy_freq_aclkh_aclkhdiv2_aclkhdiv3  /* 1.2V target */
   
} ccu_kproc_policy_freq_e;

typedef enum
{
   ccu_kproc_arm_policy_mask,
   ccu_kproc_apb0_policy_mask   
} ccu_kproc_policy_mask_e;

typedef enum
{
   ccu_esub_policy_mask_esub = 0,
   ccu_esub_policy_mask_esw_sys,
   ccu_esub_policy_mask_reserved2,
   ccu_esub_policy_mask_esw,
   ccu_esub_policy_mask_eav,
   ccu_esub_policy_mask_not_supported
} ccu_esub_policy_mask_e;




/* Function Prototypes */
static inline uint32_t  ccu_unlock_clk_manager( uint32_t addr );
static inline void      ccu_lock_clk_manager( uint32_t addr );
static inline void      ccu_set_lvm_enable( uint32_t addr );
static inline void      ccu_wait_lvm_ready( uint32_t addr );
static inline uint32_t  ccu_is_lvm_ready( uint32_t addr, int timeout );

static inline void                        ccu_set_kpm_policy_freq( ccu_policy_num_e policy_num, ccu_kpm_policy_freq_e freq );
static inline ccu_kpm_policy_freq_e  ccu_get_kpm_policy_freq( uint32_t policy_num );
static inline void                        ccu_cfg_kpm_policy_mask( ccu_policy_num_e policy_num, ccu_kpm_policy_mask_e policy_type, uint32_t val );
static inline void                        ccu_set_kpm_policy_mask( ccu_policy_num_e policy_num, ccu_kpm_policy_mask_e policy_type );
static inline void                        ccu_clr_kpm_policy_mask( ccu_policy_num_e policy_num, ccu_kpm_policy_mask_e policy_type );
static inline uint32_t                    ccu_get_kpm_policy_mask( ccu_policy_num_e policy_num, ccu_kpm_policy_mask_e policy_type );
static inline ccu_policy_num_e       ccu_get_kpm_active_policy( void );

static inline void                        ccu_set_kps_policy_freq( ccu_policy_num_e policy_num, ccu_kps_policy_freq_e freq );
static inline ccu_kps_policy_freq_e  ccu_get_kps_policy_freq( ccu_policy_num_e policy_num );
static inline void                        ccu_cfg_kps_policy_mask( ccu_policy_num_e policy_num, ccu_kps_policy_mask_e policy_type, uint32_t val );
static inline void                        ccu_set_kps_policy_mask( ccu_policy_num_e policy_num, ccu_kps_policy_mask_e policy_type );
static inline void                        ccu_clr_kps_policy_mask( ccu_policy_num_e policy_num, ccu_kps_policy_mask_e policy_type );
static inline uint32_t                    ccu_get_kps_policy_mask( ccu_policy_num_e policy_num, ccu_kps_policy_mask_e policy_type );
static inline ccu_policy_num_e       ccu_get_kps_active_policy( void );

static inline void                           ccu_set_khubaon_policy_freq( ccu_policy_num_e policy_num, ccu_khubaon_policy_freq_e freq );
static inline ccu_khubaon_policy_freq_e ccu_get_khubaon_policy_freq( ccu_policy_num_e policy_num );
static inline void                           ccu_cfg_khubaon_policy_mask( ccu_policy_num_e policy_num, ccu_khubaon_policy_mask_e policy_type, uint32_t val );
static inline void                           ccu_set_khubaon_policy_mask( ccu_policy_num_e policy_num, ccu_khubaon_policy_mask_e policy_type );
static inline void                           ccu_clr_khubaon_policy_mask( ccu_policy_num_e policy_num, ccu_khubaon_policy_mask_e policy_type );
static inline uint32_t                       ccu_get_khubaon_policy_mask( ccu_policy_num_e policy_num, ccu_khubaon_policy_mask_e policy_type );
static inline ccu_policy_num_e          ccu_get_khubaon_active_policy( void );

static inline void                        ccu_set_khub_policy_freq( ccu_policy_num_e policy_num, ccu_khub_policy_freq_e freq );
static inline ccu_khub_policy_freq_e ccu_get_khub_policy_freq( ccu_policy_num_e policy_num );
static inline void                        ccu_cfg_khub_policy_mask( ccu_policy_num_e policy_num, ccu_khub_policy_mask_e policy_type, uint32_t val );
static inline void                        ccu_set_khub_policy_mask( ccu_policy_num_e policy_num, ccu_khub_policy_mask_e policy_type );
static inline void                        ccu_clr_khub_policy_mask( ccu_policy_num_e policy_num, ccu_khub_policy_mask_e policy_type );
static inline uint32_t                    ccu_get_khub_policy_mask( ccu_policy_num_e policy_num, ccu_khub_policy_mask_e policy_type );
static inline ccu_policy_num_e       ccu_get_khub_active_policy( void );

static inline void                           ccu_set_kproc_axi_div( uint32_t div );
static inline uint32_t                       ccu_get_kproc_axi_div( ccu_kproc_policy_freq_e freq_id );
static inline int                            ccu_set_kproc_fid7_arm_swt_div( uint32_t div );
static inline uint32_t                       ccu_get_kproc_fid7_arm_swt_div( void );
static inline void                           ccu_set_kproc_mdiv( ccu_kproc_policy_freq_e fid, uint32_t mdiv );
static inline uint32_t                       ccu_get_kproc_mdiv( ccu_kproc_policy_freq_e fid );
static inline void                           ccu_set_kproc_pll( uint32_t ndiv_int, uint32_t ndiv_frac, uint8_t pdiv, uint8_t mdiv );
static inline void                           ccu_set_kproc_pl310_div(uint32_t div);
static inline uint32_t                       ccu_get_kproc_pl310_div(void);
static inline void                           ccu_get_kproc_pll( uint32_t *ndiv_int, uint32_t *ndiv_frac, uint32_t *pdiv, uint32_t *mdiv);
static inline uint32_t                       ccu_get_kproc_pll_freq( void );
static inline void                           ccu_set_kproc_policy_freq( ccu_policy_num_e policy_num, ccu_kproc_policy_freq_e freq );
static inline ccu_kproc_policy_freq_e   ccu_get_kproc_policy_freq( ccu_policy_num_e policy_num );
static inline uint32_t                       ccu_get_kproc_policy_freq_a9_hz( ccu_kproc_policy_freq_e freq );
static inline void                           ccu_cfg_kproc_policy_mask( ccu_policy_num_e policy_num, ccu_kproc_policy_mask_e policy_type, uint32_t val );
static inline void                           ccu_set_kproc_policy_mask( ccu_policy_num_e policy_num, ccu_kproc_policy_mask_e policy_type );
static inline void                           ccu_clr_kproc_policy_mask( ccu_policy_num_e policy_num, ccu_kproc_policy_mask_e policy_type );
static inline uint32_t                       ccu_get_kproc_policy_mask( ccu_policy_num_e policy_num, ccu_kproc_policy_mask_e policy_type );
static inline ccu_policy_num_e          ccu_get_kproc_active_policy( void );

static inline void                         ccu_set_esub_policy_freq( ccu_policy_num_e policy_num, ccu_esub_policy_freq_e freq );
static inline ccu_esub_policy_freq_e  ccu_get_esub_policy_freq( uint32_t policy_num );
static inline void                         ccu_cfg_esub_policy_mask( ccu_policy_num_e policy_num, ccu_esub_policy_mask_e policy_type, uint32_t val );
static inline void                         ccu_set_esub_policy_mask( ccu_policy_num_e policy_num, ccu_esub_policy_mask_e policy_type );
static inline void                         ccu_clr_esub_policy_mask( ccu_policy_num_e policy_num, ccu_esub_policy_mask_e policy_type );
static inline uint32_t                     ccu_get_esub_policy_mask( ccu_policy_num_e policy_num, ccu_esub_policy_mask_e policy_type );
static inline ccu_policy_num_e        ccu_get_esub_active_policy( void );

#define ccu_unlock_kpm_clk_mgr()           ccu_unlock_clk_manager( MM_IO_BASE_KPM_CLK + KPM_CLK_MGR_REG_WR_ACCESS_OFFSET )
#define ccu_lock_kpm_clk_mgr()             ccu_lock_clk_manager( MM_IO_BASE_KPM_CLK + KPM_CLK_MGR_REG_WR_ACCESS_OFFSET )
#define ccu_restore_kpm_clk_mgr(old_en)    ccu_restore_clk_manager( MM_IO_BASE_KPM_CLK + KPM_CLK_MGR_REG_WR_ACCESS_OFFSET, old_en )
#define ccu_set_kpm_lvm_enable()           ccu_set_lvm_enable( MM_IO_BASE_KPM_CLK + KPM_CLK_MGR_REG_LVM_EN_OFFSET )
#define ccu_wait_kpm_lvm_ready()           ccu_wait_lvm_ready( MM_IO_BASE_KPM_CLK + KPM_CLK_MGR_REG_LVM_EN_OFFSET )
#define ccu_is_kpm_lvm_ready(timeout)      ccu_is_lvm_ready( MM_IO_BASE_KPM_CLK + KPM_CLK_MGR_REG_LVM_EN_OFFSET, timeout )

#define ccu_unlock_kps_clk_mgr()           ccu_unlock_clk_manager( MM_IO_BASE_SLV_CLK + NFIX(KPS_CLK_MGR_REG_WR_ACCESS_OFFSET) )
#define ccu_lock_kps_clk_mgr()             ccu_lock_clk_manager( MM_IO_BASE_SLV_CLK + NFIX(KPS_CLK_MGR_REG_WR_ACCESS_OFFSET) )
#define ccu_restore_kps_clk_mgr(old_en)    ccu_restore_clk_manager( MM_IO_BASE_SLV_CLK + NFIX(KPS_CLK_MGR_REG_WR_ACCESS_OFFSET), old_en )
#define ccu_set_kps_lvm_enable()           ccu_set_lvm_enable( MM_IO_BASE_SLV_CLK + NFIX(KPS_CLK_MGR_REG_LVM_EN_OFFSET) )
#define ccu_wait_kps_lvm_ready()           ccu_wait_lvm_ready( MM_IO_BASE_SLV_CLK + NFIX(KPS_CLK_MGR_REG_LVM_EN_OFFSET) )
#define ccu_is_kps_lvm_ready(timeout)      ccu_is_lvm_ready( MM_IO_BASE_SLV_CLK + NFIX(KPS_CLK_MGR_REG_LVM_EN_OFFSET), timeout )

#define ccu_unlock_khubaon_clk_mgr()          ccu_unlock_clk_manager( MM_IO_BASE_AON_CLK + KHUBAON_CLK_MGR_REG_WR_ACCESS_OFFSET )
#define ccu_lock_khubaon_clk_mgr()            ccu_lock_clk_manager( MM_IO_BASE_AON_CLK + KHUBAON_CLK_MGR_REG_WR_ACCESS_OFFSET )
#define ccu_restore_khubaon_clk_mgr(old_en)   ccu_restore_clk_manager( MM_IO_BASE_AON_CLK + KHUBAON_CLK_MGR_REG_WR_ACCESS_OFFSET, old_en )
#define ccu_set_khubaon_lvm_enable()          ccu_set_lvm_enable( MM_IO_BASE_AON_CLK + KHUBAON_CLK_MGR_REG_LVM_EN_OFFSET )
#define ccu_wait_khubaon_lvm_ready()          ccu_wait_lvm_ready( MM_IO_BASE_AON_CLK + KHUBAON_CLK_MGR_REG_LVM_EN_OFFSET )
#define ccu_is_khubaon_lvm_ready(timeout)     ccu_is_lvm_ready( MM_IO_BASE_AON_CLK + KHUBAON_CLK_MGR_REG_LVM_EN_OFFSET, timeout )

#define ccu_unlock_khub_clk_mgr()          ccu_unlock_clk_manager( MM_IO_BASE_HUB_CLK + KHUB_CLK_MGR_REG_WR_ACCESS_OFFSET )
#define ccu_lock_khub_clk_mgr()            ccu_lock_clk_manager( MM_IO_BASE_HUB_CLK + KHUB_CLK_MGR_REG_WR_ACCESS_OFFSET )
#define ccu_restore_khub_clk_mgr(old_en)   ccu_restore_clk_manager( MM_IO_BASE_HUB_CLK + KHUB_CLK_MGR_REG_WR_ACCESS_OFFSET, old_en )
#define ccu_set_khub_lvm_enable()          ccu_set_lvm_enable( MM_IO_BASE_HUB_CLK + KHUB_CLK_MGR_REG_LVM_EN_OFFSET )
#define ccu_wait_khub_lvm_ready()          ccu_wait_lvm_ready( MM_IO_BASE_HUB_CLK + KHUB_CLK_MGR_REG_LVM_EN_OFFSET )
#define ccu_is_khub_lvm_ready(timeout)     ccu_is_lvm_ready( MM_IO_BASE_HUB_CLK + KHUB_CLK_MGR_REG_LVM_EN_OFFSET, timeout )

#define ccu_unlock_kproc_clk_mgr()         ccu_unlock_clk_manager( MM_IO_BASE_PROC_CLK + KPROC_CLK_MGR_REG_WR_ACCESS_OFFSET )
#define ccu_lock_kproc_clk_mgr()           ccu_lock_clk_manager( MM_IO_BASE_PROC_CLK + KPROC_CLK_MGR_REG_WR_ACCESS_OFFSET )
#define ccu_restore_kproc_clk_mgr(old_en)  ccu_restore_clk_manager( MM_IO_BASE_PROC_CLK + KPROC_CLK_MGR_REG_WR_ACCESS_OFFSET, old_en )
#define ccu_set_kproc_lvm_enable()         ccu_set_lvm_enable( MM_IO_BASE_PROC_CLK + KPROC_CLK_MGR_REG_LVM_EN_OFFSET )
#define ccu_wait_kproc_lvm_ready()         ccu_wait_lvm_ready( MM_IO_BASE_PROC_CLK + KPROC_CLK_MGR_REG_LVM_EN_OFFSET )
#define ccu_is_kproc_lvm_ready(timeout)    ccu_is_lvm_ready( MM_IO_BASE_PROC_CLK + KPROC_CLK_MGR_REG_LVM_EN_OFFSET, timeout )

#define ccu_unlock_esub_clk_mgr()           ccu_unlock_clk_manager( MM_IO_BASE_ESUB_CLK + ESUB_CLK_MGR_REG_WR_ACCESS_OFFSET )
#define ccu_lock_esub_clk_mgr()             ccu_lock_clk_manager( MM_IO_BASE_ESUB_CLK + ESUB_CLK_MGR_REG_WR_ACCESS_OFFSET )
#define ccu_restore_esub_clk_mgr(old_en)    ccu_restore_clk_manager( MM_IO_BASE_ESUB_CLK + ESUB_CLK_MGR_REG_WR_ACCESS_OFFSET, old_en )
#define ccu_set_esub_lvm_enable()           ccu_set_lvm_enable( MM_IO_BASE_ESUB_CLK + ESUB_CLK_MGR_REG_LVM_EN_OFFSET )
#define ccu_wait_esub_lvm_ready()           ccu_wait_lvm_ready( MM_IO_BASE_ESUB_CLK + ESUB_CLK_MGR_REG_LVM_EN_OFFSET )
#define ccu_is_esub_lvm_ready(timeout)      ccu_is_lvm_ready( MM_IO_BASE_ESUB_CLK + ESUB_CLK_MGR_REG_LVM_EN_OFFSET, timeout )


/* Functions */
static inline uint32_t ccu_unlock_clk_manager( uint32_t addr )
{
   uint32_t old_en;
   uint32_t access;
   access = CHAL_REG_READ32( addr );
   old_en = access & CCU_WR_ACCESS_ENABLE;    /* save the existing ACCESS ENABLE bit */
   access &= CCU_WR_ACCESS_MODE_MASK;         /* retaining the access mode bit       */
   access |= CCU_WR_ACCESS_PASSWORD | CCU_WR_ACCESS_ENABLE;
   CHAL_REG_WRITE32( addr, access );
   return old_en;
}

static inline void ccu_lock_clk_manager( uint32_t addr )
{
   uint32_t access;
   
   access = CHAL_REG_READ32( addr );
   access &= CCU_WR_ACCESS_MODE_MASK;   /* retaining the access mode bit */
   access |= CCU_WR_ACCESS_PASSWORD;
   CHAL_REG_WRITE32( addr, access );
}

static inline void ccu_restore_clk_manager( uint32_t addr, uint32_t old_en )
{
   uint32_t access;
   
   access = CHAL_REG_READ32( addr );
   access &= CCU_WR_ACCESS_MODE_MASK;         /* retaining the access mode bit       */
   access |= CCU_WR_ACCESS_PASSWORD;
   access |= (old_en & CCU_WR_ACCESS_ENABLE); /* restore the given ACCESS ENABLE bit */
   CHAL_REG_WRITE32( addr, access );
}

static inline void ccu_set_lvm_enable( uint32_t addr )
{
   ccu_set_reg_field( addr, CCU_POLICY_CONFIG_EN_MASK, 0, CCU_POLICY_CONFIG_EN_MASK);
}

static inline void ccu_wait_lvm_ready( uint32_t addr )
{
   /* wait until all existing policy configurations are complete */
   while ( (CHAL_REG_READ32(addr) & CCU_POLICY_CONFIG_EN_MASK ) != 0 );
}

static inline uint32_t ccu_is_lvm_ready( uint32_t addr, int timeout )
{
   return( ccu_wait_for_mask_clear( addr, CCU_POLICY_CONFIG_EN_MASK, timeout ) );
}

static inline void ccu_set_kpm_policy_freq( ccu_policy_num_e policy_num, ccu_kpm_policy_freq_e freq )
{
   uint32_t access;
   
   /* enable access */
   access = ccu_unlock_kpm_clk_mgr();
   
   /* enable sw update */
   ccu_set_kpm_lvm_enable();
   
   /* wait until all existing policy configurations are complete */
   ccu_wait_kpm_lvm_ready();
   
   /* set frequency policy */
   ccu_set_reg_field( MM_IO_BASE_KPM_CLK + KPM_CLK_MGR_REG_POLICY_FREQ_OFFSET,             /* addr  */
                           KPM_CLK_MGR_REG_POLICY_FREQ_POLICY0_FREQ_MASK << (8 * policy_num),   /* mask  */
                           8 * policy_num,                                                      /* shift */
                           (uint32_t)freq ) ;                                                   /* value */

   /* trigger go */
   ccu_set_reg_field( MM_IO_BASE_KPM_CLK + KPM_CLK_MGR_REG_POLICY_CTL_OFFSET,
                           KPM_CLK_MGR_REG_POLICY_CTL_GO_ATL_MASK | KPM_CLK_MGR_REG_POLICY_CTL_GO_MASK,
                           0,
                           KPM_CLK_MGR_REG_POLICY_CTL_GO_ATL_MASK | KPM_CLK_MGR_REG_POLICY_CTL_GO_MASK );
   
   /* wait until complete */
   while( ccu_get_bit( MM_IO_BASE_KPM_CLK + KPM_CLK_MGR_REG_POLICY_CTL_OFFSET, KPM_CLK_MGR_REG_POLICY_CTL_GO_SHIFT ) != 0 );
   
   /* restore access */
   ccu_restore_kpm_clk_mgr(access);
}

static inline ccu_kpm_policy_freq_e ccu_get_kpm_policy_freq( uint32_t policy_num )
{
   uint32_t access;
   uint32_t freq_id;
   
   /* enable access */
   access = ccu_unlock_kpm_clk_mgr();
   
   /* set frequency policy */
   freq_id = ccu_get_reg_field( MM_IO_BASE_KPM_CLK + KPM_CLK_MGR_REG_POLICY_FREQ_OFFSET,            /* addr  */
                                     KPM_CLK_MGR_REG_POLICY_FREQ_POLICY0_FREQ_MASK << (8 * policy_num),  /* mask  */
                                     8 * policy_num ) ;                                                  /* shift */
   
   /* restore access */
   ccu_restore_kpm_clk_mgr(access);
   
   return (ccu_kpm_policy_freq_e)freq_id;
}

static inline void ccu_set_kps_policy_freq( ccu_policy_num_e policy_num, ccu_kps_policy_freq_e freq )
{
   uint32_t access;
   
   /* enable access */
   access = ccu_unlock_kps_clk_mgr();
   
   /* enable sw update */
   ccu_set_kps_lvm_enable();
   
   /* wait until all existing policy configurations are complete */
   ccu_wait_kps_lvm_ready();
   
   /* set frequency policy */
   ccu_set_reg_field( MM_IO_BASE_SLV_CLK + NFIX(KPS_CLK_MGR_REG_POLICY_FREQ_OFFSET),            /* addr  */
                           NFIX(KPS_CLK_MGR_REG_POLICY_FREQ_POLICY0_FREQ_MASK) << (8 * policy_num),  /* mask  */
                           8 * policy_num,                                                      /* shift */
                           (uint32_t)freq ) ;                                                   /* value */

   /* trigger go */
   ccu_set_reg_field( MM_IO_BASE_SLV_CLK + NFIX(KPS_CLK_MGR_REG_POLICY_CTL_OFFSET),
                           NFIX(KPS_CLK_MGR_REG_POLICY_CTL_GO_ATL_MASK) | NFIX(KPS_CLK_MGR_REG_POLICY_CTL_GO_MASK),
                           0,
                           NFIX(KPS_CLK_MGR_REG_POLICY_CTL_GO_ATL_MASK) | NFIX(KPS_CLK_MGR_REG_POLICY_CTL_GO_MASK) );
   
   /* wait until complete */
   while( ccu_get_bit( MM_IO_BASE_SLV_CLK + NFIX(KPS_CLK_MGR_REG_POLICY_CTL_OFFSET), NFIX(KPS_CLK_MGR_REG_POLICY_CTL_GO_SHIFT) ) != 0 );
   
   /* restore access */
   ccu_restore_kps_clk_mgr(access);
}

static inline ccu_kps_policy_freq_e ccu_get_kps_policy_freq( ccu_policy_num_e policy_num )
{
   uint32_t access;
   uint32_t freq_id;
   
   /* enable access */
   access = ccu_unlock_kps_clk_mgr();
   
   /* set frequency policy */
   freq_id = ccu_get_reg_field( MM_IO_BASE_SLV_CLK + NFIX(KPS_CLK_MGR_REG_POLICY_FREQ_OFFSET),            /* addr  */
                                     NFIX(KPS_CLK_MGR_REG_POLICY_FREQ_POLICY0_FREQ_MASK) << (8 * policy_num),  /* mask  */
                                     8 * policy_num ) ;                                                  /* shift */
   
   /* restore access */
   ccu_restore_kps_clk_mgr(access);
   
   return (ccu_kps_policy_freq_e)freq_id;
}

static inline void ccu_set_khubaon_policy_freq( ccu_policy_num_e policy_num, ccu_khubaon_policy_freq_e freq )
{
   uint32_t access;
   
   /* enable access */
   access = ccu_unlock_khubaon_clk_mgr();
   
   /* enable sw update */
   ccu_set_khubaon_lvm_enable();

   /* wait until all existing policy configurations are complete */
   ccu_wait_khubaon_lvm_ready();
   
   /* set frequency policy */
   ccu_set_reg_field( MM_IO_BASE_AON_CLK + KHUBAON_CLK_MGR_REG_POLICY_FREQ_OFFSET,            /* addr  */
                           KHUBAON_CLK_MGR_REG_POLICY_FREQ_POLICY0_FREQ_MASK << (8 * policy_num),  /* mask  */
                           8 * policy_num,                                                         /* shift */
                           (uint32_t)freq ) ;                                                      /* value */

   /* trigger go */
   ccu_set_reg_field( MM_IO_BASE_AON_CLK + KHUBAON_CLK_MGR_REG_POLICY_CTL_OFFSET,
                           KHUBAON_CLK_MGR_REG_POLICY_CTL_GO_ATL_MASK | KHUBAON_CLK_MGR_REG_POLICY_CTL_GO_MASK,
                           0,
                           KHUBAON_CLK_MGR_REG_POLICY_CTL_GO_ATL_MASK | KHUBAON_CLK_MGR_REG_POLICY_CTL_GO_MASK );
   
   /* wait until complete */
   while( ccu_get_bit( MM_IO_BASE_AON_CLK + KHUBAON_CLK_MGR_REG_POLICY_CTL_OFFSET, KHUBAON_CLK_MGR_REG_POLICY_CTL_GO_SHIFT ) != 0 );

   /* restore access */
   ccu_restore_khubaon_clk_mgr(access);
}

static inline ccu_khubaon_policy_freq_e ccu_get_khubaon_policy_freq( ccu_policy_num_e policy_num )
{
   uint32_t access;
   uint32_t freq;
   
   /* enable access */
   access = ccu_unlock_khubaon_clk_mgr();
   
   /* get frequency policy */
   freq = ccu_get_reg_field( MM_IO_BASE_AON_CLK + KHUBAON_CLK_MGR_REG_POLICY_FREQ_OFFSET,            /* addr  */
                                  KHUBAON_CLK_MGR_REG_POLICY_FREQ_POLICY0_FREQ_MASK << (8 * policy_num),  /* mask  */
                                  8 * policy_num ) ;                                                      /* shift */

   /* restore access */
   ccu_restore_khubaon_clk_mgr(access);
   
   return (ccu_khubaon_policy_freq_e)freq;
}

static inline void ccu_set_khub_policy_freq( ccu_policy_num_e policy_num, ccu_khub_policy_freq_e freq )
{   
   uint32_t access;
   
   /* enable access */
   access = ccu_unlock_khub_clk_mgr();
   
   /* enable sw update */
   ccu_set_khub_lvm_enable();
   
   /* wait until all existing policy configurations are complete */
   ccu_wait_khub_lvm_ready();
   
   /* set frequency policy */
   ccu_set_reg_field( MM_IO_BASE_HUB_CLK + KHUB_CLK_MGR_REG_POLICY_FREQ_OFFSET,            /* addr  */
                           KHUB_CLK_MGR_REG_POLICY_FREQ_POLICY0_FREQ_MASK << (8 * policy_num),  /* mask  */
                           8 * policy_num,                                                      /* shift */
                           (uint32_t)freq ) ;                                                   /* value */

   /* trigger go */
   ccu_set_reg_field( MM_IO_BASE_HUB_CLK + KHUB_CLK_MGR_REG_POLICY_CTL_OFFSET,
                           KHUB_CLK_MGR_REG_POLICY_CTL_GO_ATL_MASK | KHUB_CLK_MGR_REG_POLICY_CTL_GO_MASK,
                           0,
                           KHUB_CLK_MGR_REG_POLICY_CTL_GO_ATL_MASK | KHUB_CLK_MGR_REG_POLICY_CTL_GO_MASK );
   
   /* wait until complete */
   while( ccu_get_bit( MM_IO_BASE_HUB_CLK + KHUB_CLK_MGR_REG_POLICY_CTL_OFFSET, KHUB_CLK_MGR_REG_POLICY_CTL_GO_SHIFT ) != 0 );
   
   /* restore access */
   ccu_restore_khub_clk_mgr(access);
}

static inline ccu_khub_policy_freq_e ccu_get_khub_policy_freq( ccu_policy_num_e policy_num )
{   
   uint32_t access;
   uint32_t freq;
   
   /* enable access */
   access = ccu_unlock_khub_clk_mgr();
   
   /* get frequency policy */
   freq = ccu_get_reg_field( MM_IO_BASE_HUB_CLK + KHUB_CLK_MGR_REG_POLICY_FREQ_OFFSET,            /* addr  */
                                  KHUB_CLK_MGR_REG_POLICY_FREQ_POLICY0_FREQ_MASK << (8 * policy_num),  /* mask  */
                                  8 * policy_num ) ;                                                   /* shift */

   /* restore access */
   ccu_restore_khub_clk_mgr(access);
   
   return (ccu_khub_policy_freq_e)freq;
}

static inline void ccu_cfg_kpm_policy_mask( ccu_policy_num_e policy_num, ccu_kpm_policy_mask_e policy_type, uint32_t val )
{
   uint32_t addr;
   uint32_t access;
   
   /* enable access */
   access = ccu_unlock_kpm_clk_mgr();
   
   /* enable sw update */
   ccu_set_kpm_lvm_enable();
   
   /* wait until all existing policy configurations are complete */
   ccu_wait_kpm_lvm_ready();
   
   addr  = MM_IO_BASE_KPM_CLK + KPM_CLK_MGR_REG_POLICY0_MASK_OFFSET;
   addr += (sizeof(uint32_t) * policy_num);

   ccu_set_bit( addr, policy_type, val );
   
   /* trigger go */
   ccu_set_reg_field( MM_IO_BASE_KPM_CLK + KPM_CLK_MGR_REG_POLICY_CTL_OFFSET,
                           KPM_CLK_MGR_REG_POLICY_CTL_GO_ATL_MASK | KPM_CLK_MGR_REG_POLICY_CTL_GO_MASK,
                           0,
                           KPM_CLK_MGR_REG_POLICY_CTL_GO_ATL_MASK | KPM_CLK_MGR_REG_POLICY_CTL_GO_MASK );
   
   /* wait until complete */
   while( ccu_get_bit( MM_IO_BASE_KPM_CLK + KPM_CLK_MGR_REG_POLICY_CTL_OFFSET, KPM_CLK_MGR_REG_POLICY_CTL_GO_SHIFT ) != 0 );
   
   /* restore access */
   ccu_restore_kpm_clk_mgr(access);
}

static inline void ccu_set_kpm_policy_mask( ccu_policy_num_e policy_num, ccu_kpm_policy_mask_e policy_type )
{
   ccu_cfg_kpm_policy_mask( policy_num, policy_type, 1 );
}

static inline void ccu_clr_kpm_policy_mask( ccu_policy_num_e policy_num, ccu_kpm_policy_mask_e policy_type )
{
   ccu_cfg_kpm_policy_mask( policy_num, policy_type, 0 );
}

static inline uint32_t ccu_get_kpm_policy_mask( ccu_policy_num_e policy_num, ccu_kpm_policy_mask_e policy_type )
{
   uint32_t addr;
   uint32_t bit;
   
   addr  = MM_IO_BASE_KPM_CLK + KPM_CLK_MGR_REG_POLICY0_MASK_OFFSET;
   addr += (sizeof(uint32_t) * policy_num);
   bit   = ccu_get_bit( addr, policy_type );
   
   return bit;
}

static inline ccu_policy_num_e ccu_get_kpm_active_policy( void )
{
   return (ccu_policy_num_e)ccu_get_reg_field( MM_IO_BASE_KPM_CLK + KPM_CLK_MGR_REG_POLICY_DBG_OFFSET,  /* addr */
                                                         KPM_CLK_MGR_REG_POLICY_DBG_ACT_POLICY_MASK,              /* mask */
                                                         KPM_CLK_MGR_REG_POLICY_DBG_ACT_POLICY_SHIFT );           /* shift */
}


static inline void ccu_cfg_kps_policy_mask( ccu_policy_num_e policy_num, ccu_kps_policy_mask_e policy_type, uint32_t val )
{
   uint32_t addr;
   uint32_t access;
   
   /* enable access */
   access = ccu_unlock_kps_clk_mgr();
   
   /* enable sw update */
   ccu_set_kps_lvm_enable();
   
   /* wait until all existing policy configurations are complete */
   ccu_wait_kps_lvm_ready();

   if (policy_type >= ccu_kps_policy_mask_uartb5)
   {
      addr  = MM_IO_BASE_SLV_CLK + NFIX(KPS_CLK_MGR_REG_POLICY0_MASK2_OFFSET);
      policy_type -= ccu_kps_policy_mask_uartb5;
   }
   else
   {
      addr  = MM_IO_BASE_SLV_CLK + NFIX(KPS_CLK_MGR_REG_POLICY0_MASK_OFFSET);
   }
   addr += (sizeof(uint32_t) * policy_num);

   ccu_set_bit( addr, policy_type, val );
   
   /* trigger go */
   ccu_set_reg_field( MM_IO_BASE_SLV_CLK + NFIX(KPS_CLK_MGR_REG_POLICY_CTL_OFFSET),
                           NFIX(KPS_CLK_MGR_REG_POLICY_CTL_GO_ATL_MASK) | NFIX(KPS_CLK_MGR_REG_POLICY_CTL_GO_MASK),
                           0,
                           NFIX(KPS_CLK_MGR_REG_POLICY_CTL_GO_ATL_MASK) | NFIX(KPS_CLK_MGR_REG_POLICY_CTL_GO_MASK) );
   
   /* wait until complete */
   while( ccu_get_bit( MM_IO_BASE_SLV_CLK + NFIX(KPS_CLK_MGR_REG_POLICY_CTL_OFFSET), NFIX(KPS_CLK_MGR_REG_POLICY_CTL_GO_SHIFT) ) != 0 );
   
   /* restore access */
   ccu_restore_kps_clk_mgr(access);
}

static inline void ccu_set_kps_policy_mask( ccu_policy_num_e policy_num, ccu_kps_policy_mask_e policy_type )
{
   ccu_cfg_kps_policy_mask( policy_num, policy_type, 1 );
}

static inline void ccu_clr_kps_policy_mask( ccu_policy_num_e policy_num, ccu_kps_policy_mask_e policy_type )
{
   ccu_cfg_kps_policy_mask( policy_num, policy_type, 0 );
}

static inline uint32_t ccu_get_kps_policy_mask( ccu_policy_num_e policy_num, ccu_kps_policy_mask_e policy_type )
{
   uint32_t addr;
   uint32_t bit;
   
   if (policy_type >= ccu_kps_policy_mask_uartb5)
   {
      addr  = MM_IO_BASE_SLV_CLK + NFIX(KPS_CLK_MGR_REG_POLICY0_MASK2_OFFSET);
      policy_type -= ccu_kps_policy_mask_uartb5;
   }
   else
   {
      addr  = MM_IO_BASE_SLV_CLK + NFIX(KPS_CLK_MGR_REG_POLICY0_MASK_OFFSET);
   }
   addr += (sizeof(uint32_t) * policy_num);
   bit   = ccu_get_bit( addr, policy_type );

   return bit;
}

static inline ccu_policy_num_e ccu_get_kps_active_policy( void )
{
   return (ccu_policy_num_e)ccu_get_reg_field( MM_IO_BASE_SLV_CLK + NFIX(KPS_CLK_MGR_REG_POLICY_DBG_OFFSET),  /* addr */
                                                         NFIX(KPS_CLK_MGR_REG_POLICY_DBG_ACT_POLICY_MASK),              /* mask */
                                                         NFIX(KPS_CLK_MGR_REG_POLICY_DBG_ACT_POLICY_SHIFT) );           /* shift */
}

static inline void ccu_cfg_khubaon_policy_mask( ccu_policy_num_e policy_num, ccu_khubaon_policy_mask_e policy_type, uint32_t val )
{
   uint32_t addr;
   uint32_t access;
   
   /* enable access */
   access = ccu_unlock_khubaon_clk_mgr();
   
   /* enable sw update */
   ccu_set_khubaon_lvm_enable();

   /* wait until all existing policy configurations are complete */
   ccu_wait_khubaon_lvm_ready();
   
   addr  = MM_IO_BASE_AON_CLK + KHUBAON_CLK_MGR_REG_POLICY0_MASK1_OFFSET;
   addr += (sizeof(uint32_t) * policy_num);
   ccu_set_bit( addr, policy_type, val );

   /* trigger go */
   ccu_set_reg_field( MM_IO_BASE_AON_CLK + KHUBAON_CLK_MGR_REG_POLICY_CTL_OFFSET,
                           KHUBAON_CLK_MGR_REG_POLICY_CTL_GO_ATL_MASK | KHUBAON_CLK_MGR_REG_POLICY_CTL_GO_MASK,
                           0,
                           KHUBAON_CLK_MGR_REG_POLICY_CTL_GO_ATL_MASK | KHUBAON_CLK_MGR_REG_POLICY_CTL_GO_MASK );
   
   /* wait until complete */
   while( ccu_get_bit( MM_IO_BASE_AON_CLK + KHUBAON_CLK_MGR_REG_POLICY_CTL_OFFSET, KHUBAON_CLK_MGR_REG_POLICY_CTL_GO_SHIFT ) != 0 );

   /* restore access */
   ccu_restore_khubaon_clk_mgr(access);
}

static inline void ccu_set_khubaon_policy_mask( ccu_policy_num_e policy_num, ccu_khubaon_policy_mask_e policy_type )
{
   ccu_cfg_khubaon_policy_mask( policy_num, policy_type, 1 );
}

static inline void ccu_clr_khubaon_policy_mask( ccu_policy_num_e policy_num, ccu_khubaon_policy_mask_e policy_type )
{
   ccu_cfg_khubaon_policy_mask( policy_num, policy_type, 0 );
}

static inline uint32_t ccu_get_khubaon_policy_mask( ccu_policy_num_e policy_num, ccu_khubaon_policy_mask_e policy_type )
{
   uint32_t addr;
   uint32_t bit;
   
   addr  = MM_IO_BASE_AON_CLK + KHUBAON_CLK_MGR_REG_POLICY0_MASK1_OFFSET;
   addr += (sizeof(uint32_t) * policy_num);
   bit   = ccu_get_bit( addr, policy_type );

   return bit;
}

static inline ccu_policy_num_e ccu_get_khubaon_active_policy( void )
{
   return (ccu_policy_num_e)ccu_get_reg_field( MM_IO_BASE_AON_CLK + KHUBAON_CLK_MGR_REG_POLICY_DBG_OFFSET,  /* addr */
                                                         KHUBAON_CLK_MGR_REG_POLICY_DBG_ACT_POLICY_MASK,              /* mask */
                                                         KHUBAON_CLK_MGR_REG_POLICY_DBG_ACT_POLICY_SHIFT );           /* shift */
}


static inline void ccu_cfg_khub_policy_mask( ccu_policy_num_e policy_num, ccu_khub_policy_mask_e policy_type, uint32_t val )
{
   uint32_t addr;
   uint32_t access;
   
   /* enable access */
   access = ccu_unlock_khub_clk_mgr();
   
   /* enable sw update */
   ccu_set_khub_lvm_enable();
   
   /* wait until all existing policy configurations are complete */
   ccu_wait_khub_lvm_ready();

   if (policy_type >= ccu_khub_policy_mask_reserved32)
   {
      addr         = MM_IO_BASE_HUB_CLK + KHUB_CLK_MGR_REG_POLICY0_MASK2_OFFSET;
      policy_type -= ccu_khub_policy_mask_reserved32;
   }
   else
   {
      addr  = MM_IO_BASE_HUB_CLK + KHUB_CLK_MGR_REG_POLICY0_MASK1_OFFSET;
   }
   addr += (sizeof(uint32_t) * policy_num);

   ccu_set_bit( addr, policy_type, val );

   /* trigger go */
   ccu_set_reg_field( MM_IO_BASE_HUB_CLK + KHUB_CLK_MGR_REG_POLICY_CTL_OFFSET,
                           KHUB_CLK_MGR_REG_POLICY_CTL_GO_ATL_MASK | KHUB_CLK_MGR_REG_POLICY_CTL_GO_MASK,
                           0,
                           KHUB_CLK_MGR_REG_POLICY_CTL_GO_ATL_MASK | KHUB_CLK_MGR_REG_POLICY_CTL_GO_MASK );
   
   /* wait until complete */
   while( ccu_get_bit( MM_IO_BASE_HUB_CLK + KHUB_CLK_MGR_REG_POLICY_CTL_OFFSET, KHUB_CLK_MGR_REG_POLICY_CTL_GO_SHIFT ) != 0 );
   
   /* restore access */
   ccu_restore_khub_clk_mgr(access);
}

static inline void ccu_set_khub_policy_mask( ccu_policy_num_e policy_num, ccu_khub_policy_mask_e policy_type )
{
   ccu_cfg_khub_policy_mask( policy_num, policy_type, 1 );
}

static inline void ccu_clr_khub_policy_mask( ccu_policy_num_e policy_num, ccu_khub_policy_mask_e policy_type )
{
   ccu_cfg_khub_policy_mask( policy_num, policy_type, 0 );
}

static inline uint32_t ccu_get_khub_policy_mask( ccu_policy_num_e policy_num, ccu_khub_policy_mask_e policy_type )
{
   uint32_t addr;
   uint32_t bit;
   
   if (policy_type >= ccu_khub_policy_mask_reserved32)
   {
      addr         = MM_IO_BASE_HUB_CLK + KHUB_CLK_MGR_REG_POLICY0_MASK2_OFFSET;
      policy_type -= ccu_khub_policy_mask_reserved32;
   }
   else
   {
      addr  = MM_IO_BASE_HUB_CLK + KHUB_CLK_MGR_REG_POLICY0_MASK1_OFFSET;
   }
   addr += (sizeof(uint32_t) * policy_num);

   bit = ccu_get_bit( addr, policy_type );
   
   return bit;
}

static inline ccu_policy_num_e ccu_get_khub_active_policy( void )
{
   return (ccu_policy_num_e)ccu_get_reg_field( MM_IO_BASE_HUB_CLK + KHUB_CLK_MGR_REG_POLICY_DBG_OFFSET,  /* addr */
                                                         KHUB_CLK_MGR_REG_POLICY_DBG_ACT_POLICY_MASK,              /* mask */
                                                         KHUB_CLK_MGR_REG_POLICY_DBG_ACT_POLICY_SHIFT );           /* shift */
}

static inline void ccu_set_kproc_axi_div( uint32_t div )
{
   uint32_t access;
   uint32_t arm_div;

   /* enable access */
   access = ccu_unlock_kproc_clk_mgr();
   
   /* enable arm switch policy override */
   if (div > 1)
   {
      ccu_set_bit( MM_IO_BASE_PROC_CLK + KPROC_CLK_MGR_REG_CLKGATE_DBG_OFFSET,
                        KPROC_CLK_MGR_REG_CLKGATE_DBG_ARM_SWITCH_POLICY_OVERRIDE_VALUE_SHIFT,
                        1 );
   }
   else
   {
      ccu_set_bit( MM_IO_BASE_PROC_CLK + KPROC_CLK_MGR_REG_CLKGATE_DBG_OFFSET,
                        KPROC_CLK_MGR_REG_CLKGATE_DBG_ARM_SWITCH_POLICY_OVERRIDE_VALUE_SHIFT,
                        0 );
   }

   /* set new arm_div */
   arm_div  = CHAL_REG_READ32(MM_IO_BASE_PROC_CLK + KPROC_CLK_MGR_REG_ARM_DIV_OFFSET);
   arm_div &= ~KPROC_CLK_MGR_REG_ARM_DIV_ARM_SWITCH_DIV_MASK;
   arm_div &= ~KPROC_CLK_MGR_REG_ARM_DIV_ARM_SWITCH_DIV_OVERRIDE_MASK;
   arm_div &= ~KPROC_CLK_MGR_REG_ARM_DIV_ARM_PLL_SELECT_MASK;
   
   /* simply use default settings for divide by 1 or divide by 2 */
   if (div > 1)
   {
      arm_div |= ((div << KPROC_CLK_MGR_REG_ARM_DIV_ARM_SWITCH_DIV_SHIFT) & KPROC_CLK_MGR_REG_ARM_DIV_ARM_SWITCH_DIV_MASK);
      arm_div |= KPROC_CLK_MGR_REG_ARM_DIV_ARM_SWITCH_DIV_OVERRIDE_MASK;
   }
   CHAL_REG_WRITE32(MM_IO_BASE_PROC_CLK + KPROC_CLK_MGR_REG_ARM_DIV_OFFSET, arm_div);

   /* trigger ARM_SEG_TRG */
   CHAL_REG_WRITE32(MM_IO_BASE_PROC_CLK + KPROC_CLK_MGR_REG_ARM_SEG_TRG_OFFSET, KPROC_CLK_MGR_REG_ARM_SEG_TRG_ARM_TRIGGER_MASK);
   while ( ccu_get_bit( MM_IO_BASE_PROC_CLK + KPROC_CLK_MGR_REG_ARM_SEG_TRG_OFFSET, KPROC_CLK_MGR_REG_ARM_SEG_TRG_ARM_TRIGGER_SHIFT ) != 0 );
      
   /* restore previous access */
   ccu_restore_kproc_clk_mgr(access);
}

static inline uint32_t ccu_get_kproc_axi_div( ccu_kproc_policy_freq_e freq_id )
{
   
   uint32_t axi_div;
   
   /* retrieve frequency id for given policy id */
   if (freq_id <= ccu_kproc_policy_freq_156_156_78_0V9)
   {
      axi_div = 0;   /* divide by 1 */   
   }
   else if (freq_id < ccu_kproc_policy_freq_aclk_aclkdiv2_aclkdiv3)
   {
      axi_div = 1;   /* divide by 2 */
   }
   else if ( (freq_id == ccu_kproc_policy_freq_aclkh_aclkhdiv2_aclkhdiv3) &&
             (CHAL_REG_READ32(MM_IO_BASE_PROC_CLK + KPROC_CLK_MGR_REG_ARM_SWITCH_TRIGGER_OFFSET) & KPROC_CLK_MGR_REG_ARM_SWITCH_TRIGGER_ARM_SWITCH_CLK_OVERRIDE_MASK) )
   {
     axi_div = ccu_get_kproc_fid7_arm_swt_div();
   }
   else
   {
      /* check for presence of overrides */
      axi_div  = CHAL_REG_READ32(MM_IO_BASE_PROC_CLK + KPROC_CLK_MGR_REG_ARM_DIV_OFFSET);
      if ((axi_div & KPROC_CLK_MGR_REG_ARM_DIV_ARM_SWITCH_DIV_OVERRIDE_MASK) != 0)
      {
         axi_div   = axi_div & KPROC_CLK_MGR_REG_ARM_DIV_ARM_SWITCH_DIV_MASK;
         axi_div >>= KPROC_CLK_MGR_REG_ARM_DIV_ARM_SWITCH_DIV_SHIFT;
      }
      else
      {
         axi_div = 1; /* divide by 2 */
      }
   }
   
   return axi_div;
   
}

static inline int ccu_set_kproc_fid7_arm_swt_div( uint32_t div )
{
	uint32_t regval;

	/* if we're in freq ID 7, exit -- can't set while we're in it */
	regval = CHAL_REG_READ32( MM_IO_BASE_PROC_CLK + KPROC_CLK_MGR_REG_POLICY_DBG_OFFSET );
	regval = (regval & KPROC_CLK_MGR_REG_POLICY_DBG_ACT_FREQ_MASK) >> KPROC_CLK_MGR_REG_POLICY_DBG_ACT_FREQ_SHIFT;
   
	if ( regval == ccu_freq_id_7 )
	{
		return -1;
	}
	else
	{
		uint32_t access;
		access = ccu_unlock_kproc_clk_mgr();
		CHAL_REG_WRITE32(MM_IO_BASE_PROC_CLK + KPROC_CLK_MGR_REG_ARM_SWITCH_TRIGGER_OFFSET, 0);
		div = (div << KPROC_CLK_MGR_REG_ARM_SWITCH_DIV_ARM_SWITCH_CLK_DIV_SHIFT) & KPROC_CLK_MGR_REG_ARM_SWITCH_DIV_ARM_SWITCH_CLK_DIV_MASK;
		CHAL_REG_WRITE32(MM_IO_BASE_PROC_CLK + KPROC_CLK_MGR_REG_ARM_SWITCH_DIV_OFFSET, div);
		CHAL_REG_WRITE32(MM_IO_BASE_PROC_CLK + KPROC_CLK_MGR_REG_ARM_SWITCH_TRIGGER_OFFSET, KPROC_CLK_MGR_REG_ARM_SWITCH_TRIGGER_ARM_SWITCH_CLK_OVERRIDE_MASK);
		ccu_restore_kproc_clk_mgr( access );
		return 0;
   }
}

static inline uint32_t ccu_get_kproc_fid7_arm_swt_div( void )
{
	uint32_t regval;
	
	regval   = CHAL_REG_READ32( MM_IO_BASE_PROC_CLK + KPROC_CLK_MGR_REG_ARM_SWITCH_DIV_OFFSET );
	regval  &= KPROC_CLK_MGR_REG_ARM_SWITCH_DIV_ARM_SWITCH_CLK_DIV_MASK;
	regval >>= KPROC_CLK_MGR_REG_ARM_SWITCH_DIV_ARM_SWITCH_CLK_DIV_SHIFT;
	return regval;
}

static inline void ccu_set_kproc_mdiv( ccu_kproc_policy_freq_e fid, uint32_t mdiv )
{
	uint32_t access;
	uint32_t mdiv_mask;
	uint32_t mdiv_shift;
	uint32_t loaden_mask;
	uint32_t loaden_shift;
	uint32_t reg_val;
	uint32_t reg_addr;
   
	if ( fid >= ccu_kproc_policy_freq_aclk_aclkdiv2_aclkdiv3 )
	{
		if ( fid == ccu_kproc_policy_freq_aclk_aclkdiv2_aclkdiv3 )
		{
			reg_addr = MM_IO_BASE_PROC_CLK + KPROC_CLK_MGR_REG_PLLARMC_OFFSET;
			loaden_mask = KPROC_CLK_MGR_REG_PLLARMC_PLLARM_LOAD_EN_MASK;
			loaden_shift = KPROC_CLK_MGR_REG_PLLARMC_PLLARM_LOAD_EN_SHIFT;
			mdiv_mask = KPROC_CLK_MGR_REG_PLLARMC_PLLARM_MDIV_MASK;
			mdiv_shift = KPROC_CLK_MGR_REG_PLLARMC_PLLARM_MDIV_SHIFT;

		}
		else
		{
			reg_addr = MM_IO_BASE_PROC_CLK + KPROC_CLK_MGR_REG_PLLARMCTRL5_OFFSET;
			loaden_mask = KPROC_CLK_MGR_REG_PLLARMCTRL5_PLLARM_H_LOAD_EN_MASK;
			loaden_shift = KPROC_CLK_MGR_REG_PLLARMCTRL5_PLLARM_H_LOAD_EN_SHIFT;
			mdiv_mask = KPROC_CLK_MGR_REG_PLLARMCTRL5_PLLARM_H_MDIV_MASK;
			mdiv_shift = KPROC_CLK_MGR_REG_PLLARMCTRL5_PLLARM_H_MDIV_SHIFT;
		}

		/* limit input range */
		mdiv = (mdiv << mdiv_shift) & mdiv_mask;

		access = ccu_unlock_kproc_clk_mgr();

		reg_val  = CHAL_REG_READ32( reg_addr );
		reg_val &= ~loaden_mask;
		reg_val &= ~mdiv_mask;
		reg_val |= mdiv;
		CHAL_REG_WRITE32( reg_addr, reg_val );
		CHAL_REG_WRITE32( reg_addr, (reg_val |  loaden_mask) );
		CHAL_REG_WRITE32( reg_addr, (reg_val & ~loaden_mask) );
		while( ccu_get_bit( reg_addr, loaden_shift ) != 0 );
		
		ccu_restore_kproc_clk_mgr( access );
   }
}

static inline uint32_t ccu_get_kproc_mdiv( ccu_kproc_policy_freq_e fid )
{
	uint32_t mdiv = 0;
	uint32_t reg_addr;
	uint32_t mdiv_mask;
	uint32_t mdiv_shift;

	if ( fid >= ccu_kproc_policy_freq_aclk_aclkdiv2_aclkdiv3 )
	{
		if ( fid == ccu_kproc_policy_freq_aclk_aclkdiv2_aclkdiv3 )
		{
			reg_addr = MM_IO_BASE_PROC_CLK + KPROC_CLK_MGR_REG_PLLARMC_OFFSET;
			mdiv_mask = KPROC_CLK_MGR_REG_PLLARMC_PLLARM_MDIV_MASK;
			mdiv_shift = KPROC_CLK_MGR_REG_PLLARMC_PLLARM_MDIV_SHIFT;

		}
		else
		{
			reg_addr = MM_IO_BASE_PROC_CLK + KPROC_CLK_MGR_REG_PLLARMCTRL5_OFFSET;
			mdiv_mask = KPROC_CLK_MGR_REG_PLLARMCTRL5_PLLARM_H_MDIV_MASK;
			mdiv_shift = KPROC_CLK_MGR_REG_PLLARMCTRL5_PLLARM_H_MDIV_SHIFT;
		}

		mdiv = (CHAL_REG_READ32( reg_addr ) & mdiv_mask) >> mdiv_shift;
	}

	return mdiv;
}

static inline void ccu_set_kproc_pl310_div(uint32_t div)
{
	uint32_t access, reg_addr, reg_val;
	
	/* enable access */
	access = ccu_unlock_kproc_clk_mgr();

	reg_addr = MM_IO_BASE_PROC_CLK + KPROC_CLK_MGR_REG_PL310_DIV_OFFSET;
	reg_val = CHAL_REG_READ32(reg_addr);
	reg_val &= ~KPROC_CLK_MGR_REG_PL310_DIV_ARM_PL310_DRAM_DIV_MASK;
	reg_val |= (div & KPROC_CLK_MGR_REG_PL310_DIV_ARM_PL310_DRAM_DIV_MASK);
	CHAL_REG_WRITE32(reg_addr, reg_val);

	reg_addr = MM_IO_BASE_PROC_CLK + KPROC_CLK_MGR_REG_PL310_TRIGGER_OFFSET;
	reg_val = CHAL_REG_READ32(reg_addr);
	reg_val |= (1 & KPROC_CLK_MGR_REG_PL310_TRIGGER_ARM_PL310_DRAM_OVERRIDE_MASK);
	CHAL_REG_WRITE32(reg_addr, reg_val);

	/* restore access */
	ccu_restore_kproc_clk_mgr(access);
}

static inline uint32_t ccu_get_kproc_pl310_div(void)
{
	uint32_t reg_addr, reg_val;

	reg_addr = MM_IO_BASE_PROC_CLK + KPROC_CLK_MGR_REG_PL310_DIV_OFFSET;
	reg_val = CHAL_REG_READ32(reg_addr) & KPROC_CLK_MGR_REG_PL310_DIV_ARM_PL310_DRAM_DIV_MASK;
	reg_val = reg_val >> KPROC_CLK_MGR_REG_PL310_DIV_ARM_PL310_DRAM_DIV_SHIFT;

	return reg_val;
}


static inline void ccu_set_kproc_policy_freq( ccu_policy_num_e policy_num, ccu_kproc_policy_freq_e freq )
{
	uint32_t access;
	uint32_t reg_val;
	uint32_t hz;
	uint32_t cur_axi_div;
	uint32_t new_axi_div;
	uint32_t cur_pid;
	uint32_t cur_fid;
	uint32_t mdiv_save = 0;

	/* get the current running policy # */
	reg_val = CHAL_REG_READ32( MM_IO_BASE_PROC_CLK + KPROC_CLK_MGR_REG_POLICY_DBG_OFFSET );
	cur_pid = (reg_val & KPROC_CLK_MGR_REG_POLICY_DBG_ACT_POLICY_MASK) >> KPROC_CLK_MGR_REG_POLICY_DBG_ACT_POLICY_SHIFT;
	cur_fid = (reg_val & KPROC_CLK_MGR_REG_POLICY_DBG_ACT_FREQ_MASK) >> KPROC_CLK_MGR_REG_POLICY_DBG_ACT_FREQ_SHIFT;

	/* find the current policy axi div */
	cur_axi_div = ccu_get_kproc_axi_div( cur_fid );

	/* find the A9 frequency to be switch to */
	hz = ccu_get_kproc_policy_freq_a9_hz( freq );

	/* determine the AXI div */
   if ( hz > 3*CCU_MAX_A9_AXI_FREQ )
   {
      new_axi_div = 3;  /* divide by 4 */
   }
	else if ( hz > 2*CCU_MAX_A9_AXI_FREQ )
	{
		new_axi_div = 2;	/* divide by 3 */
	}
	else if ( hz > CCU_MAX_A9_AXI_FREQ )
	{
		new_axi_div = 1;	/* divide by 2 */
	}
	else
	{
		new_axi_div = 0;	/* use default -- i.e. no overrides */
	}

	/* do not change the axi_div if new policy is not the current active */
	if ( cur_pid != policy_num )
	{
		new_axi_div = cur_axi_div;
	}

	/* if the axi div is non-default, then we've better scale down the post divider of the new freq ID - assuming there's a change in the axi div */
	if ( (new_axi_div != 0) && (new_axi_div != cur_axi_div) && (freq >= ccu_kproc_policy_freq_aclk_aclkdiv2_aclkdiv3) )
	{
		mdiv_save = ccu_get_kproc_mdiv( freq );
		ccu_set_kproc_mdiv( freq, 0x06 );
	}
	
	/* enable access */
	access = ccu_unlock_kproc_clk_mgr();

	/* enable sw update */
	ccu_set_kproc_lvm_enable();

	/* wait until all existing policy configurations are complete */
	ccu_wait_kproc_lvm_ready();

	/* set frequency policy */
	ccu_set_reg_field( MM_IO_BASE_PROC_CLK + KPROC_CLK_MGR_REG_POLICY_FREQ_OFFSET,          /* addr  */
							KPROC_CLK_MGR_REG_POLICY_FREQ_POLICY0_FREQ_MASK << (8 * policy_num), /* mask  */
							8 * policy_num,                                                      /* shift */
							(uint32_t)freq ) ;                                                   /* value */

	/* trigger go */
	ccu_set_reg_field( MM_IO_BASE_PROC_CLK + KPROC_CLK_MGR_REG_POLICY_CTL_OFFSET,
							KPROC_CLK_MGR_REG_POLICY_CTL_GO_ATL_MASK | KPROC_CLK_MGR_REG_POLICY_CTL_GO_MASK,
							0,
							KPROC_CLK_MGR_REG_POLICY_CTL_GO_ATL_MASK | KPROC_CLK_MGR_REG_POLICY_CTL_GO_MASK );

	/* wait until complete */
	while( ccu_get_bit( MM_IO_BASE_PROC_CLK + KPROC_CLK_MGR_REG_POLICY_CTL_OFFSET, KPROC_CLK_MGR_REG_POLICY_CTL_GO_SHIFT ) != 0 );                           

	/* restore previous access */
	ccu_restore_kproc_clk_mgr(access);

	/* set axi div, if it changes */
	if ( new_axi_div != cur_axi_div )
	{
		ccu_set_kproc_axi_div( new_axi_div );
	}

	/* if we scaled down the post divider previously, restore it */
	if ( (new_axi_div != 0) && (new_axi_div != cur_axi_div) )
	{
		ccu_set_kproc_mdiv( freq, mdiv_save );
	}
}

static inline ccu_kproc_policy_freq_e ccu_get_kproc_policy_freq( ccu_policy_num_e policy_num )
{
   ccu_kproc_policy_freq_e  freq_id;
   
   freq_id = (ccu_kproc_policy_freq_e)ccu_get_reg_field( MM_IO_BASE_PROC_CLK + KPROC_CLK_MGR_REG_POLICY_FREQ_OFFSET,          /* addr  */
                                                                   KPROC_CLK_MGR_REG_POLICY_FREQ_POLICY0_FREQ_MASK << (8 * policy_num), /* mask  */
                                                                   8 * policy_num );                                                    /* shift */

   return freq_id;
}

static inline void ccu_set_kproc_pll(uint32_t ndiv_int, uint32_t ndiv_frac, uint8_t pdiv, uint8_t mdiv)
{
   uint32_t access;
   uint32_t regval;
   ccu_policy_num_e pid;
   ccu_freq_id_e fid;
   ccu_kproc_policy_freq_e freq_sv = 0;
   
   access = ccu_unlock_kproc_clk_mgr();

   /* get current active policy & freq id */
   regval = CHAL_REG_READ32( MM_IO_BASE_PROC_CLK + KPROC_CLK_MGR_REG_POLICY_DBG_OFFSET );
   pid = (regval & KPROC_CLK_MGR_REG_POLICY_DBG_ACT_POLICY_MASK) >> KPROC_CLK_MGR_REG_POLICY_DBG_ACT_POLICY_SHIFT;
   fid = (regval & KPROC_CLK_MGR_REG_POLICY_DBG_ACT_FREQ_MASK) >> KPROC_CLK_MGR_REG_POLICY_DBG_ACT_FREQ_SHIFT;
   
   /* need to temporary bump frequency down to root PLL frequencies if we're currently running the local PLL */
   if ((fid >= ccu_freq_id_6))
   {
      freq_sv = ccu_get_kproc_policy_freq( pid );
      ccu_set_kproc_policy_freq( pid, ccu_kproc_policy_freq_312_156_104_1V0 );
   }
   
   /* enable sw update */
   ccu_set_kproc_lvm_enable();
   
   /* wait until all existing policy configurations are complete */
   ccu_wait_kproc_lvm_ready();
   
   /* configure PLL */
   regval  = CHAL_REG_READ32(MM_IO_BASE_PROC_CLK + KPROC_CLK_MGR_REG_PLLARMA_OFFSET);
   regval &= ~(KPROC_CLK_MGR_REG_PLLARMA_PLLARM_NDIV_INT_MASK           |
               KPROC_CLK_MGR_REG_PLLARMA_PLLARM_PDIV_MASK               |
               KPROC_CLK_MGR_REG_PLLARMA_PLLARM_SOFT_RESETB_MASK        |
               KPROC_CLK_MGR_REG_PLLARMA_PLLARM_SOFT_POST_RESETB_MASK);
   regval |= ((ndiv_int << KPROC_CLK_MGR_REG_PLLARMA_PLLARM_NDIV_INT_SHIFT) & KPROC_CLK_MGR_REG_PLLARMA_PLLARM_NDIV_INT_MASK);
   regval |= ((pdiv     << KPROC_CLK_MGR_REG_PLLARMA_PLLARM_PDIV_SHIFT)     & KPROC_CLK_MGR_REG_PLLARMA_PLLARM_PDIV_MASK);
   regval &= ~KPROC_CLK_MGR_REG_PLLARMA_PLLARM_IDLE_PWRDWN_SW_OVRRIDE_MASK;

   CHAL_REG_WRITE32(MM_IO_BASE_PROC_CLK + KPROC_CLK_MGR_REG_PLLARMA_OFFSET, regval);
   
   CHAL_REG_WRITE32(MM_IO_BASE_PROC_CLK + KPROC_CLK_MGR_REG_PLLARMB_OFFSET, (ndiv_frac & KPROC_CLK_MGR_REG_PLLARMB_PLLARM_NDIV_FRAC_MASK));
   
   CHAL_REG_WRITE32(MM_IO_BASE_PROC_CLK + KPROC_CLK_MGR_REG_PLLARMCTRL0_OFFSET, 0x00424000);
   
   ccu_set_reg_field(MM_IO_BASE_PROC_CLK + KPROC_CLK_MGR_REG_PLLARMCTRL5_OFFSET,   /* addr  */
                          KPROC_CLK_MGR_REG_PLLARMCTRL5_PLLARM_H_MDIV_MASK,             /* mask  */
                          KPROC_CLK_MGR_REG_PLLARMCTRL5_PLLARM_H_MDIV_SHIFT,            /* shift */
                          mdiv);
   
   ccu_set_reg_field(MM_IO_BASE_PROC_CLK + KPROC_CLK_MGR_REG_PLLARMC_OFFSET,       /* addr  */
                          KPROC_CLK_MGR_REG_PLLARMC_PLLARM_ENB_CLKOUT_MASK,             /* mask  */
                          KPROC_CLK_MGR_REG_PLLARMC_PLLARM_ENB_CLKOUT_SHIFT,            /* shift */
                          1);
   
   /* release reset */
   regval  = CHAL_REG_READ32(MM_IO_BASE_PROC_CLK + KPROC_CLK_MGR_REG_PLLARMA_OFFSET);   
   regval |= KPROC_CLK_MGR_REG_PLLARMA_PLLARM_SOFT_RESETB_MASK;
   regval |= KPROC_CLK_MGR_REG_PLLARMA_PLLARM_SOFT_POST_RESETB_MASK;
   CHAL_REG_WRITE32(MM_IO_BASE_PROC_CLK + KPROC_CLK_MGR_REG_PLLARMA_OFFSET, regval);
   
   while( ccu_get_bit( MM_IO_BASE_PROC_CLK + KPROC_CLK_MGR_REG_PLLARMA_OFFSET, KPROC_CLK_MGR_REG_PLLARMA_PLLARM_FLOCK_SHIFT ) == 0 );
   
   ccu_set_reg_field(MM_IO_BASE_PROC_CLK + KPROC_CLK_MGR_REG_PLLARMC_OFFSET,       /* addr  */
                          KPROC_CLK_MGR_REG_PLLARMC_PLLARM_ENB_CLKOUT_MASK,             /* mask  */
                          KPROC_CLK_MGR_REG_PLLARMC_PLLARM_ENB_CLKOUT_SHIFT,            /* shift */
                          0);

   {
      uint32_t hz, div;

      /* setup AXI switch div -- for frequency ID 7 */
      hz = ccu_get_kproc_policy_freq_a9_hz( ccu_kproc_policy_freq_aclkh_aclkhdiv2_aclkhdiv3 );
      if ( hz > 3*CCU_MAX_A9_AXI_FREQ )
      {
         div = 3;	/* divide by 4 */
      }
      else if ( hz > 2*CCU_MAX_A9_AXI_FREQ )
      {
         div = 2;	/* divide by 3 */
      }
      else if ( hz > 1*CCU_MAX_A9_AXI_FREQ )
      {
         div = 1;	/* divide by 2 */
      }
      else
      {
         div = 0;	/* divide by 1 */
      }
      ccu_set_kproc_fid7_arm_swt_div( div );
   }

   /* restore original frequency ID, if necessary */
   if ((fid >= ccu_freq_id_6))
   {
      ccu_set_kproc_policy_freq( pid, freq_sv );
   }

   /* restore previous access */
   ccu_restore_kproc_clk_mgr(access);

}

static inline void ccu_get_kproc_pll(uint32_t *ndiv_int, uint32_t *ndiv_frac, uint32_t *pdiv, uint32_t *mdiv)
{
   *ndiv_int = ccu_get_reg_field(MM_IO_BASE_PROC_CLK + KPROC_CLK_MGR_REG_PLLARMA_OFFSET,   /* addr  */
                                      KPROC_CLK_MGR_REG_PLLARMA_PLLARM_NDIV_INT_MASK,           /* mask  */
                                      KPROC_CLK_MGR_REG_PLLARMA_PLLARM_NDIV_INT_SHIFT);         /* shift */
   
   *ndiv_frac = ccu_get_reg_field(MM_IO_BASE_PROC_CLK + KPROC_CLK_MGR_REG_PLLARMB_OFFSET,  /* addr  */
                                       KPROC_CLK_MGR_REG_PLLARMB_PLLARM_NDIV_FRAC_MASK,         /* mask  */
                                       KPROC_CLK_MGR_REG_PLLARMB_PLLARM_NDIV_FRAC_SHIFT);       /* shift */
                                     
   *pdiv = ccu_get_reg_field(MM_IO_BASE_PROC_CLK + KPROC_CLK_MGR_REG_PLLARMA_OFFSET,       /* addr  */
                                  KPROC_CLK_MGR_REG_PLLARMA_PLLARM_PDIV_MASK,                   /* mask  */
                                  KPROC_CLK_MGR_REG_PLLARMA_PLLARM_PDIV_SHIFT);                 /* shift */

   *mdiv = ccu_get_reg_field(MM_IO_BASE_PROC_CLK + KPROC_CLK_MGR_REG_PLLARMCTRL5_OFFSET,   /* addr  */
                                  KPROC_CLK_MGR_REG_PLLARMCTRL5_PLLARM_H_MDIV_MASK,             /* mask  */
                                  KPROC_CLK_MGR_REG_PLLARMCTRL5_PLLARM_H_MDIV_SHIFT);           /* shift */
}

static inline uint32_t ccu_get_kproc_pll_freq( void )
{
   uint32_t freq_int;
   uint32_t freq_frac;
   uint32_t ndiv_int;
   uint32_t ndiv_frac;
   uint32_t pdiv;
   uint32_t mdiv;
   uint32_t osc_in_10khz;

   ccu_get_kproc_pll(&ndiv_int, &ndiv_frac, &pdiv, &mdiv);

   /* re-adjust divisors if 0 */
   if (ndiv_int == 0)
      ndiv_int = 256;
   if (pdiv == 0)
      pdiv = 16;
   if (mdiv == 0)
      mdiv = 256;


   /* determine oscillator frequency */
   /* assume 26MHz crystal clock */
   osc_in_10khz = 2600;
      
   /* calculate CPU frequency: F = OSC x [ndiv_int + ndiv_frac/2^20]/[pdiv x mdiv] */
   freq_int = (osc_in_10khz * ndiv_int) / (pdiv * mdiv);
   freq_int = freq_int * 10000;
   
   freq_frac = (osc_in_10khz * ndiv_frac) / (1 << 20);
   freq_frac = freq_frac * 10000;
   freq_frac = freq_frac / (pdiv * mdiv);
   
   return freq_int + freq_frac;

}

static inline uint32_t ccu_get_kproc_policy_freq_a9_hz( ccu_kproc_policy_freq_e freq )
{

   uint32_t freq_a9_hz;
   uint32_t mdiv;

   switch( freq )
   {
      case ccu_kproc_policy_freq_xtal:
         freq_a9_hz = 26000000;  /* 26Mhz */
         break;
      case ccu_kproc_policy_freq_52_52_52:
         freq_a9_hz = 52000000;  /* 52Mhz */
         break;
      case ccu_kproc_policy_freq_156_156_78:
         freq_a9_hz = 156000000; /* 156Mhz */
         break;
      case ccu_kproc_policy_freq_156_156_78_0V9:
         freq_a9_hz = 156000000; /* 156Mhz */
         break;
      case ccu_kproc_policy_freq_312_156_104:
         freq_a9_hz = 312000000; /* 312Mhz */
         break;
      case ccu_kproc_policy_freq_312_156_104_1V0:
         freq_a9_hz = 312000000; /* 312Mhz */
         break;
      case ccu_kproc_policy_freq_aclk_aclkdiv2_aclkdiv3:
         mdiv = ccu_get_kproc_mdiv( ccu_kproc_policy_freq_aclk_aclkdiv2_aclkdiv3 );
         freq_a9_hz = (ccu_get_kproc_pll_freq()/mdiv) * 2;
         break;
      case ccu_kproc_policy_freq_aclkh_aclkhdiv2_aclkhdiv3:
         mdiv = ccu_get_kproc_mdiv( ccu_kproc_policy_freq_aclkh_aclkhdiv2_aclkhdiv3 );
         freq_a9_hz = (ccu_get_kproc_pll_freq()/mdiv) * 2;
         break;

      default:
         freq_a9_hz = 0; /* not supported */
         break;
   }

   return freq_a9_hz;
   
}

static inline void ccu_cfg_kproc_policy_mask( ccu_policy_num_e policy_num, ccu_kproc_policy_mask_e policy_type, uint32_t val )
{
   uint32_t access;
   uint32_t addr;
   uint32_t bit;

   /* enable access */
   access = ccu_unlock_kproc_clk_mgr();

   /* enable sw update */
   ccu_set_kproc_lvm_enable();
   
   /* wait until all existing policy configurations are complete */
   ccu_wait_kproc_lvm_ready();

   addr  = MM_IO_BASE_PROC_CLK + KPROC_CLK_MGR_REG_POLICY0_MASK_OFFSET;
   addr += sizeof(uint32_t) * policy_num;
   bit   = (policy_type == ccu_kproc_arm_policy_mask) ? KPROC_CLK_MGR_REG_POLICY0_MASK_ARM_POLICY0_MASK_SHIFT : KPROC_CLK_MGR_REG_POLICY0_MASK_APB0_POLICY0_MASK_SHIFT;

   ccu_set_bit( addr, bit, val );
   
   /* trigger go */
   ccu_set_reg_field( MM_IO_BASE_PROC_CLK + KPROC_CLK_MGR_REG_POLICY_CTL_OFFSET,
                           KPROC_CLK_MGR_REG_POLICY_CTL_GO_ATL_MASK | KPROC_CLK_MGR_REG_POLICY_CTL_GO_MASK,
                           0,
                           KPROC_CLK_MGR_REG_POLICY_CTL_GO_ATL_MASK | KPROC_CLK_MGR_REG_POLICY_CTL_GO_MASK );
   
   /* wait until complete */
   while( ccu_get_bit( MM_IO_BASE_PROC_CLK + KPROC_CLK_MGR_REG_POLICY_CTL_OFFSET, KPROC_CLK_MGR_REG_POLICY_CTL_GO_SHIFT ) != 0 );
   
   /* restore previous access */
   ccu_restore_kproc_clk_mgr(access);
}

static inline void ccu_set_kproc_policy_mask( ccu_policy_num_e policy_num, ccu_kproc_policy_mask_e policy_type )
{
   ccu_cfg_kproc_policy_mask( policy_num, policy_type, 1 );
}

static inline void ccu_clr_kproc_policy_mask( ccu_policy_num_e policy_num, ccu_kproc_policy_mask_e policy_type )
{
   ccu_cfg_kproc_policy_mask( policy_num, policy_type, 0 );
}

static inline uint32_t ccu_get_kproc_policy_mask( ccu_policy_num_e policy_num, ccu_kproc_policy_mask_e policy_type )
{
   uint32_t addr;
   uint32_t bit;
   uint32_t val;
   
   addr  = MM_IO_BASE_PROC_CLK + KPROC_CLK_MGR_REG_POLICY0_MASK_OFFSET;
   addr += sizeof(uint32_t) * policy_num;
   bit   = (policy_type == ccu_kproc_arm_policy_mask) ? KPROC_CLK_MGR_REG_POLICY0_MASK_ARM_POLICY0_MASK_SHIFT : KPROC_CLK_MGR_REG_POLICY0_MASK_APB0_POLICY0_MASK_SHIFT;
   val   = ccu_get_bit( addr, bit );
   
   return val;
}

static inline ccu_policy_num_e ccu_get_kproc_active_policy( void )
{
   return (ccu_policy_num_e)ccu_get_reg_field( MM_IO_BASE_PROC_CLK + KPROC_CLK_MGR_REG_POLICY_DBG_OFFSET,   /* addr */
                                                         KPROC_CLK_MGR_REG_POLICY_DBG_ACT_POLICY_MASK,                /* mask */
                                                         KPROC_CLK_MGR_REG_POLICY_DBG_ACT_POLICY_SHIFT );             /* shift */
}

static inline void ccu_set_esub_policy_freq( ccu_policy_num_e policy_num, ccu_esub_policy_freq_e freq )
{
   uint32_t access;
   
   /* enable access */
   access = ccu_unlock_esub_clk_mgr();
   
   /* enable sw update */
   ccu_set_esub_lvm_enable();
   
   /* wait until all existing policy configurations are complete */
   ccu_wait_esub_lvm_ready();
   

   /* set frequency policy */
   ccu_set_reg_field( MM_IO_BASE_ESUB_CLK + ESUB_CLK_MGR_REG_POLICY_FREQ_OFFSET,             /* addr  */
                           ESUB_CLK_MGR_REG_POLICY_FREQ_POLICY0_FREQ_MASK << (8 * policy_num),    /* mask  */
                           8 * policy_num,                                                        /* shift */
                           (uint32_t)freq ) ;                                                     /* value */

   /* trigger go */
   ccu_set_reg_field( MM_IO_BASE_ESUB_CLK + ESUB_CLK_MGR_REG_POLICY_CTL_OFFSET,
                           ESUB_CLK_MGR_REG_POLICY_CTL_GO_ATL_MASK | ESUB_CLK_MGR_REG_POLICY_CTL_GO_MASK,
                           0,
                           ESUB_CLK_MGR_REG_POLICY_CTL_GO_ATL_MASK | ESUB_CLK_MGR_REG_POLICY_CTL_GO_MASK );
   
   /* wait until complete */
   while( ccu_get_bit( MM_IO_BASE_ESUB_CLK + ESUB_CLK_MGR_REG_POLICY_CTL_OFFSET, ESUB_CLK_MGR_REG_POLICY_CTL_GO_SHIFT ) != 0 );
   
   /* restore access */
   ccu_restore_esub_clk_mgr(access);
}

static inline ccu_esub_policy_freq_e ccu_get_esub_policy_freq( uint32_t policy_num )
{
   uint32_t access;
   uint32_t freq_id;
   
   /* enable access */
   access = ccu_unlock_esub_clk_mgr();
   
   /* set frequency policy */
   freq_id = ccu_get_reg_field( MM_IO_BASE_ESUB_CLK + ESUB_CLK_MGR_REG_POLICY_FREQ_OFFSET,            /* addr  */
                                     ESUB_CLK_MGR_REG_POLICY_FREQ_POLICY0_FREQ_MASK << (8 * policy_num),   /* mask  */
                                     8 * policy_num ) ;                                                    /* shift */
   
   /* restore access */
   ccu_restore_esub_clk_mgr(access);
   
   return (ccu_esub_policy_freq_e)freq_id;
}

static inline void ccu_cfg_esub_policy_mask( ccu_policy_num_e policy_num, ccu_esub_policy_mask_e policy_type, uint32_t val )
{
   uint32_t addr;
   uint32_t access;
   
   /* enable access */
   access = ccu_unlock_esub_clk_mgr();
   
   /* enable sw update */
   ccu_set_esub_lvm_enable();
   
   /* wait until all existing policy configurations are complete */
   ccu_wait_esub_lvm_ready();
   
   addr  = MM_IO_BASE_ESUB_CLK + ESUB_CLK_MGR_REG_POLICY0_MASK_OFFSET;
   addr += (sizeof(uint32_t) * policy_num);

   ccu_set_bit( addr, policy_type, val );
   
   /* trigger go */
   ccu_set_reg_field( MM_IO_BASE_ESUB_CLK + ESUB_CLK_MGR_REG_POLICY_CTL_OFFSET,
                           ESUB_CLK_MGR_REG_POLICY_CTL_GO_ATL_MASK | ESUB_CLK_MGR_REG_POLICY_CTL_GO_MASK,
                           0,
                           ESUB_CLK_MGR_REG_POLICY_CTL_GO_ATL_MASK | ESUB_CLK_MGR_REG_POLICY_CTL_GO_MASK );
   
   /* wait until complete */
   while( ccu_get_bit( MM_IO_BASE_ESUB_CLK + ESUB_CLK_MGR_REG_POLICY_CTL_OFFSET, ESUB_CLK_MGR_REG_POLICY_CTL_GO_SHIFT ) != 0 );
   
   /* restore access */
   ccu_restore_esub_clk_mgr(access);
}

static inline void ccu_set_esub_policy_mask( ccu_policy_num_e policy_num, ccu_esub_policy_mask_e policy_type )
{
   ccu_cfg_esub_policy_mask( policy_num, policy_type, 1 );
}

static inline void ccu_clr_esub_policy_mask( ccu_policy_num_e policy_num, ccu_esub_policy_mask_e policy_type )
{
   ccu_cfg_esub_policy_mask( policy_num, policy_type, 0 );
}

static inline uint32_t ccu_get_esub_policy_mask( ccu_policy_num_e policy_num, ccu_esub_policy_mask_e policy_type )
{
   uint32_t addr;
   uint32_t bit;
   
   addr  = MM_IO_BASE_ESUB_CLK + ESUB_CLK_MGR_REG_POLICY0_MASK_OFFSET;
   addr += (sizeof(uint32_t) * policy_num);
   bit   = ccu_get_bit( addr, policy_type );
   
   return bit;
}

static inline ccu_policy_num_e ccu_get_esub_active_policy( void )
{
   return (ccu_policy_num_e)ccu_get_reg_field( MM_IO_BASE_ESUB_CLK + ESUB_CLK_MGR_REG_POLICY_DBG_OFFSET,  /* addr */
                                                         ESUB_CLK_MGR_REG_POLICY_DBG_ACT_POLICY_MASK,               /* mask */
                                                         ESUB_CLK_MGR_REG_POLICY_DBG_ACT_POLICY_SHIFT );            /* shift */
}



#ifdef __cplusplus
}
#endif

#endif /* _CCU_INLINE_H_*/

