/*
 *  armboot - Startup Code for ARM926EJS CPU-core
 *
 *  Copyright (c) 2003  Texas Instruments
 *
 *  ----- Adapted for OMAP1610 OMAP730 from ARM925t code ------
 *
 *  Copyright (c) 2001	Marius Gröger <mag@sysgo.de>
 *  Copyright (c) 2002	Alex Züpke <azu@sysgo.de>
 *  Copyright (c) 2002	Gary Jennejohn <gj@denx.de>
 *  Copyright (c) 2003	Richard Woodruff <r-woodruff2@ti.com>
 *  Copyright (c) 2003	Kshitij <kshitij@ti.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */


#include "../../../include/configs/wpcm450.h"


.align  5
.globl reset_cpu

reset_cpu:

#if 0

    LDR     R3, CLK_PMCON_REG   /* PMCON, power management control register */
    MOV     R2, #0x8            /* set 1 to soft reset control bit */
    STR     R2, [R3,#0x0]       /* *((unsigned long *) 0xB0000214) = 0x08 */

CLK_PMCON_REG:
    .word   0xB0000214

#else

#ifdef CONFIG_WPCM450_BOOT_STATUS
    LDR     R3, BOOT_STATUS_REG /* boot status memory address */
    LDR     R2, [R3,#0x0]       /* read first byte to R2 */
    ADD     R2, R2, #1          /* increase by 1 */
    STR     R2, [R3,#0x0]       /* write back to corrupt checksum */
#endif

    LDR     R3, WATCHDOG_REG    /* WTCR, watchdog timer control register */
    MOV     R2, #0x83           /* set 0x83 to trigger watchdog in 0.17 s /256 */
    STR     R2, [R3,#0x0]       /* *((unsigned long *) 0xB800101C) = 0x083 */

WATCHDOG_REG:
    .word   0xB800101C


#ifdef CONFIG_WPCM450_BOOT_STATUS
BOOT_STATUS_REG:
    .word   0xC6000000
#endif


#endif


_loop_forever:
    b   _loop_forever
