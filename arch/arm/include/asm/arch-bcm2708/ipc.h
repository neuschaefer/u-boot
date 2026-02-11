/*
 * arch/arm/mach-versatile/include/mach/ipc.h
 *
 * Copyright (c) ARM Limited 2003.  All rights reserved.
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef _BCM2708_IPC_H_
#define _BCM2708_IPC_H_

#include <asm/io.h>

#define IPC_BLOCK_MAGIC			0xCAFEBABE

#define IPC_BLOCK_MAGIC_OFFSET		0x000

#define IPC_BLOCK_INFO_OFFSET		0x200

#define IPC_VC_ARM_INTERRUPT_OFFSET	0x800  /* VC  -> ARM */
#define IPC_ARM_VC_INTERRUPT_OFFSET	0x804  /* ARM -> VC */

//currently we tie the interrupt number to the user slot
#define IPC_BLOCK_MAX_NUM_USERS		32

#define IPC_BLOCK_SIZE			SZ_2M

#define IPC_IRQNUM_NONE		-1

#define __ipc_bus_to_phys(x) (x - 0xC0000000)
#define __ipc_phys_to_bus(x) (x + 0xC0000000)

extern int ipc_init(void);
extern int ipc_notify_vc_event(uint32_t ipc_id);
extern int ipc_lookup_irqnum(u32 four_cc);
extern void *ipc_bus_to_virt(uint32_t bus_addr);

extern uint32_t ipc_get_service_addr(const uint32_t fourcc, uint32_t *ipc_num);

#if 0
static inline int ipc_regs_wait_for_register( uint32_t reg, uint32_t bitmask,
        uint32_t required, uint32_t timeout, uint32_t cond)
{
    uint32_t val;
    uint32_t cnt = timeout;

    printf("wait for reg %x %x\n", reg, bitmask);

    while (1) {
        val = readl(reg);
        printf("got val %x\n", val);
        if ((bitmask & val)) {
            if (required && (bitmask & val) != required) {
                continue;
            }

            if (val & ~bitmask) {
                /* Default vc implementation has very strict rules about conditions
                 * which don't seem to be needed. If you get this error and ipc isn't working,
                 * this is a possibile error source. */
                printf("Warning: possible error, using out of spec ipc reg wait.\n");
            }
            break;
        }
        if (timeout != -1) {
            if (!cnt)
                return -1;
            cnt--;
            udelay(1000);
        }
    }

    return 0;
}
#endif

#endif  /* _BCM2708_IPC_H_ */
