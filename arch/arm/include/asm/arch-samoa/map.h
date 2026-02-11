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


#ifndef __ASM_ARCH_MAP_H
#define __ASM_ARCH_MAP_H __FILE__

#define KONA_RAM_START	(0x80000000)

#define KONA_PA_SYSTMR	(0x3E00D000)

#define KONA_PA_SDIO1	(0x3F180000) 
#define KONA_PA_SDIO2	(0x3F190000)

#define KONA_PA_UART0	(0x3E000000)

#define KONA_PA_UART1	(0x3E001000)

#define KONA_PA_GPIO	(0x35003000) 

#define KONA_SECWD_BASE_ADDR	(0x3500C000)

#define KONA_PAD_CTRL_BASE_ADDR (0x35004800)

#define HSOTG_BASE_ADDR           0x3F120000 /* brcm_rdb_hsotg.h */
#define HSOTG_CTRL_BASE_ADDR      0x3F130000 /* brcm_rdb_hsotg_ctrl.h */


/* From brcm_rdb_kpm_clk_mgr_reg.h */
#define KPM_CLK_MGR_REG_SDIO1_DIV_OFFSET                                  0x00000A28
#define KPM_CLK_MGR_REG_SDIO2_DIV_OFFSET                                  0x00000A2C

#endif /* __ASM_ARCH_MAP_H */
