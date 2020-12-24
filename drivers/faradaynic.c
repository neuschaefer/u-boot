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

#if (CONFIG_COMMANDS & CFG_CMD_NET) && defined(CONFIG_NET_MULTI) \
	&& defined(CONFIG_FARADAYNIC)

#include <malloc.h>
#include <net.h>
#include <pci.h>


#define pci_find_devices    NULL
#define pci_read_config_dword NULL
unsigned int faradaynic_iobase[1] = {0x1E660000};

#undef DEBUG_SROM
#undef DEBUG_SROM2

#undef UPDATE_SROM

/* PCI Registers.
 */
#define PCI_CFDA_PSM		0x43

#define CFRV_RN		0x000000f0	/* Revision Number */

#define WAKEUP		0x00		/* Power Saving Wakeup */
#define SLEEP		0x80		/* Power Saving Sleep Mode */

#define DC2114x_BRK	0x0020		/* CFRV break between DC21142 & DC21143 */

/* MAC chip register */
#define ISR_REG				0x00				// interrups status register
#define IER_REG				0x04				// interrupt maks register
#define MAC_MADR_REG		        0x08				// MAC address (Most significant)
#define MAC_LADR_REG		        0x0c				// MAC address (Least significant)

#define MAHT0_REG			0x10				// Multicast Address Hash Table 0 register
#define MAHT1_REG			0x14				// Multicast Address Hash Table 1 register
#define TXPD_REG			0x18				// Transmit Poll Demand register
#define RXPD_REG			0x1c				// Receive Poll Demand register
#define TXR_BADR_REG		        0x20				// Transmit Ring Base Address register
#define RXR_BADR_REG		        0x24				// Receive Ring Base Address register

#define HPTXPD_REG			0x28	//
#define HPTXR_BADR_REG		        0x2c	//

#define ITC_REG				0x30				// interrupt timer control register
#define APTC_REG			0x34				// Automatic Polling Timer control register
#define DBLAC_REG			0x38				// DMA Burst Length and Arbitration control register

#define DMAFIFOS_REG		        0x3c	//
#define FEAR_REG			0x44	//
#define TPAFCR_REG			0x48	//
#define RBSR_REG			0x4c	//for NC Body
#define MACCR_REG			0x50				// MAC control register
#define MACSR_REG			0x54				// MAC status register
#define PHYCR_REG			0x60				// PHY control register
#define PHYDATA_REG			0x64				// PHY Write Data register
#define FCR_REG				0x68				// Flow Control register
#define BPR_REG				0x6c				// back pressure register
#define WOLCR_REG			0x70				// Wake-On-Lan control register
#define WOLSR_REG			0x74				// Wake-On-Lan status register
#define WFCRC_REG			0x78				// Wake-up Frame CRC register
#define WFBM1_REG			0x80				// wake-up frame byte mask 1st double word register
#define WFBM2_REG			0x84				// wake-up frame byte mask 2nd double word register
#define WFBM3_REG			0x88				// wake-up frame byte mask 3rd double word register
#define WFBM4_REG			0x8c				// wake-up frame byte mask 4th double word register

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

// --------------------------------------------------------------------
//		MACCR_REG
// --------------------------------------------------------------------

#define SW_RST_bit			(1UL<<31)				// software reset/
#define DIRPATH_bit			(1UL<<21)	
#define RX_IPCS_FAIL_bit	(1UL<<20)	//
#define RX_TCPCS_FAIL_bit	(1UL<<19)	//
#define RX_UDPCS_FAIL_bit	(1UL<<18)	//
#define RX_BROADPKT_bit		(1UL<<17)				// Receiving broadcast packet
#define RX_MULTIPKT_bit		(1UL<<16)				// receiving multicast packet
#define RX_HT_EN_bit		(1UL<<15)
#define RX_ALLADR_bit		(1UL<<14)				// not check incoming packet's destination address
#define JUMBO_LF_bit		(1UL<<13)	//
#define RX_RUNT_bit			(1UL<<12)				// Store incoming packet even its length is les than 64 byte
#define CRC_CHK_bit			(1UL<<11)	//
#define CRC_APD_bit			(1UL<<10)				// append crc to transmit packet
#define GMAC_MODE_bit		(1UL<<9)	//
#define FULLDUP_bit			(1UL<<8)				// full duplex
#define ENRX_IN_HALFTX_bit	(1UL<<7)	//
#define LOOP_EN_bit			(1UL<<6)				// Internal loop-back
#define HPTXR_EN_bit		(1UL<<5)	//
#define REMOVE_VLAN_bit		(1UL<<4)	//
//#define MDC_SEL_bit		(1UL<<13)				// set MDC as TX_CK/10
//#define RX_FTL_bit		(1UL<<11)				// Store incoming packet even its length is great than 1518 byte
#define RXMAC_EN_bit		(1UL<<3)				// receiver enable
#define TXMAC_EN_bit		(1UL<<2)				// transmitter enable
#define RXDMA_EN_bit		(1UL<<1)				// enable DMA receiving channel
#define TXDMA_EN_bit		(1UL<<0)				// enable DMA transmitting channel

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
#define OMR_PR		0x00000040  /* Promiscuous mode */
#define OMR_OP		0x80000000  /* MAC operate in 100 */


