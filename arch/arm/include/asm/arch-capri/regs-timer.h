/*****************************************************************************
* Copyright 2005 - 2008 Broadcom Corporation.  All rights reserved.
*
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed to you
* under the terms of the GNU General Public License version 2, available at
* http://www.broadcom.com/licenses/GPLv2.php (the "GPL"). 
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a
* license other than the GPL, without Broadcom's express prior written
* consent.
*****************************************************************************/


#ifndef __ASM_ARCH_REGS_TIMER_H
#define __ASM_ARCH_REGS_TIMER_H __FILE__

/****  SYSTEM TIMER  ****/

#define TIMER_STCS                       (TIMER_BASE_ADDR + TIMER_STCS_OFFSET)  /* System Timer Control / Status. */
#define TIMER_STCS_OFFSET                0x0000
  #define TIMER_STCS_NOT_USED                        0xFFFFFE00                 /* RO Bits must be written with 0.  A */
  #define TIMER_STCS_NOT_USED_SHIFT                  9
  #define TIMER_STCS_TIMERCLK_DEBUGCTL               0x0100                     /* RO When = 1: Allow timer 0 to keep */
  #define TIMER_STCS_TIMERCLK_DEBUGCTL_SHIFT         8
  #define TIMER_STCS_COMPARE_ENABLE                  0x00F0                     /* RO Timer Channel Enable,  */
  #define TIMER_STCS_COMPARE_ENABLE_SHIFT            4
  #define TIMER_STCS_TIMER_MATCH                     0x000F                     /* WO Timer Match Occurred on this ch */
  #define TIMER_STCS_TIMER_MATCH_SHIFT               0
#define TIMER_STCLO                      (TIMER_BASE_ADDR + TIMER_STCLO_OFFSET)  /* System Timer Counter Lower bits */
#define TIMER_STCLO_OFFSET               0x0004
  #define TIMER_STCLO_STCLO                          0xFFFFFFFF                  /* RW Timer counter value, write to  */
  #define TIMER_STCLO_STCLO_SHIFT                    0
#define TIMER_STCHI                      (TIMER_BASE_ADDR + TIMER_STCHI_OFFSET)  /* System Timer Counter Upper bits */
#define TIMER_STCHI_OFFSET               0x0008
  #define TIMER_STCHI_STCHI                          0xFFFFFFFF                  /* RW Timer counter value, write to  */
  #define TIMER_STCHI_STCHI_SHIFT                    0
#define TIMER_STCM0                      (TIMER_BASE_ADDR + TIMER_STCM0_OFFSET)  /* System Timer Compare Value */
#define TIMER_STCM0_OFFSET               0x000C
  #define TIMER_STCM0_STCM0                          0xFFFFFFFF                  /* RW When counter channel is enable */
  #define TIMER_STCM0_STCM0_SHIFT                    0
#define TIMER_STCM1                      (TIMER_BASE_ADDR + TIMER_STCM1_OFFSET)  /* System Timer Compare Value */
#define TIMER_STCM1_OFFSET               0x0010
  #define TIMER_STCM1_reserved0                      0xFFFFFFFF                  /* reserved0  */
  #define TIMER_STCM1_reserved0_SHIFT                0
#define TIMER_STCM2                      (TIMER_BASE_ADDR + TIMER_STCM2_OFFSET)  /* System Timer Compare Value */
#define TIMER_STCM2_OFFSET               0x0014
  #define TIMER_STCM2_STCM2                          0xFFFFFFFF                  /* RW When counter channel is enable */
  #define TIMER_STCM2_STCM2_SHIFT                    0
#define TIMER_STCM3                      (TIMER_BASE_ADDR + TIMER_STCM3_OFFSET)  /* System Timer Compare Value */
#define TIMER_STCM3_OFFSET               0x0018
  #define TIMER_STCM3_STCM3                          0xFFFFFFFF                  /* RW When counter channel is enable */
  #define TIMER_STCM3_STCM3_SHIFT                    0

#endif /* __ASM_ARCH_REGS_TIMER_H */
