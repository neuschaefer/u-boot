
#include <common.h>
#include <asm/io.h>
#include <asm/arch/hardware.h>
#include <asm/kona-common/ipc.h>
#include <asm/arch/brcm_rdb_ipcopen.h>
#include <asm/arch/brcm_rdb_sysmap.h>
#include <asm/arch/brcm_rdb_ipcsec.h>
#include <asm/arch/irqs.h>

#include <common.h>
#include <asm/io.h>
#include <linux/err.h>
#include <linux/stddef.h>

#define VC4_IPC_PHYS_ADDR_BASE	0x60040000UL
#define ARM_IPC_PHYS_ADDR_BASE	0x34040000UL
#define IPC_BASE_ADDR             0x34005000 /* brcm_rdb_ipcopen.h */

/* Struct to store the user info in */
typedef struct IPC_BLOCK_USER_INFO
{
   uint32_t four_cc;
   uint32_t block_base_address;
   uint32_t interrupt_number_in_ipc;
   const void *funcs;

} IPC_BLOCK_USER_INFO_T;


#define IPC_HANDSHAKE_PATTERN	0xdeadcafeUL

#define WAKEUP_MASK	(0x1 << 0)

#define VC4_DIRECT_ACCESS_PHYS_ADDR_BASE	0xC0000000UL
#define ARM_VC4_PHYS_ADDR_BASE			       0x40000000UL
#define ARM_SRAM_MPU_CTRL0	0x34002000UL
#define ARM_SRAM_MPU_CTRL1	0x34002004UL
#define ARM_SRAM_MPU_CTRL2	0x34002008UL


uint32_t ipc_query_wakeup_vc( void )
{
   return readl( IPCSEC_BASE_ADDR + IPCSEC_IPCAWAKE_OFFSET );
}

void ipc_wakeup_vc(unsigned chan, uint32_t run_addr)
{
	(void)chan;

	writel(0xffffffff,  ARM_SRAM_MPU_CTRL0);
	writel(0xffffffff,  ARM_SRAM_MPU_CTRL1);
	writel(0x0000ffff,  ARM_SRAM_MPU_CTRL2);

	/* Clear up the mailbox to fixed pattern and wait for the VC to come up */
	writel(IPC_HANDSHAKE_PATTERN, IPCSEC_BASE_ADDR + IPCSEC_IPCMAIL0_OFFSET);

	writel(run_addr | WAKEUP_MASK, IPCSEC_BASE_ADDR + IPCSEC_IPCAWAKE_OFFSET);
}

uint32_t ipc_read_mailbox( unsigned mailbox_id )
{
    return readl( IPCSEC_BASE_ADDR + IPCSEC_IPCMAIL0_OFFSET + mailbox_id*sizeof(uint32_t) );
}

/*
 * Init just makes sure that VC's IPC block has been loaded and we have a base address.
 * Each individual service checks if their block is active on first use.
 */

int ipc_init(void) {

    //wait for VC to load
    volatile uint32_t magic_bits = 0;
    int timeout = 0xFFFFF0;

    do
    {
       magic_bits = readl(ARM_IPC_PHYS_ADDR_BASE + IPC_BLOCK_MAGIC_OFFSET);

    } while( (magic_bits != IPC_BLOCK_MAGIC) && timeout-- );

    printf( "ipc_init finished with 0x%08X and timeout = %i\n", magic_bits, timeout );

    if( magic_bits == IPC_BLOCK_MAGIC )
       return 0;
    else
       return -1;
}

void ipc_shutdown(void) {
    return;
}

static void send_req_atomic(uint32_t req)
{
    //printf("%s(%d): Enter, req = 0x%x\n", __FUNCTION__, __LINE__, req);
    writel(req, (IPC_BASE_ADDR) + (IPCOPEN_IPCASET_OFFSET));
    //printf("%s(%d): Exit\n", __FUNCTION__, __LINE__);    
}

static uint32_t vc_to_host_phys_addr(uint32_t vc_addr)
{
	if (vc_addr < (VC4_IPC_PHYS_ADDR_BASE))
	{
	    return(0);
	}

	return(vc_addr - (VC4_IPC_PHYS_ADDR_BASE) + (ARM_IPC_PHYS_ADDR_BASE));
}

uint32_t ipc_get_service_addr(const uint32_t fourcc, uint32_t *ipc_num)
{
    uint32_t fcc_offset = (uint32_t)((char*)ARM_IPC_PHYS_ADDR_BASE + IPC_BLOCK_INFO_OFFSET);
    uint32_t vc_service_addr = 0;
    uint32_t arm_service_addr = 0;    
    int blk_num = 0;
    uint32_t start, current, magic_bits = 0;

    /*
    char fourcc_char[5];
    fourcc_char[0] = (fourcc >>  0) & 0xff;
    fourcc_char[1] = (fourcc >>  8) & 0xff;
    fourcc_char[2] = (fourcc >> 16) & 0xff;
    fourcc_char[3] = (fourcc >> 24) & 0xff;
    fourcc_char[4] = '\0';
    printf("get serv addr %s %x\n", fourcc_char, fourcc);
    */

    magic_bits = readl(ARM_IPC_PHYS_ADDR_BASE + IPC_BLOCK_MAGIC_OFFSET);
    if (IPC_BLOCK_MAGIC != magic_bits) {
        printk("Error: VC Has not initialized the IPC block yet, expected [0x%x], read [0x%x]\n", (uint32_t)IPC_BLOCK_MAGIC, (uint32_t)magic_bits);
        return 0;
    }

    for (blk_num = 0; blk_num < IPC_BLOCK_MAX_NUM_USERS; blk_num++) {
        start = 0;
        current  = readl(fcc_offset + offsetof(IPC_BLOCK_USER_INFO_T, four_cc));

        if (current == fourcc) {
            //printf("Found fourcc!\n");
            vc_service_addr = readl(fcc_offset + offsetof(IPC_BLOCK_USER_INFO_T, block_base_address));
            arm_service_addr = vc_to_host_phys_addr(vc_service_addr);
            if (ipc_num != NULL) {
                *ipc_num = readl(fcc_offset + offsetof(IPC_BLOCK_USER_INFO_T, interrupt_number_in_ipc));
            }
            break;
        } else if (current == 0) {
            break;
        }

	/*
        fourcc_char[0] = (current >>  0) & 0xff;
        fourcc_char[1] = (current >>  8) & 0xff;
        fourcc_char[2] = (current >> 16) & 0xff;
        fourcc_char[3] = (current >> 24) & 0xff;
        fourcc_char[4] = '\0';
        printf("skipping fourcc %s %x\n",fourcc_char, current);
        */

        fcc_offset += sizeof(IPC_BLOCK_USER_INFO_T);
    }
    if (current == 0) {
        printf("Error: fourcc 0x%x not found\n", fourcc);
    }

    return arm_service_addr;
}

int ipc_notify_vc_event(uint32_t ipc_id){

    uint32_t vc_irq_status = 0;

    //printf("ipc notify vc event %d\n", ipc_id);
    
    vc_irq_status |= (0x1 << ipc_id);
    
    send_req_atomic(vc_irq_status);
    
    //printf("ipc notify vc event %d done \n", ipc_id);
    
    return 0;

}


uint32_t ipc_wait_event(uint32_t req_ipc)
{
   return(0);
}

