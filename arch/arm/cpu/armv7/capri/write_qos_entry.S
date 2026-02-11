/************************************************************************************************/
/*                                                                                              */
/*  Copyright 2011  Broadcom Corporation                                                        */
/*                                                                                              */
/*     Unless you and Broadcom execute a separate written software license agreement governing  */
/*     use of this software, this software is licensed to you under the terms of the GNU        */
/*     General Public License version 2 (the GPL), available at                                 */
/*                                                                                              */
/*          http://www.broadcom.com/licenses/GPLv2.php                                          */
/*                                                                                              */
/*     with the following added to such license:                                                */
/*                                                                                              */
/*     As a special exception, the copyright holders of this software give you permission to    */
/*     link this software with independent modules, and to copy and distribute the resulting    */
/*     executable under terms of your choice, provided that you also meet, for each linked      */
/*     independent module, the terms and conditions of the license of that module.              */
/*     An independent module is a module which is not derived from this software.  The special  */
/*     exception does not apply to any modifications of the software.                           */
/*                                                                                              */
/*     Notwithstanding the above, under no circumstances may you combine this software in any   */
/*     way with any other Broadcom software provided under a license other than the GPL,        */
/*     without Broadcom's express prior written consent.                                        */
/*                                                                                              */
/************************************************************************************************/

#include <asm/arch/brcm_rdb_csr.h>

/* void write_qos_entry(uint32_t base, int slot, unsigned long tag, unsigned long val); */

.globl write_qos_entry
write_qos_entry:
	stmfd   sp!, {r4}

	add	r1, r0, r1, LSL #2
	b  update_cam

	/* Enabling CAM updates disables memory accesses, so this must fit in a single cache line */
	.align 5
update_cam:
	mov	r4, #1
	str	r4, [r0, #CSR_CAM_UPDATE_OFFSET]
	str	r3, [r1, #CSR_CAM_ENTRY_0_OFFSET]
	str	r2, [r1, #CSR_CAM_ENABLE_0_OFFSET]
	mov	r4, #0
	str	r4, [r0, #CSR_CAM_UPDATE_OFFSET]

	ldmfd   sp!, {r4}
	mov	pc, lr
