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


#ifndef __ASM_ARCH_SYSTEM_H
#define __ASM_ARCH_SYSTEM_H __FILE__

#include <mach/regs-sdio.h>

/* FIXME: Actually wait in sleep mode */
static inline void arch_idle (void)
{
	/* Do nothing here for now */
}

/* FIXME: Finish correct arch_reset() */
static inline void arch_reset (char mode)
{
	/* Do nothing */
}

#endif /*__ASM_ARCH_SYSTEM_H */
