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
* @file  ccu_util_inline.c
*
* @brief Clock Control Unit inline functions
*
* @note
*
*******************************************************************************/
#ifndef _CCU_UTIL_INLINE_H_
#define _CCU_UTIL_INLINE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <asm/arch/reg.h>
#define CCU_REG_READ32(addr) reg32_read((volatile uint32_t *)(addr))
#define CCU_REG_WRITE32(addr, val) reg32_write((volatile uint32_t *)(addr), val)
#define CCU_REG_CLRBIT32(addr, bits)  reg32_clear_bits((volatile uint32_t *)(addr), bits)
#define CCU_REG_SETBIT32(addr, bits)  reg32_set_bits((volatile uint32_t *)(addr), bits)

#define CCU_INLINE_RC_OK      0
#define CCU_INLINE_RC_FAIL    1


/* Macros to set/get multiple register bits at one time */
#define ccu_set_reg_field(addr, mask, shift, val)      \
            do                                              \
            {                                               \
               uint32_t tmp;                                \
               tmp  = CCU_REG_READ32(addr);                \
               tmp &= ~(mask);                              \
               tmp |= (((val) << (shift)) & (mask));        \
               CCU_REG_WRITE32(addr, tmp);                 \
                                                            \
            } while(0)

#define ccu_get_reg_field(addr, mask, shift)           \
            ((CCU_REG_READ32(addr) & (mask)) >> (shift))


/* Function Prototypes */
static inline void      ccu_set_bit( uint32_t addr, uint32_t bit_num, uint32_t val );
static inline uint32_t  ccu_get_bit( uint32_t addr, uint32_t bit_num );
static inline uint32_t  ccu_wait_for_mask_clear(uint32_t reg, uint32_t mask, uint32_t msec);
static inline uint32_t  ccu_inline_wait_for_mask_set(uint32_t reg, uint32_t mask, uint32_t msec);

/* Functions */
static inline void ccu_set_bit( uint32_t addr, uint32_t bit_num, uint32_t val )
{
   if( val )
   {
      CCU_REG_SETBIT32( addr, 1 << bit_num );
   }
   else
   {
      CCU_REG_CLRBIT32( addr, 1 << bit_num );
   }
}

static inline uint32_t ccu_get_bit( uint32_t addr, uint32_t bit_num )
{
   return (CCU_REG_READ32( addr ) >> bit_num) & 0x1;
}

static inline uint32_t ccu_wait_for_mask_clear(uint32_t reg, uint32_t mask, uint32_t msec)
{
    while (msec)
    {
       udelay(1000);
       msec--;
       if ((CCU_REG_READ32(reg) & mask) == 0)
       {
          return CCU_INLINE_RC_OK;
       }
    }

    if ((CCU_REG_READ32(reg) & mask))
    {
        return CCU_INLINE_RC_FAIL;
    }

    return CCU_INLINE_RC_OK;
}

static inline uint32_t ccu_inline_wait_for_mask_set(uint32_t reg, uint32_t mask, uint32_t msec)
{
    while (msec)
    {
       udelay(1000);
       msec--;
       if ((CCU_REG_READ32(reg) & mask) == mask)
       {
          return CCU_INLINE_RC_OK;
       }
    }

    if ((CCU_REG_READ32(reg) & mask) != mask)
    {
        return CCU_INLINE_RC_FAIL;
    }

    return CCU_INLINE_RC_OK;
}

#ifdef __cplusplus
}
#endif

#endif /* _CCU_UTIL_INLINE_H_*/

