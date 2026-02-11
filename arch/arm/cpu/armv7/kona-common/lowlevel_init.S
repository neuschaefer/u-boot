/*
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

.globl lowlevel_init
lowlevel_init:
	stmfd   sp!, {r0-r12, lr}
	bl	v7_invalidate_cache_all
	ldmfd   sp!, {r0-r12, lr}
	mov	pc, lr

.global start_vc4
start_vc4:
  push  {r0-r1}      /* Save registers */
  mov   r0, #2       /* Set service request */
  .word 0xE1600070   /* SMC #0 (call monitor) */
  pop   {r0-r1}      /* Restore registers */
  bx    r14
