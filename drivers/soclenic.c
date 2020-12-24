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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>

#if (CONFIG_COMMANDS & CFG_CMD_NET) \
	&& defined(CONFIG_SOCLENIC)

#include <malloc.h>
#include <net.h>
#include <pci.h>

unsigned int soclenic_iobase[1] = {0x19C80000};

/* Ethernet chip registers.
 */
#define DE4X5_BMR	0x000		/* Bus Mode Register */
#define DE4X5_TPD	0x008		/* Transmit Poll Demand Reg */
#define DE4X5_RRBA	0x018		/* RX Ring Base Address Reg */
#define DE4X5_TRBA	0x020		/* TX Ring Base Address Reg */
#define DE4X5_STS	0x028		/* Status Register */
#define DE4X5_OMR	0x030		/* Operation Mode Register */
#define DE4X5_SICR	0x068		/* SIA Connectivity Register */
#define DE4X5_APROM	0x048		/* Ethernet Address PROM */

/* Register bits.
 */
#define BMR_SWR		0x00000001	/* Software Reset */
#define STS_TS		0x00700000	/* Transmit Process State */
#define STS_RS		0x000e0000	/* Receive Process State */
#define OMR_ST		0x00002000	/* Start/Stop Transmission Command */
#define OMR_SR		0x00000002	/* Start/Stop Receive */
#define OMR_PS		0x00040000	/* Port Select */
#define OMR_SDP		0x02000000	/* SD Polarity - MUST BE ASSERTED */
#define OMR_PM		0x00000080	/* Pass All Multicast */
#define OMR_PR		0x00000040      /* Promiscuous mode */
#define OMR_OP		0x80000000      /* MAC operate in 100 */


/* Descriptor bits.
 */
#define R_OWN		0x80000000	/* Own Bit */
#define RD_RER		0x02000000	/* Receive End Of Ring */
#define RD_LS		0x00000100	/* Last Descriptor */
#define RD_ES		0x00008000	/* Error Summary */
#define TD_TER		0x02000000	/* Transmit End Of Ring */
#define T_OWN		0x80000000	/* Own Bit */
#define TD_LS		0x40000000	/* Last Segment */
#define TD_FS		0x20000000	/* First Segment */
#define TD_ES		0x00008000	/* Error Summary */
#define TD_SET		0x08000000	/* Setup Packet */

#define POLL_DEMAND	1

#define RESET_DE4X5(dev) {\
    int i;\
    i=INL(dev, DE4X5_BMR);\
    udelay(1000);\
    OUTL(dev, i | BMR_SWR, DE4X5_BMR);\
    udelay(1000);\
    OUTL(dev, i, DE4X5_BMR);\
    udelay(1000);\
    for (i=0;i<5;i++) {INL(dev, DE4X5_BMR); udelay(10000);}\
    udelay(1000);\
}

#define START_DE4X5(dev) {\
    s32 omr; \
    omr = INL(dev, DE4X5_OMR);\
    omr |= OMR_ST | OMR_SR;\
    OUTL(dev, omr, DE4X5_OMR);		/* Enable the TX and/or RX */\
}

#define STOP_DE4X5(dev) {\
    s32 omr; \
    omr = INL(dev, DE4X5_OMR);\
    omr &= ~(OMR_ST|OMR_SR);\
    OUTL(dev, omr, DE4X5_OMR);		/* Disable the TX and/or RX */ \
}

#define NUM_RX_DESC PKTBUFSRX
#define NUM_TX_DESC 1			/* Number of TX descriptors   */
#define RX_BUFF_SZ  PKTSIZE_ALIGN

#define TOUT_LOOP   1000000

#define SETUP_FRAME_LEN 192
#define ETH_ALEN	6

struct de4x5_desc {
	volatile s32 status;
	u32 des1;
	u32 buf;
	u32 next;
};

static struct de4x5_desc rx_ring[NUM_RX_DESC] __attribute__ ((aligned(32))); /* RX descriptor ring         */
static struct de4x5_desc tx_ring[NUM_TX_DESC] __attribute__ ((aligned(32))); /* TX descriptor ring         */
static int rx_new;                             /* RX descriptor ring pointer */
static int tx_new;                             /* TX descriptor ring pointer */