/* Descriptor bits.
 */
#define TXDMA_OWN	0x80000000	/* Own Bit */
#define RXPKT_RDY       0x00000000      
#define RXPKT_STATUS    0x80000000
//#define EDORR		0x00008000	/* Receive End Of Ring */
#define EDORR		0x40000000	/* Receive End Of Ring */
#define LRS		0x10000000	/* Last Descriptor */
#define RD_ES		0x00008000	/* Error Summary */
//#define EDOTR		0x00008000	/* Transmit End Of Ring */
#define EDOTR		0x40000000	/* Transmit End Of Ring */
#define T_OWN		0x80000000	/* Own Bit */
#define LTS		0x10000000	/* Last Segment */
#define FTS		0x20000000	/* First Segment */
#define CRC_ERR         0x00080000
#define TD_ES		0x00008000	/* Error Summary */
#define TD_SET		0x08000000	/* Setup Packet */
#define RX_ERR          0x00040000
#define FTL             0x00100000
#define RUNT            0x00200000
#define RX_ODD_NB       0x00400000

#define POLL_DEMAND	1
#define RESET_DE4X5(dev) {\
    int i;\
    i=INL(dev, MACCR_REG);\
    udelay(1000);\
    OUTL(dev, i | SW_RST_bit, MACCR_REG);\
    for (; (INL(dev, MACCR_REG ) & SW_RST_bit) != 0; ) {udelay(1000);}\
    OUTL(dev, 0, IER_REG );	\
}

#define START_MAC(dev) {\
    s32 omr; \
    omr = INL(dev, MACCR_REG);\
    omr |= RXMAC_EN_bit | TXMAC_EN_bit | RXDMA_EN_bit | TXDMA_EN_bit;\
    OUTL(dev, omr, MACCR_REG);		/* Enable the TX and/or RX */\
}

#define STOP_MAC(dev) {\
    s32 omr; \
    omr = INL(dev, MACCR_REG);\
    omr &= ~(RXMAC_EN_bit | TXMAC_EN_bit | RXDMA_EN_bit | TXDMA_EN_bit);\
    OUTL(dev, omr, MACCR_REG);		/* Disable the TX and/or RX */ \
}

#define NUM_RX_DESC PKTBUFSRX
#define NUM_TX_DESC 1			/* Number of TX descriptors   */
#define RX_BUFF_SZ  PKTSIZE_ALIGN

#define TOUT_LOOP   1000000
#define ETH_ALEN	6

struct de4x5_desc {
	volatile s32 status;
	u32 des1;
	u32 reserved;
	u32 buf;	
};

static struct de4x5_desc rx_ring[NUM_RX_DESC] __attribute__ ((aligned(32))); /* RX descriptor ring         */
static struct de4x5_desc tx_ring[NUM_TX_DESC] __attribute__ ((aligned(32))); /* TX descriptor ring         */
static int rx_new;                             /* RX descriptor ring pointer */
static int tx_new;                             /* TX descriptor ring pointer */

static char rxRingSize;
static char txRingSize;


static unsigned short ftgmac100_read_phy_register(unsigned int ioaddr, unsigned char phyaddr, unsigned char phyreg);
static void ftgmac100_write_phy_register(unsigned int ioaddr, unsigned char phyaddr, unsigned char phyreg, unsigned short phydata);

static void  send_setup_frame(struct eth_device* dev, bd_t * bis);

static int   faradaynic_init(struct eth_device* dev, bd_t* bis);
static int   faradaynic_send(struct eth_device* dev, volatile void *packet, int length);
static int   faradaynic_recv(struct eth_device* dev);
static void  faradaynic_halt(struct eth_device* dev);

#if defined(CONFIG_E500)
#define phys_to_bus(a) (a)
#else
#define phys_to_bus(a)	pci_phys_to_mem((pci_dev_t)dev->priv, a)
#endif

static int INL(struct eth_device* dev, u_long addr)
{
	return le32_to_cpu(*(volatile u_long *)(addr + dev->iobase));
}

static void OUTL(struct eth_device* dev, int command, u_long addr)
{
	*(volatile u_long *)(addr + dev->iobase) = cpu_to_le32(command);
}


struct eth_device	faradaynic_device[1];

int faradaynic_initialize(bd_t *bis)
{
	int             	card_number = 0;
	unsigned int		iobase;
	struct eth_device* 	dev;

	iobase = faradaynic_iobase[card_number];

	dev = &faradaynic_device[card_number];
	

	sprintf(dev->name, "faradaynic#%d", card_number);

	dev->iobase = iobase;


	dev->init   = faradaynic_init;
	dev->halt   = faradaynic_halt;
	dev->send   = faradaynic_send;
	dev->recv   = faradaynic_recv;

	/* Ensure we're not sleeping. */
	udelay(10 * 1000);

	dev->init(dev, bis);


	eth_register(dev);
	

	return card_number;
}

