#include <common.h>
#include <asm/io.h>
#include <linux/err.h>
#include <linux/stddef.h>
#include <asm/arch/ipc.h>



/* Struct to store the user info in */
typedef struct IPC_BLOCK_USER_INFO
{
   uint32_t four_cc;
   uint32_t block_base_address;
   uint32_t interrupt_number_in_ipc;
   const void *funcs;

} IPC_BLOCK_USER_INFO_T;

typedef enum {
	IPC_SEMAPHORE_ID_MIN    = 0,
	IPC_SEMAPHORE_ID_0  = IPC_SEMAPHORE_ID_MIN,
	IPC_SEMAPHORE_ID_1,
	IPC_SEMAPHORE_ID_2,
	IPC_SEMAPHORE_ID_3,
	IPC_SEMAPHORE_ID_4,
	IPC_SEMAPHORE_ID_5,
	IPC_SEMAPHORE_ID_6,
	IPC_SEMAPHORE_ID_7,
	IPC_SEMAPHORE_ID_MAX    = IPC_SEMAPHORE_ID_7,
} IPC_SEMAPHORE_ID_T;

#define __ipc_bus_to_phys(x) (x - 0xC0000000)
#define __ipc_phys_to_bus(x) (x + 0xC0000000)

static uint32_t g_ipc_base = 0;

/*
 * Init just makes sure that VC FW has been loaded and we have a base address.
 * Each individual service checks if their block is active on first use.
 */

int ipc_init(void){
    uint32_t ipc_base;
    if (g_ipc_base) {
        printf("Error: Already got IPC base.\n");
        return -1;
    }

    //printf("Polling for IPC Base addr..\n");
    while ((readl(ARM_0_MAIL0_STA)) & ARM_MS_EMPTY) {
    }

    ipc_base = readl(ARM_0_MAIL0_RD);
    printf("Got IPC Base addr: 0x%08lx\n", ipc_base);

    if (ipc_base == 0) {
        printf("Error: Invalid IPC address\n");
        return -1;
    }

    g_ipc_base = __ipc_bus_to_phys(ipc_base);
    return 0;
}

void ipc_shutdown(void) {
    if (g_ipc_base != 0) {
        // Send message to vc saying we disconnected?
        g_ipc_base = 0;
    } else {
        printf("Error: IPC not active.\n");
    }
}


uint32_t ipc_get_service_addr(const uint32_t fourcc, uint32_t *ipc_num)
{
    uint32_t fcc_offset = (uint32_t)((char*)g_ipc_base + IPC_BLOCK_INFO_OFFSET);
    uint32_t service_addr = 0;
    int blk_num = 0;
    uint32_t start, current;
    char tmp[5];
    tmp[4] = '\0';
    *(uint32_t *)tmp = fourcc;

    //printf("get serv addr %s %x\n", tmp, fourcc);

    if (!g_ipc_base) {
        printf("Error: VC not running.\n");
        return 0;
    }

    if (IPC_BLOCK_MAGIC != readl(g_ipc_base + IPC_BLOCK_MAGIC_OFFSET)) {
        printk("Error: VC Has not initialized the IPC block yet!\n");
        return 0;
    }

    for (blk_num = 0; blk_num < IPC_BLOCK_MAX_NUM_USERS; blk_num++) {
        start = 0;
        current  = readl(fcc_offset + offsetof(IPC_BLOCK_USER_INFO_T, four_cc));

        if (current == fourcc) {
            //printf("Found fourcc!\n");
            service_addr = __ipc_bus_to_phys(readl(fcc_offset + offsetof(IPC_BLOCK_USER_INFO_T, block_base_address)));
            if (ipc_num != NULL) {
                *ipc_num = readl(fcc_offset + offsetof(IPC_BLOCK_USER_INFO_T, interrupt_number_in_ipc));
            }
            break;
        } else if (current == 0) {
            break;
        }
        *(uint32_t *)tmp = current;
        //printf("skipping fourcc %s %x\n",tmp, current);
        fcc_offset += sizeof(IPC_BLOCK_USER_INFO_T);
    }
    if (current == 0) {
        printf("Error: fourcc not found\n");
    }

    return service_addr;
}

static void ipc_spin_lock(IPC_SEMAPHORE_ID_T lock_id)
{
    while (0x0 != readl(ARM_0_SEM0 + (lock_id << 2)));
}

static void ipc_spin_unlock(IPC_SEMAPHORE_ID_T lock_id)
{
    BUG_ON(0x0 == readl(ARM_0_SEM0 + (lock_id << 2)));
    writel(0x1, ARM_0_SEM0 + (lock_id << 2));
}


int ipc_notify_vc_event(uint32_t ipc_id){

    u32 vc_irq_status;

    //printf("ipc notify vc event %d\n", ipc_id);
    ipc_spin_lock(IPC_SEMAPHORE_ID_0);
    vc_irq_status = readl(g_ipc_base + IPC_ARM_VC_INTERRUPT_OFFSET);
    vc_irq_status |= (0x1 << ipc_id);
    writel(vc_irq_status, g_ipc_base + IPC_ARM_VC_INTERRUPT_OFFSET);
    ipc_spin_unlock(IPC_SEMAPHORE_ID_0);

    writel(0x1, ARM_0_BELL2);

    return 0;

}


uint32_t ipc_wait_event(uint32_t req_ipc)
{
	u32 vc_irq_status, ipc_id;

    printf("ipc wait event %d\n", req_ipc);

	if (!req_ipc) {
		req_ipc = -1;
	}

	do {
		ipc_spin_lock(IPC_SEMAPHORE_ID_1);
		vc_irq_status = readl(g_ipc_base + IPC_VC_ARM_INTERRUPT_OFFSET);

		if (vc_irq_status & req_ipc) {
			writel(vc_irq_status & ~req_ipc, g_ipc_base + IPC_VC_ARM_INTERRUPT_OFFSET);
			ipc_spin_unlock(IPC_SEMAPHORE_ID_1);
			break;
		}
		ipc_spin_unlock(IPC_SEMAPHORE_ID_1);
        udelay(9);
	} while (1);
}