static char rxRingSize;
static char txRingSize;

static void  send_setup_frame(struct eth_device* dev, bd_t * bis);

static int   soclenic_init(struct eth_device* dev, bd_t* bis);
static int   soclenic_send(struct eth_device* dev, volatile void *packet, int length);
static int   soclenic_recv(struct eth_device* dev);
static void  soclenic_halt(struct eth_device* dev);
/*
#if defined(CONFIG_E500)
#define phys_to_bus(a) (a)
#else
#define phys_to_bus(a)	pci_phys_to_mem((pci_dev_t)dev->priv, a)
#endif
*/
static int INL(struct eth_device* dev, u_long addr)
{
	return le32_to_cpu(*(volatile u_long *)(addr + dev->iobase));
}

static void OUTL(struct eth_device* dev, int command, u_long addr)
{
	*(volatile u_long *)(addr + dev->iobase) = cpu_to_le32(command);
}


struct eth_device	soclenic_device[2];

int soclenic_initialize(bd_t *bis)
{
	int             	idx=0;
	int             	card_number = 0;
	int             	cfrv;
	unsigned char   	timer;
	unsigned int		iobase;
	unsigned short		status;
	struct eth_device* 	dev;

	iobase = soclenic_iobase[card_number];

	dev = &soclenic_device[card_number];
	

	sprintf(dev->name, "soclenic#%d", card_number);

	dev->iobase = iobase;


	dev->init   = soclenic_init;
	dev->halt   = soclenic_halt;
	dev->send   = soclenic_send;
	dev->recv   = soclenic_recv;
	/* Ensure we're not sleeping. */
	udelay(10 * 1000);

	dev->init(dev, bis);

	eth_register(dev);
	
	return card_number;
}

static int soclenic_init(struct eth_device* dev, bd_t* bis)
{
	unsigned long	i;
	/* Ensure we're not sleeping. */

	RESET_DE4X5(dev);

	if ((INL(dev, DE4X5_STS) & (STS_TS | STS_RS)) != 0) {
		printf("Error: Cannot reset ethernet controller.\n");
		return 0;
	}

	//check if AST 2000 in RMII Mode
	//In MII Mode we don't care 10/100 Mb/s , while in RMII 
	//we should take care CSR6 bit 31, here, we force bit31 as 1
	i=  (*((volatile long *)0x1E6E0170) & 0x10000) >> 16;
        if (i) {   
		//Enable promiscous and 100 Mb/s
            OUTL(dev, OMR_OP | OMR_SDP | OMR_PS | OMR_PM | OMR_PR, DE4X5_OMR);
	}
	else {
   	    OUTL(dev, OMR_SDP | OMR_PS | OMR_PM, DE4X5_OMR);
        }

	for (i = 0; i < NUM_RX_DESC; i++) {
		rx_ring[i].status = cpu_to_le32(R_OWN);
		rx_ring[i].des1 = cpu_to_le32(RX_BUFF_SZ);
		rx_ring[i].buf = cpu_to_le32((u32) NetRxPackets[i]);
		rx_ring[i].next = 0;
	}

	for (i=0; i < NUM_TX_DESC; i++) {
		tx_ring[i].status = 0;
		tx_ring[i].des1 = 0;
		tx_ring[i].buf = 0;
		tx_ring[i].next = 0;
	}

	rxRingSize = NUM_RX_DESC;
	txRingSize = NUM_TX_DESC;

	/* Write the end of list marker to the descriptor lists. */
	rx_ring[rxRingSize - 1].des1 |= cpu_to_le32(RD_RER);
	tx_ring[txRingSize - 1].des1 |= cpu_to_le32(TD_TER);

	/* Tell the adapter where the TX/RX rings are located. */
	OUTL(dev, ((u32) &rx_ring), DE4X5_RRBA);
	OUTL(dev, ((u32) &tx_ring), DE4X5_TRBA);

	START_DE4X5(dev);

	tx_new = 0;
	rx_new = 0;

	send_setup_frame(dev, bis);

	return 1;
}