static int faradaynic_init(struct eth_device* dev, bd_t* bis)
{
	unsigned long	i;

	RESET_DE4X5(dev);
	i=  (*((volatile long *)0x1E6E0170) & 0x10000) >> 16;
        if (i) {   
            OUTL(dev, OMR_OP | OMR_SDP | OMR_PS | OMR_PM | OMR_PR, MACCR_REG);
	}
	else
	    OUTL(dev, RX_TCPCS_FAIL_bit | RX_ALLADR_bit | FULLDUP_bit | \
RXMAC_EN_bit | RXDMA_EN_bit | TXMAC_EN_bit | TXDMA_EN_bit | CRC_APD_bit, MACCR_REG);

	for (i = 0; i < NUM_RX_DESC; i++) {
		rx_ring[i].status = cpu_to_le32(RXPKT_RDY + RX_BUFF_SZ);
		rx_ring[i].buf = cpu_to_le32((u32) NetRxPackets[i]);
		rx_ring[i].reserved = 0;
	}

	for (i=0; i < NUM_TX_DESC; i++) {
		tx_ring[i].status = 0;
		tx_ring[i].des1 = 0;
		tx_ring[i].buf = 0;
		tx_ring[i].reserved = 0;
	}

	rxRingSize = NUM_RX_DESC;
	txRingSize = NUM_TX_DESC;

	rx_ring[rxRingSize - 1].status |= cpu_to_le32(EDORR);
	tx_ring[txRingSize - 1].status |= cpu_to_le32(EDOTR);

	OUTL(dev, ((u32) &tx_ring), TXR_BADR_REG);
	OUTL(dev, ((u32) &rx_ring), RXR_BADR_REG);

	START_MAC(dev);

	tx_new = 0;
	rx_new = 0;

	return 1;
}

static int faradaynic_send(struct eth_device* dev, volatile void *packet, int length)
{
	int		status = -1, oldlength = 0, fail = 0;
	int		i;

	if (length <= 0) {
		printf("%s: bad packet size: %d\n", dev->name, length);
		goto Done;
	}


	for(i = 0; (tx_ring[tx_new].status & cpu_to_le32(TXDMA_OWN)) == 0x80000000; i++) {
		if (i >= TOUT_LOOP) {
			printf("%s: tx error buffer not ready\n", dev->name);
                        fail = 1;
			goto Done;
		}
	}
        

        if (length < 60) {
            oldlength = length;
            memset ((void *)cpu_to_le32((u32) (packet + length)), 0, 60 - length);
            length = 60;
        }
	tx_ring[tx_new].buf    = cpu_to_le32(((u32) packet));
	tx_ring[tx_new].status   |= cpu_to_le32(LTS | FTS | length);
	tx_ring[tx_new].status |= cpu_to_le32(TXDMA_OWN);
        
	OUTL(dev, POLL_DEMAND, TXPD_REG);

	for (i = 0; (tx_ring[tx_new].status & cpu_to_le32(TXDMA_OWN)) == 0x80000000; i++) 
	{
		if (i >= TOUT_LOOP) 
		{
			printf(".%s: tx buffer not ready\n", dev->name);
                        fail = 1;
			goto Done;
		}
	}

	if (le32_to_cpu(tx_ring[tx_new].status) & CRC_ERR) {
		tx_ring[tx_new].status = 0x0;
                fail = 1;
		goto Done;
	}
        
        if (fail != 1) {
            status = oldlength;
        }

 Done:
    tx_new = (tx_new+1) % NUM_TX_DESC;

	return status;
}

static int faradaynic_recv(struct eth_device* dev)
{
	s32		status;
	int		length    = 0;

	for ( ; ; ) 
	{
		status = (s32)le32_to_cpu(rx_ring[rx_new].status);

		if ((status & RXPKT_STATUS) == 0) {
			break;
		}

		if (status & LRS) {
			/* Valid frame status.
			 */
			if (status & (RX_ERR | CRC_ERR | FTL | RUNT | RX_ODD_NB)) {

				/* There was an error.
				 */
				printf("RX error status = 0x%08X\n", status);
			} else {
				/* A valid frame received.
				 */
				length = (le32_to_cpu(rx_ring[rx_new].status) & 0x3FFF);
				/* Pass the packet up to the protocol
				 * layers.
				 */
				NetReceive(NetRxPackets[rx_new], length - 4);
			}

			/* Change buffer ownership for this frame, back
			 * to the adapter.
			 */
			rx_ring[rx_new].status &= cpu_to_le32(0x7FFFFFFF);
//			rx_ring[rx_new].status = cpu_to_le32(RXPKT_RDY);
		}

		/* Update entry information.
		 */
		rx_new = (rx_new + 1) % rxRingSize;
	}

	return length;
}

static void faradaynic_halt(struct eth_device* dev)
{
	STOP_MAC(dev);
}

#endif	/* CFG_CMD_NET && CONFIG_NET_MULTI && CONFIG_FARADAYMAC */
