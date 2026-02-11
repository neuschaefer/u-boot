/*****************************************************************************
*  Copyright 2001 - 2011 Broadcom Corporation.  All rights reserved.
*
*  Unless you and Broadcom execute a separate written software license
*  agreement governing use of this software, this software is licensed to you
*  under the terms of the GNU General Public License version 2, available at
*  http://www.gnu.org/licenses/old-license/gpl-2.0.html (the "GPL").
*
*  Notwithstanding the above, under no circumstances may you combine this
*  software in any way with any other Broadcom software provided under a
*  license other than the GPL, without Broadcom's express prior written
*  consent.
*
*****************************************************************************/

/****************************************************************************/
/**
*  @file    boot_core1.S
*
*  @brief   Patch function to boot core 1 using SWDEV mode (secure mode)
*
*  @note
*****************************************************************************/


.globl	boot_core1_stub
.globl	boot_core1_stub_end
.func	boot_core1_stub
boot_core1_stub:
	/*
	** calculate base of register address constants table - need to do this because we're copying
	** code to SRAM w/o the literal table
	*/
	mov		r0, pc
	add		r0, r0, #(boot_core1_stub_const-boot_core1_stub-8)	@ sub 8 to account for pipelining
	ldr		r1, [r0, #0]	@ get boot 2nd address register address
	ldr		r2, [r0, #4]	@ get address mask
	ldr		r1, [r1]	@ get boot 2nd address register (kernel entry point)
	and		r1, r1, r2

	/* disable MMU */
	mrc		p15, 0, r0, c1, c0, 0	@ get SCTLR
	bic		r0, r0, #1
	mcr		p15, 0, r0, c1, c0, 0	@ update SCTLR

	mov		pc, r1
	nop
boot_core1_error:
	b		boot_core1_error
	nop
boot_core1_stub_const:
	.word	0x3500417c
	.word	0xfffffffc
boot_core1_stub_end:
.size boot_core1_stub, . - boot_core1_stub
.endfunc