static int soclenic_send(struct eth_device* dev, volatile void *packet, int length)
{
	int		status = -1;
	int		i;

	if (length <= 0) {
		printf("%s: bad packet size: %d\n", dev->name, length);
		goto Done;
	}

	for(i = 0; tx_ring[tx_new].status & cpu_to_le32(T_OWN); i++) {
		if (i >= TOUT_LOOP) {
			printf("%s: tx error buffer not ready\n", dev->name);
			goto Done;
		}
	}
	tx_ring[tx_new].buf    = cpu_to_le32(((u32) packet));
	tx_ring[tx_new].des1   = cpu_to_le32(TD_TER | TD_LS | TD_FS | length);
	tx_ring[tx_new].status = cpu_to_le32(T_OWN);

	OUTL(dev, POLL_DEMAND, DE4X5_TPD);

	for (i = 0; tx_ring[tx_new].status & cpu_to_le32(T_OWN); i++) 
	{
		if (i >= TOUT_LOOP) 
		{
			printf(".%s: tx buffer not ready\n", dev->name);
			goto Done;
		}
	}

	if (le32_to_cpu(tx_ring[tx_new].status) & TD_ES) {
		tx_ring[tx_new].status = 0x0;
		goto Done;
	}

	status = length;

 Done:
    tx_new = (tx_new+1) % NUM_TX_DESC;
	return status;
}

static int soclenic_recv(struct eth_device* dev)
{
	s32		status;
	int		length    = 0;

        
	for ( ; ; ) 
	{
		status = (s32)le32_to_cpu(rx_ring[rx_new].status);

		if (status & R_OWN) {
			break;
		}

		if (status & RD_LS) {
			/* Valid frame status.
			 */
			if (status & RD_ES) {

				/* There was an error.
				 */
				printf("RX error status = 0x%08X\n", status);
			} else {
				/* A valid frame received.
				 */
				length = (le32_to_cpu(rx_ring[rx_new].status) >> 16);

				/* Pass the packet up to the protocol
				 * layers.
				 */
				NetReceive(NetRxPackets[rx_new], length - 4);
			}

			/* Change buffer ownership for this frame, back
			 * to the adapter.
			 */
			rx_ring[rx_new].status = cpu_to_le32(R_OWN);
		}

		/* Update entry information.
		 */
		rx_new = (rx_new + 1) % rxRingSize;
	}

	return length;
}

static void soclenic_halt(struct eth_device* dev)
{
	STOP_DE4X5(dev);
	OUTL(dev, 0, DE4X5_SICR);
}


static void send_setup_frame(struct eth_device* dev, bd_t *bis)
{
	int     i;
	char	setup_frame[SETUP_FRAME_LEN];
	char 	*pa = &setup_frame[0];

	memset(pa, 0xff, SETUP_FRAME_LEN);

	for (i = 0; i < ETH_ALEN; i++) 
	{
		*(pa + (i & 1)) = dev->enetaddr[i];
		if (i & 0x01) 
			pa += 4;
		
	}

	for (i = 0; tx_ring[tx_new].status & cpu_to_le32(T_OWN); i++) 
	{
		if (i >= TOUT_LOOP) 
		{
			printf("%s: tx error buffer not ready\n", dev->name);
			goto Done;
		}
	}

	tx_ring[tx_new].buf = cpu_to_le32(((u32) &setup_frame[0]));
	tx_ring[tx_new].des1 = cpu_to_le32(TD_TER | TD_SET| SETUP_FRAME_LEN);
	tx_ring[tx_new].status = cpu_to_le32(T_OWN);

	OUTL(dev, POLL_DEMAND, DE4X5_TPD);

	for (i = 0; tx_ring[tx_new].status & cpu_to_le32(T_OWN); i++) 
	{
		if (i >= TOUT_LOOP) 
		{
			printf("%s: tx buffer not ready\n", dev->name);
			goto Done;
		}
	}
	tx_new = (tx_new+1) % NUM_TX_DESC;

Done:
	return;
}





#endif	/* CFG_CMD_NET && CONFIG_NET_MULTI && CONFIG_SOCLENIC */
