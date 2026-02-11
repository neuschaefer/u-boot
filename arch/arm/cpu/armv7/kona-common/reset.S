/*
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
.globl reset_cpu
reset_cpu:

#include <config.h>

#ifdef CONFIG_USE_SECWD2_FOR_RESET
#include <asm/arch/brcm_rdb_sysmap.h>

	ldr	r1, =SECWD2_BASE_ADDR
	ldr	r2, [r1]

        ldr     r3, =0xF2000000
        and     r2, r2, r3

        ldr     r3, =0x0C800008
        orr     r2, r2, r3

        str     r2, [r1]

#else
	ldr	r1, =0x35001f00
	ldr	r2, [r1]
	ldr	r4, =0x80000000
	and	r4, r2, r4
	ldr     r3, =0xA5A500
	orr     r4, r4, r3
        orr     r4, r4, #0x1
        
        str     r4, [r1]

	ldr	r1, =0x35001f04
	ldr	r2, [r1]
	ldr	r4, =0x80000000
	and	r4, r2, r4
	str	r4, [r1]
#endif

_loop_forever:
	b	_loop_forever
