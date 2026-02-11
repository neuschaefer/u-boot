/*****************************************************************************
* Copyright 2006 - 2011 Broadcom Corporation.  All rights reserved.
*
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed to you
* under the terms of the GNU General Public License version 2, available at
* http://www.broadcom.com/licenses/GPLv2.php (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a
* license other than the GPL, without Broadcom's express prior written
* consent.
*****************************************************************************/
//#define BCM11140_ETH_DEBUG

#ifdef BCM11140_ETH_DEBUG
#ifndef DEBUG
#define DEBUG
#endif
#endif

#include <common.h>
#include <malloc.h>
#include <net.h>
#include <config.h>

#include <asm/io.h>
#include <asm/sizes.h>
#include <asm/arch/ethHw.h>
#include <asm/arch/ethHw_dma.h>
#include <asm/gpio.h>

#include <asm/arch/brcm_rdb_esub_clk_mgr_reg.h>
#include <asm/arch/brcm_rdb_esw_busif_imp.h>
#include <asm/arch/brcm_rdb_padctrlreg.h>
#include <asm/arch/brcm_rdb_chipreg.h>

static void configureMode(int port, int mii_mode);
static int configurePhy(int port);
static int pollLink(int port);

/* phy_msw is the contents of PHY register 0x2, phy_lsw is the contents of PHY register 0x3 */
#define MAKE_PHY_ID(phy_msw, phy_lsw) ((((phy_msw) & 0xFFFF) << 16) | ((phy_lsw) & 0xFFFF))

/* Convenience names */
#define PHY_BCM54612E   MAKE_PHY_ID(0x0362, 0x5E6A)
#define PHY_BCM5241     MAKE_PHY_ID(0x0143, 0xBC31)

/* If there are more than 1 external port, assume they use the same type of PHY */
static uint32_t phy_id;

#ifdef CONFIG_BCM11140_FPGA
#define GPIO_PHY_RST0	8
#define GPIO_PHY_RST1	9
#else
//#define GPIO_PHY_RST0	53
//#define GPIO_PHY_RST1	55
#define GPIO_PHY_RST0	10
#define GPIO_PHY_RST1	55 //un-used GPIO
#endif

#define BCM11140_ETH_DEV_NAME          "bcm11140_eth"
#define ETH_DMA_CONTROLLER     0     // DMA0

#define BCM_NET_MODULE_DESCRIPTION    "Broadcom BCM11140 Ethernet driver"
#define BCM_NET_MODULE_VERSION        "0.1"

#define ETH_DMA_CH_RX         1     // PTM
#define ETH_DMA_CH_TX         0     // MTP

#define ETH_DMA_BURST_SIZE    8
#define ETH_DMA_BLOCK_SIZE    256

/* Memory for RX buffers and RX DMA descriptors. */
#define RX_BUF_SIZE        2048
#define RX_BUF_NUM         8
#define RX_DESC_NUM        RX_BUF_NUM

static uint8_t *rx_tx_buffer, *aligned_rx_tx_buffer;
#define RX_BUF_BASE        (aligned_rx_tx_buffer)
#define RX_DESC_BASE       (RX_BUF_BASE + (RX_BUF_NUM * RX_BUF_SIZE))

#define RX_BUF(i)          (uint8_t *)(RX_BUF_BASE + ((i) * RX_BUF_SIZE))
#define RX_DESC(i)         (ETHHW_DMA_DESC *)(RX_DESC_BASE + ((i) * sizeof( ETHHW_DMA_DESC )))
#define RX_FLUSH_CACHE()

/* Memory for TX buffers and TX DMA descriptors.
 * For every Tx buffer, there must be two descriptors (one for config and one
 * for data) and one config buffer (for config information)
 */
#define TX_BUF_SIZE        2048
#define TX_BUF_NUM         1
#define TX_DESC_NUM        (TX_BUF_NUM * 2)  // Require one config and one data descriptor for every data buffer
#define CFG_BUF_SIZE       8
#define CFG_BUF_NUM        TX_BUF_NUM

/* Starting from RX_BUF_BASE + 128KB for Tx buffer */
#define TX_BUF_BASE        (RX_BUF_BASE + 0x20000)
#define TX_DESC_BASE       (TX_BUF_BASE + (TX_BUF_NUM * TX_BUF_SIZE))
#define CFG_BUF_BASE       (TX_DESC_BASE + (TX_DESC_NUM * sizeof( ETHHW_DMA_DESC )))

#define TX_BUF(i)          (uint8_t *)(TX_BUF_BASE + ((i) * TX_BUF_SIZE))
#define TX_DESC(i)         (ETHHW_DMA_DESC *)(TX_DESC_BASE + ((i) * sizeof( ETHHW_DMA_DESC )))
#define CFG_BUF(i)         (uint8_t *)(CFG_BUF_BASE + ((i) * CFG_BUF_SIZE))

#define TX_FLUSH_CACHE()

#define mdelay(n)	udelay((n)*1000)

static const char banner[] = BCM_NET_MODULE_DESCRIPTION " " BCM_NET_MODULE_VERSION "\n";

#ifdef BCM11140_ETH_DEBUG
static int bcm11140_eth_info( void );

static int bcm11140_eth_info( void )
{
   printf( "\n" );
   printf( "Ethernet Console\n" );
   printf( "================\n" );
   printf( "\n" );
   printf( __FILE__ " built on " __DATE__ " at " __TIME__ "\n" );
   printf( "\n" );
   printf( "Buffer              Address     Num   Size\n" );
   printf( "--------------    ----------   ----   ----\n" );
   printf( "Rx Data           0x%08x   %4i   %4i\n", (int)RX_BUF( 0 ), RX_BUF_NUM, RX_BUF_SIZE );
   printf( "Rx Descriptors    0x%08x   %4i\n", (int)RX_DESC( 0 ), RX_DESC_NUM );
   printf( "Tx Data           0x%08x   %4i   %4i\n", (int)TX_BUF( 0 ), TX_BUF_NUM, TX_BUF_SIZE );
   printf( "Tx Descriptors    0x%08x   %4i\n", (int)TX_DESC( 0 ), TX_DESC_NUM  );
   printf( "Config Data       0x%08x   %4i   %4i\n", (int)CFG_BUF( 0 ), CFG_BUF_NUM, CFG_BUF_SIZE );
   printf( "\n" );
   printf( "DMA Config     Value\n" );
   printf( "----------     -----\n" );
   printf( "Controller      %4i\n", ETH_DMA_CONTROLLER );
   printf( "Rx Channel      %4i\n", ETH_DMA_CH_RX );
   printf( "Tx Channel      %4i\n", ETH_DMA_CH_TX );
   printf( "Burst Size      %4i\n", ETH_DMA_BURST_SIZE );
   printf( "Block Size      %4i\n", ETH_DMA_BLOCK_SIZE );
   printf( "\n" );

   return 0;
}

static void txDump(int index, int len)
{
   uint8_t *bufp;
   int i;

   bufp = (uint8_t *) TX_BUF(index);

   debug("Tx Buf: idx %d len=%d\n", index, len);
   for (i = 0; i < len; i++) {
       if ((i + 1) % 16)
       {
           debug("%02X ", bufp[i]);
       }
       else
       {
           debug("%02X\n", bufp[i]);
       }
   }
   debug("\n");
}
#endif

#ifdef CONFIG_BCM11140_ETH
static int ethPhyStart( void )
{
#ifndef CONFIG_BCM11140_FPGA
   /*** Enable PLL-E ***/

   /* Enable Access to CCU registers */
   writel(
      (1 << ESUB_CLK_MGR_REG_WR_ACCESS_CLKMGR_ACC_SHIFT) |
      (0xA5A5 << ESUB_CLK_MGR_REG_WR_ACCESS_PASSWORD_SHIFT),
      ESUB_CLK_BASE_ADDR + ESUB_CLK_MGR_REG_WR_ACCESS_OFFSET);

   writel(
      readl(ESUB_CLK_BASE_ADDR + ESUB_CLK_MGR_REG_PLLE_POST_RESETB_OFFSET) &
      ~ESUB_CLK_MGR_REG_PLLE_POST_RESETB_I_POST_RESETB_PLLE_MASK,
      ESUB_CLK_BASE_ADDR + ESUB_CLK_MGR_REG_PLLE_POST_RESETB_OFFSET);

   /* Take PLL out of reset and put into normal mode */
   writel(
      readl(ESUB_CLK_BASE_ADDR + ESUB_CLK_MGR_REG_PLLE_RESETB_OFFSET) |
      ESUB_CLK_MGR_REG_PLLE_RESETB_I_PLL_RESETB_PLLE_MASK,
      ESUB_CLK_BASE_ADDR + ESUB_CLK_MGR_REG_PLLE_RESETB_OFFSET);

   /* Wait for PLL lock */
   while( !(readl(ESUB_CLK_BASE_ADDR + ESUB_CLK_MGR_REG_PLL_LOCK_OFFSET) & ESUB_CLK_MGR_REG_PLL_LOCK_PLL_LOCK_PLLE_MASK) ) {;}

   writel(
      readl(ESUB_CLK_BASE_ADDR + ESUB_CLK_MGR_REG_PLLE_POST_RESETB_OFFSET) |
      ESUB_CLK_MGR_REG_PLLE_POST_RESETB_I_POST_RESETB_PLLE_MASK,
      ESUB_CLK_BASE_ADDR + ESUB_CLK_MGR_REG_PLLE_POST_RESETB_OFFSET);

   /* Switch esw_sys_clk to use 104MHz(208MHz/2) clock */
   writel(
      (readl(ESUB_CLK_BASE_ADDR + ESUB_CLK_MGR_REG_ESW_SYS_DIV_OFFSET) &
       ~(ESUB_CLK_MGR_REG_ESW_SYS_DIV_ESW_SYS_PLL_SELECT_MASK | ESUB_CLK_MGR_REG_ESW_SYS_DIV_ESW_SYS_DIV_MASK)) |
      (ESUB_CLK_MGR_REG_ESW_SYS_DIV_ESW_SYS_PLL_SELECT_CMD_VAR_208M_CLK << ESUB_CLK_MGR_REG_ESW_SYS_DIV_ESW_SYS_PLL_SELECT_SHIFT) |
      (1 << ESUB_CLK_MGR_REG_ESW_SYS_DIV_ESW_SYS_DIV_SHIFT),
      ESUB_CLK_BASE_ADDR + ESUB_CLK_MGR_REG_ESW_SYS_DIV_OFFSET);

   writel(
      readl(ESUB_CLK_BASE_ADDR + ESUB_CLK_MGR_REG_ESW_SYS_DIV_OFFSET) |
      ESUB_CLK_MGR_REG_ESW_SYS_DIV_ESW_SYS_TRIGGER_MASK,
      ESUB_CLK_BASE_ADDR + ESUB_CLK_MGR_REG_ESW_SYS_DIV_OFFSET);

   /* Wait for trigger complete */
   while( (readl(ESUB_CLK_BASE_ADDR + ESUB_CLK_MGR_REG_ESW_SYS_DIV_OFFSET) & ESUB_CLK_MGR_REG_ESW_SYS_DIV_ESW_SYS_TRIGGER_MASK) ) {;}


   /* switch Esub AXI clock to 208MHz */
   writel(
      (readl(ESUB_CLK_BASE_ADDR + ESUB_CLK_MGR_REG_ESUB_AXI_DIV_DEBUG_OFFSET) &
       ~(ESUB_CLK_MGR_REG_ESUB_AXI_DIV_DEBUG_ESUB_AXI_PLL_SELECT_MASK |
         ESUB_CLK_MGR_REG_ESUB_AXI_DIV_DEBUG_ESUB_AXI_PLL_SELECT_OVERRIDE_MASK |
         ESUB_CLK_MGR_REG_ESUB_AXI_DIV_DEBUG_ESUB_AXI_TRIGGER_MASK)) |
      (ESUB_CLK_MGR_REG_ESUB_AXI_DIV_DEBUG_ESUB_AXI_PLL_SELECT_CMD_VAR_208M_CLK << ESUB_CLK_MGR_REG_ESUB_AXI_DIV_DEBUG_ESUB_AXI_PLL_SELECT_SHIFT) |
      (ESUB_CLK_MGR_REG_ESUB_AXI_DIV_DEBUG_ESUB_AXI_PLL_SELECT_OVERRIDE_MASK),
      ESUB_CLK_BASE_ADDR + ESUB_CLK_MGR_REG_ESUB_AXI_DIV_DEBUG_OFFSET);

   writel(
      readl(ESUB_CLK_BASE_ADDR + ESUB_CLK_MGR_REG_ESUB_AXI_DIV_DEBUG_OFFSET) |
      ESUB_CLK_MGR_REG_ESUB_AXI_DIV_DEBUG_ESUB_AXI_TRIGGER_MASK,
      ESUB_CLK_BASE_ADDR + ESUB_CLK_MGR_REG_ESUB_AXI_DIV_DEBUG_OFFSET);

   /* Wait for trigger complete */
   while( (readl(ESUB_CLK_BASE_ADDR + ESUB_CLK_MGR_REG_ESUB_AXI_DIV_DEBUG_OFFSET) & ESUB_CLK_MGR_REG_ESUB_AXI_DIV_DEBUG_ESUB_AXI_TRIGGER_MASK) ) {;}

   /* Disable Access to CCU registers */
   writel(
      (0 << ESUB_CLK_MGR_REG_WR_ACCESS_CLKMGR_ACC_SHIFT) |
      (0xA5A5 << ESUB_CLK_MGR_REG_WR_ACCESS_PASSWORD_SHIFT),
      ESUB_CLK_BASE_ADDR + ESUB_CLK_MGR_REG_WR_ACCESS_OFFSET);
#endif

   /* Hold both PHYs in reset */
   gpio_request(GPIO_PHY_RST0, "ephy0_rst");
   gpio_request(GPIO_PHY_RST1, "ephy1_rst");
   gpio_direction_output(GPIO_PHY_RST0, 0);
   gpio_direction_output(GPIO_PHY_RST1, 0);

#ifdef CONFIG_ETH_PHY_POWER_GPIO
   gpio_request(CONFIG_ETH_PHY_POWER_GPIO, "ephy_power_enable");
   gpio_direction_output(CONFIG_ETH_PHY_POWER_GPIO, 0);
   mdelay(10);
   gpio_set_value( CONFIG_ETH_PHY_POWER_GPIO, 1 );
   mdelay( 100 );
#endif
   mdelay( 200 );

   /* Take PORT0 PHY out of reset */
   gpio_set_value( GPIO_PHY_RST0, 1 );
   /* 
    * Even if the hardware supports only 1 phy, both are available 
    * internally so it doesn't hurt to enable both.
    */
   /* Take PORT1 PHY out of reset */
   gpio_set_value( GPIO_PHY_RST1, 1 );

   mdelay( 200 );

   /* Set MDIO address */
   ETHHW_REG_SET( ETHHW_REG32( MM_IO_BASE_ESW + 0x00380 ), 0 );
   ETHHW_REG_SET( ETHHW_REG32( MM_IO_BASE_ESW + 0x00388 ), 1 );

   /* soft-reset */
   ethHw_miiSet( 0, ETHHW_MII_FLAGS_EXT, 0, 0xB100 );
   ethHw_miiSet( 1, ETHHW_MII_FLAGS_EXT, 0, 0xB100 );

   mdelay( 200 );

   configurePhy(0);
   //configurePhy(1);

   return 0;
}
#endif

static int txPacketAdd(int index, size_t len, uint8_t * tx_buf)
{
   uint8_t *bufp;
   ETHHW_DMA_DESC *descp;
   uint64_t *cfgp;

   /* Fill SA and DA */
   memcpy((uint8_t *) TX_BUF(index), tx_buf, 12);
   /* add empty brcmTag */
   memset((uint8_t *) TX_BUF(index) + 12, 0, 4);
   /* Fill other data */
   memcpy((uint8_t *) TX_BUF(index) + 16, tx_buf + 12, len - 12);

   /* Add length of BCM tag */
   len += sizeof(uint32_t);   // Account for addition of Broadcom tag
   
   /* The Ethernet packet has to be >= 64 bytes required by switch 
    * padding it with zeros
    */
   if (len < 64)
   {
      memset((uint8_t *) TX_BUF(index) + len, 0, 64 - len);
      len = 64;
   }

   /* Add 4 bytes for Ethernet FCS/CRC */
   len += 4;

   /* Fill config data 
    * (there is a one-to-one mapping of config data and buffer data) 
    */
   bufp = (uint8_t *) TX_BUF(index);
   cfgp = (uint64_t *) CFG_BUF(index);

   *cfgp = ETHHW_DMA_CFG_EOP_MASK | ETHHW_DMA_CFG_OFFSET( bufp, len );

   /* Setup descriptor */
   bufp = (uint8_t *) TX_BUF(index);
   cfgp = (uint64_t *) CFG_BUF(index);
   descp = (ETHHW_DMA_DESC *) TX_DESC(2 * index);

   // Tx config descriptor
   // The config descriptor allows transfer to be purely 64-bit
   // transactions, spanning an arbitrary number of descriptors,
   // so information must be provided to define offsets and EOP
   ETHHW_DMA_DESC_CREATE( 
      descp,
      cfgp, 
      ETHHW_DMA_MTP_FIFO_ADDR_CFG,
      descp,
      ETHHW_DMA_MTP_CTL_LO, 
      ETHHW_DMA_MTP_TRANSACTION_SIZE( cfgp, sizeof( uint64_t ) ) 
      );

   descp++;

   // Tx data descriptor, only one data buffer will be added
   ETHHW_DMA_DESC_CREATE_NEXT( 
      descp,
      bufp, 
      ETHHW_DMA_MTP_FIFO_ADDR_DATA,
      NULL,
      ETHHW_DMA_MTP_CTL_LO, 
      ETHHW_DMA_MTP_TRANSACTION_SIZE( bufp, len) 
      );

   //Flush data, config, and descriptors to external memory
   TX_FLUSH_CACHE();

#ifdef BCM11140_ETH_DEBUG
   txDump(index, len);
#endif
   return 0;
}

/******************************************************************
 * u-boot net functions
 */
static int bcm11140_eth_send(struct eth_device *dev, volatile void *packet, int length)
{
   uint8_t *buf = (uint8_t *) packet;
   int rc = 0;
   int i = 0;
   
   debug("bcm11140 net start Tx: %d\n", length);

   /* Always use 1st buffer since send is synchronous */
   txPacketAdd(0, length, buf);

   /* Start transmit */
   ethHw_dmaEnable( ETH_DMA_CONTROLLER, ETH_DMA_CH_TX, TX_DESC( 0 ) );

   while (!ETHHW_DMA_MTP_TRANSFER_DONE(TX_DESC(0))) 
   {
      udelay(100);
      debug(".");
      i++;
      if(i > 20)
      {
         error("\nbcm11140 ethernet Tx failure! Already retried 20 times\n");
         rc = -1;
         break;
      }
   }

   ethHw_dmaDisable( ETH_DMA_CONTROLLER, ETH_DMA_CH_TX );

   return rc;
}

static int bcm11140_eth_rcv(struct eth_device *dev)
{
   uint8_t *buf = (uint8_t *) NetRxPackets[0];
   int rc = 0;
   volatile ETHHW_DMA_DESC *descp = RX_DESC( 0 );   // First Rx descriptor
   uint8_t *bufp;
   uint16_t rcvlen;
   uint32_t status;

   //debug("entering bcm11140_eth_rcv\n");

   udelay(50);

   while(1)
   {
      /* Poll Rx queue to get a packet */
      if( ETHHW_DMA_PTM_TRANSFER_DONE( descp ) )
      {
         debug("Recieved\n");
         bufp = (uint8_t *)descp->dar;

         status = descp->stat1;   // Get SSTATx
         //rcvlen = ((status >> 13) & 0x0001ffff);
         rcvlen = ETHHW_DMA_BUF_LEN(descp);
         
         if(( rcvlen == 0) || (rcvlen > RX_BUF_SIZE))
         {
            error("Wrong Rx packet size %d, drop it\n", rcvlen);
            ETHHW_DMA_DESC_RX_UPDATE( descp, descp->dar, RX_BUF_SIZE );
            break;
         }

         memcpy(buf, bufp, 12);
         /* skip brcmTag */
         memcpy(buf + 12, bufp + 16, rcvlen - 12 - 4);
         rcvlen -= 4;

#ifdef BCM11140_ETH_DEBUG
         debug("Rx Buf: len=%d\n", rcvlen);
         int i;
         for (i = 0; i < min(rcvlen, 128); i++) {
            if ((i + 1) % 16)
            {
                debug("%02X ", buf[i]);
            }
            else
            {
                debug("%02X\n", buf[i]);
            }
         }
         debug("\n");
#endif
         /* A packet has been received, so forward to uboot network handler */
         NetReceive(buf, rcvlen);

         ETHHW_DMA_DESC_RX_UPDATE( descp, descp->dar, RX_BUF_SIZE );
      }
      else
      {
         //debug("Rx");
         descp = (ETHHW_DMA_DESC *)(descp)->lli;                   // Advance to next descriptor
         
         if(descp == RX_DESC( 0 ))
         {
            /* Tell caller that no packet was received when Rx queue was polled */
            rc = -1;
            //debug("\nNO Rx\n");
            break;
         }
      }
   }

   return rc;
}

static inline void ethMiiSet(int port, int addr, int data)
{
   //ethHw_miiSet(port, 0, (uint32_t) addr, (uint32_t) data);
}

int bcm11140_miiphy_read(char *devname, unsigned char const addr, 
   unsigned char const reg, unsigned short *const value)
{
    uint32_t RegVal;

    if (ETHHW_RC_NONE == ethHw_miiGet(addr, ETHHW_MII_FLAGS_EXT, (uint32_t) reg, &RegVal))
    {
        *value = (unsigned short)(RegVal & 0xFFFF);
        return 0;
    }

    return -1;
}

int bcm11140_miiphy_write(char *devname, unsigned char const addr, 
   unsigned char const reg, unsigned short const value)
{
    if (ETHHW_RC_NONE == ethHw_miiSet(addr, ETHHW_MII_FLAGS_EXT, (uint32_t) reg, (uint32_t) value))
    {
        return 0;
    }

    return -1;
}

static int bcm11140_eth_write_hwaddr(struct eth_device* dev)
{
   int rc;

   printf("\nMAC: %02x:%02x:%02x:%02x:%02x:%02x\n",
      dev->enetaddr[0], dev->enetaddr[1], dev->enetaddr[2],
      dev->enetaddr[3], dev->enetaddr[4], dev->enetaddr[5]);

   rc = ethHw_arlEntrySet((char *)dev->enetaddr, 0, ETHHW_PORT_INT, 0, 1, 1);

   if (rc != ETHHW_RC_NONE) 
   {
      error("MAC set error!\n");
      return -1;
   }

   return 0;
}

static int bcm11140_eth_open( struct eth_device *dev, bd_t * bt )
{
   uint32_t extPortSpeed;
   ETHHW_DMA_DESC *descp = NULL;
   uint8_t *bufp;
   uint32_t reg32;
   int extPort;
   int i;

   debug( "Enable BCM11140 Ethernet \n" );

   /* Set MAC address from env */
   if (bcm11140_eth_write_hwaddr(dev) != 0) 
   {
      error("MAC set error when opening !\n");
      return -1;
   }

   /* Configure link */
   if (phy_id == PHY_BCM5241)
   {
      pollLink(0);
   }

   /* Check which port is connected and take PORT0 with priority */
   if( ethHw_portLinkStatus(ETHHW_PORT_0) )
   {
      extPort = ETHHW_PORT_0;
   }
   else if( ethHw_portLinkStatus(ETHHW_PORT_1) )
   {
      extPort = ETHHW_PORT_1;
   }
   else
   {
      error(" None of Ethernet external ports are connected \n");
      return -1;
   }
   debug("\r\n=== Speed_Summary_0(0x3820_0820)=0x%x ===\r\n", ETHHW_REG_GET_VAL(ethHw_regStatusPortSpeed));
   extPortSpeed = ethHw_portSpeed(extPort);
   /* Configure internal port speed 
    * always 1Gb (switch->CPU) and follow external port speed (CPU->switch)
    */
#ifdef CONFIG_BCM11140_FPGA
   ethHw_impSpeedSet(100, 100);
#else
   ethHw_impSpeedSet(extPortSpeed, 1000);
#endif

   extPort = ETHHW_PORT_0;
   extPortSpeed = ethHw_portSpeed(extPort);
   debug("\r\n=== ExtPort%d Speed=%d ===\r\n", extPort, extPortSpeed);
   if (phy_id == PHY_BCM54612E)
   {
       if (extPortSpeed == 1000)
       {
          /* Disable the RGMII transmit timing delay on the external PHY */
          ethHw_miiSet( extPort, ETHHW_MII_FLAGS_EXT, 0x1c, 0x0c00 );
          ethHw_miiGet( extPort, ETHHW_MII_FLAGS_EXT, 0x1c, &reg32 );
          reg32 |= 0x8000; /* write enable */
          reg32 &= ~0x0200; /* turn off bit 9 to disable RGMII Tx delay */
          ethHw_miiSet( extPort, ETHHW_MII_FLAGS_EXT, 0x1c, reg32 );
       }
       else
       {
          /* Enable the RGMII transmit timing delay on the external PHY */
          ethHw_miiSet( extPort, ETHHW_MII_FLAGS_EXT, 0x1c, 0x0c00 );
          ethHw_miiGet( extPort, ETHHW_MII_FLAGS_EXT, 0x1c, &reg32 );
          reg32 |= 0x8000; /* write enable */
          reg32 |= 0x0200; /* turn on bit 9 to enable RGMII Tx delay */
          ethHw_miiSet( extPort, ETHHW_MII_FLAGS_EXT, 0x1c, reg32 );
       }
   }

   /* 
    * Even if the hardware supports only 1 phy, both are available 
    * internally so it doesn't hurt to configure both.
    */
   extPort = ETHHW_PORT_1;
   extPortSpeed = ethHw_portSpeed(extPort);
   debug("\r\n=== ExtPort%d Speed=%d ===\r\n", extPort, extPortSpeed);

    if (phy_id == PHY_BCM54612E)
    {
       if (extPortSpeed == 1000)
       {
          /* Disable the RGMII transmit timing delay on the external PHY */
          ethHw_miiSet( extPort, ETHHW_MII_FLAGS_EXT, 0x1c, 0x0c00 );
          ethHw_miiGet( extPort, ETHHW_MII_FLAGS_EXT, 0x1c, &reg32 );
          reg32 |= 0x8000; /* write enable */
          reg32 &= ~0x0200; /* turn off bit 9 to disable RGMII Tx delay */
          ethHw_miiSet( extPort, ETHHW_MII_FLAGS_EXT, 0x1c, reg32 );
       }
       else
       {
          /* Enable the RGMII transmit timing delay on the external PHY */
          ethHw_miiSet( extPort, ETHHW_MII_FLAGS_EXT, 0x1c, 0x0c00 );
          ethHw_miiGet( extPort, ETHHW_MII_FLAGS_EXT, 0x1c, &reg32 );
          reg32 |= 0x8000; /* write enable */
          reg32 |= 0x0200; /* turn on bit 9 to enable RGMII Tx delay */
          ethHw_miiSet( extPort, ETHHW_MII_FLAGS_EXT, 0x1c, reg32 );
       }
    }

   /* Enable forwarding to internal port */
   ethHw_impEnableSet(1);
   ethHw_macEnableSet(ETHHW_PORT_INT, 1, 1);

   int rx, tx;
   ethHw_impSpeedGet(&rx, &tx);
   debug("internal port speed: CPU->port(%d), port->CPU(%d)\n",
      rx, tx);

   /* Initialize RX DMA descriptors */
   for (i = 0; i < RX_BUF_NUM; i++) 
   {
      bufp = (uint8_t *) RX_BUF(i);
      descp = (ETHHW_DMA_DESC *) RX_DESC(i);

      // Rx config descriptor
      // The config descriptor allows transfer to be purely 64-bit
      // transactions, spanning an arbitrary number of descriptors,
      // so information must be provided to define offsets and EOP
      ETHHW_DMA_DESC_CREATE( 
         descp,
         ETHHW_DMA_PTM_FIFO_ADDR,
         bufp, 
         descp,
         ETHHW_DMA_PTM_CTL_LO, 
         RX_BUF_SIZE 
         );

      descp++;
   }
   // Wrap last descriptor back to beginning
   descp--;
   ETHHW_DMA_DESC_WRAP(descp, (uint32_t)RX_DESC(0));

   /* Init DMA for Ethernet */
   ethHw_dmaInit(ETH_DMA_CONTROLLER);
   ethHw_dmaDisable(ETH_DMA_CONTROLLER, ETH_DMA_CH_RX);
   ethHw_dmaDisable(ETH_DMA_CONTROLLER, ETH_DMA_CH_TX);

   /* Configure Rx DMA */
   ethHw_dmaRxConfig(ETH_DMA_BURST_SIZE, ETH_DMA_BLOCK_SIZE);
   ethHw_dmaConfig( ETH_DMA_CONTROLLER, ETH_DMA_CH_RX,
              ETHHW_DMA_PTM_CTL_HI, ETHHW_DMA_PTM_CTL_LO,
              ETHHW_DMA_PTM_CFG_HI, ETHHW_DMA_PTM_CFG_LO,
              ETHHW_DMA_PTM_SSTAT, 0 );

   // Configure Tx DMA
   ethHw_dmaTxConfig(ETH_DMA_BURST_SIZE);
   ethHw_dmaConfig( ETH_DMA_CONTROLLER, ETH_DMA_CH_TX,
              ETHHW_DMA_MTP_CTL_HI, ETHHW_DMA_MTP_CTL_LO,
              ETHHW_DMA_MTP_CFG_HI, ETHHW_DMA_MTP_CFG_LO,
              0, ETHHW_DMA_MTP_DSTAT );

   /* If RX DMA is not enabled, packet will be held in switch for a while, 
    * but eventually it will be dropped so that RX DMA should be enabled all the time
    */
   ethHw_dmaEnable(ETH_DMA_CONTROLLER, ETH_DMA_CH_RX, RX_DESC(0));

   debug( "Enable Ethernet Done \n" );

   return 0;
}

static void bcm11140_eth_close(struct eth_device *dev)
{
   debug("entering bcm11140_eth_close!\n");

   ethHw_dmaDisable(ETH_DMA_CONTROLLER, ETH_DMA_CH_RX);
   ethHw_dmaDisable(ETH_DMA_CONTROLLER, ETH_DMA_CH_TX);

   /* Disable forwarding to internal port */
   ethHw_macEnableSet(ETHHW_PORT_INT, 0, 0);
   ethHw_impEnableSet(0);

   debug("bcmring_net_close!\n");
}

//static int pollLink( char* portp )
static int pollLink( int port )
{
    //eth_port_t port;
    uint32_t override_reg;  /* Override register */
    uint32_t override_settings;
    uint32_t override_mask;
    uint32_t link_speed = 0;    /* in Mbps - valid settings are 0 (no link), 10, 100, 1000 */
    uint32_t full_duplex = 0;   /* 0 = half duplex, 1 = full duplex */
    uint32_t tx_pause = 0;      /* TX Pause resolution */
    uint32_t rx_pause = 0;      /* RX Pause resolution */
    uint32_t phy_data;

    //port = (eth_port_t)strtoul(portp, NULL, 0);

    //if (current_phy->phy_flags & PHY_FLAGS_POLL_LINK)
    //{
        /* Get link settings from phy */
        //if (current_phy->phy_id == PHY_BCM5241)
        //{
            /* Restart autonegotiation */
            printf("BCM5421: Restart Auto-Negotiation\n");
            //ethHw_port_phy_get(port, ETH_PORT_PHY_EXT, 0x00, &phy_data);
            ethHw_miiGet(port, ETHHW_MII_FLAGS_EXT, 0x00, &phy_data);
            phy_data |= 0x1200; // bit 9 - Restart Auto-Neg, bit 12 - Auto-Neg enable
            //ethHw_port_phy_set(port, ETH_PORT_PHY_EXT, 0x00, phy_data);
            ethHw_miiSet(port, ETHHW_MII_FLAGS_EXT, 0x00, phy_data);

            /* Spin until autonegotiation done - bit 5 of MII Status Register (0x01) */
            printf("BCM5241: Waiting for auto-negotiation completion\n");
            do
            {
                //ethHw_port_phy_get(port, ETH_PORT_PHY_EXT, 0x01, &phy_data);
                ethHw_miiGet(port, ETHHW_MII_FLAGS_EXT, 0x01, &phy_data);
            }
            while (!(phy_data & 0x0020)); // Bit goes from 0->1 when complete
            printf("BCM5241: Autonegotiation complete\n");
            mdelay( 200 );
            /* Get latest version of status register */
            //ethHw_port_phy_get(port, ETH_PORT_PHY_EXT, 0x01, &phy_data);
            ethHw_miiGet(port, ETHHW_MII_FLAGS_EXT, 0x01, &phy_data);
            if (phy_data & 0x4)
            {
                /* LInk Up */
                /* Read Aux control/status register to get speed statu s*/
                //ethHw_port_phy_get(port, ETH_PORT_PHY_EXT, 0x18, &phy_data);
                ethHw_miiGet(port, ETHHW_MII_FLAGS_EXT, 0x18, &phy_data);

                /* Bit 0 - full duplex */
                full_duplex = (phy_data & 0x1) ? 1 : 0;
                /* Bit 1 - speed */
                if (phy_data & 0x2)
                {
                    link_speed = 100;
                }
                else
                {
                    link_speed = 10;
                }

                /* Get Pause capability */
                //ethHw_port_phy_get(port, ETH_PORT_PHY_EXT, 0x19, &phy_data);
                ethHw_miiGet(port, ETHHW_MII_FLAGS_EXT, 0x19, &phy_data);

                if (phy_data & 0x0800) // bit 11
                {
                    tx_pause = 1;
                    rx_pause = 1;
                }
            }
            else
            {
                /* Link down */
                printf("BCM5241: Link down.\n");
                link_speed = 0;
            }
        //}
        /*
        else if (current_phy->phy_id == PHY_...)
        {
        }
        */
        //else
        //{
        //    printf("PHY %s does not have a method ot get link status\n");
        //    link_speed = 0;
        //    full_duplex = 0;
        //    tx_pause = 0;
        //    rx_pause = 0;
        //}

        if (link_speed == 0)
        {
            printf("PHY detected no link\n");
            full_duplex = 0;
            tx_pause = 0;
            rx_pause = 0;
        }
        else
        {
            printf("PHY settings: link %d Mbps, %s duplex, tx pause %d, rx pause %d\n", link_speed, (full_duplex) ? "full" : "half", tx_pause, rx_pause);
        }

        /* Now configure */
        if (port == 0)
        { /* port 0 */
            override_mask = 0xFFFFFF00; /* Low byte is port 0 */
            switch (link_speed)
            {
                case 1000:
                    override_settings = 0x0; // 00b = 1000Mbps
                    break;

                case 100:
                    override_settings = 0x1; // 01b = 100Mbps
                    break;

                case 10:
                    override_settings = 0x2; // 10b = 10Mbps
                    break;

                default:
                    printf("Unknown link speed %d\n", link_speed);
                    /* Fall through */
                case 0:
                    override_settings = 0x3; // 11b = link down, reserved
                    break;
            }
            /* Shift bit over as appropriate */
            override_settings <<= ESW_BUSIF_IMP_EXTERNAL_PORT_RGMII_OVERRIDE_PORT_0_EXT_PORT_LNKSPD_OVERRIDE_SHIFT;
            /* Duplex */
            if (full_duplex)
            {
                override_settings |= ESW_BUSIF_IMP_EXTERNAL_PORT_RGMII_OVERRIDE_PORT_0_EN_EXT_PORT_DUPLEX_OVERRIDE_MASK;
            }
            else
            {
                override_settings &= ~(ESW_BUSIF_IMP_EXTERNAL_PORT_RGMII_OVERRIDE_PORT_0_EN_EXT_PORT_DUPLEX_OVERRIDE_MASK);
            }
            /* Pause */
            if (tx_pause)
            {
                override_settings |= ESW_BUSIF_IMP_EXTERNAL_PORT_RGMII_OVERRIDE_PORT_0_EXT_PORT_TX_PASUE_RESOLUTION_MASK;
            }
            else
            {
                override_settings &= ~(ESW_BUSIF_IMP_EXTERNAL_PORT_RGMII_OVERRIDE_PORT_0_EXT_PORT_TX_PASUE_RESOLUTION_MASK);
            }
            if (rx_pause)
            {
                override_settings |= ESW_BUSIF_IMP_EXTERNAL_PORT_RGMII_OVERRIDE_PORT_0_EXT_PORT_RX_PASUE_RESOLUTION_MASK;
            }
            else
            {
                override_settings &= ~(ESW_BUSIF_IMP_EXTERNAL_PORT_RGMII_OVERRIDE_PORT_0_EXT_PORT_RX_PASUE_RESOLUTION_MASK);
            }
            /* Force override */
            override_settings |= ESW_BUSIF_IMP_EXTERNAL_PORT_RGMII_OVERRIDE_PORT_0_EN_EXT_PORT_OVERRIDE_EN_MASK;
        }
        else
        { /* port 1 */
            override_mask = 0xFFFF00FF; /* port 1 */
            switch (link_speed)
            {
                case 1000:
                    override_settings = 0x0; // 00b = 1000Mbps
                    break;

                case 100:
                    override_settings = 0x1; // 01b = 100Mbps
                    break;

                case 10:
                    override_settings = 0x2; // 10b = 10Mbps
                    break;

                default:
                    printf("Unknown link speed %d\n", link_speed);
                    /* Fall through */
                case 0:
                    override_settings = 0x3; // 11b = link down, reserved
                    break;
            }
            /* Shift bit over as appropriate */
            override_settings <<= ESW_BUSIF_IMP_EXTERNAL_PORT_RGMII_OVERRIDE_PORT_1_EXT_PORT_LNKSPD_OVERRIDE_SHIFT;
            /* Duplex */
            if (full_duplex)
            {
                override_settings |= ESW_BUSIF_IMP_EXTERNAL_PORT_RGMII_OVERRIDE_PORT_1_EN_EXT_PORT_DUPLEX_OVERRIDE_MASK;
            }
            else
            {
                override_settings &= ~(ESW_BUSIF_IMP_EXTERNAL_PORT_RGMII_OVERRIDE_PORT_1_EN_EXT_PORT_DUPLEX_OVERRIDE_MASK);
            }
            /* Pause */
            if (tx_pause)
            {
                override_settings |= ESW_BUSIF_IMP_EXTERNAL_PORT_RGMII_OVERRIDE_PORT_1_EXT_PORT_TX_PASUE_RESOLUTION_MASK;
            }
            else
            {
                override_settings &= ~(ESW_BUSIF_IMP_EXTERNAL_PORT_RGMII_OVERRIDE_PORT_1_EXT_PORT_TX_PASUE_RESOLUTION_MASK);
            }
            if (rx_pause)
            {
                override_settings |= ESW_BUSIF_IMP_EXTERNAL_PORT_RGMII_OVERRIDE_PORT_1_EXT_PORT_RX_PASUE_RESOLUTION_MASK;
            }
            else
            {
                override_settings &= ~(ESW_BUSIF_IMP_EXTERNAL_PORT_RGMII_OVERRIDE_PORT_1_EXT_PORT_RX_PASUE_RESOLUTION_MASK);
            }
            /* Force override */
            override_settings |= ESW_BUSIF_IMP_EXTERNAL_PORT_RGMII_OVERRIDE_PORT_1_EN_EXT_PORT_OVERRIDE_EN_MASK;
        }

        //override_reg = ETH_REG32_GET(ETH_BASE_ADDR | 0xA0098); /* esw_busif_imp.external_port_rgmii_override */
        override_reg = readl(ESW_BUSIF_IMP_BASE_ADDR + ESW_BUSIF_IMP_EXTERNAL_PORT_RGMII_OVERRIDE_OFFSET);
        printf("DEBUG: override_reg 0x%08X, override_mask 0x%08X, override_settings 0x%08X\n", override_reg, override_mask, override_settings);
        override_reg &= override_mask;
        override_reg |= override_settings;
        printf("DEBUG: override_reg 0x%08X\n", override_reg);
        //ETH_REG32_SET(ETH_BASE_ADDR | 0xA0098, override_reg);
        writel(override_reg, ESW_BUSIF_IMP_BASE_ADDR + ESW_BUSIF_IMP_EXTERNAL_PORT_RGMII_OVERRIDE_OFFSET);
    //}
    //else
    //{
    //    printf("PHY %s does not require link to be forced\n", current_phy->phy_name);
    //}

    return 0;
}

static void configureMode(int port, int mii_mode)
{
    uint32_t reg32;
    uint32_t padctrl;
    uint32_t mii_mode_sel;

    ///////////////////////////////////////////////////////////////////////////
    // In RGMII, TXC is an output. In MII, TXC is an input from the PHY.
    // Capri A0 had this fixed as output and would generate alignment errors on transmit.
    // A1 does not have problem.
    ///////////////////////////////////////////////////////////////////////////
    if (port == 0)
    {
        padctrl = readl(PAD_CTRL_BASE_ADDR + PADCTRLREG_RGMII_0_TXC_OFFSET);
    }
    else
    {
        padctrl = readl(PAD_CTRL_BASE_ADDR + PADCTRLREG_RGMII_1_TXC_OFFSET);
    }

    if (mii_mode)
    {
        /* Configure for FE MII mode */
        /* port 0 -- Bit 0 must be set (undocumented) */
        /* port 1 -- Bit 1 must be set (undocumented) */
        padctrl |= (0x1 << port);
    }
    else
    {
        /* Configure for RGMII mode */
        /* port 0 -- Bit 0 must be cleared (undocumented) */
        /* port 1 -- Bit 1 must be cleared (undocumented) */
        padctrl &= ~(0x1 << port);
    }

    if (port == 0)
    {
        writel(padctrl, PAD_CTRL_BASE_ADDR + PADCTRLREG_RGMII_0_TXC_OFFSET);
    }
    else
    {
        writel(padctrl, PAD_CTRL_BASE_ADDR + PADCTRLREG_RGMII_1_TXC_OFFSET);
    }
    ///////////////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////////////
    /* esw_busif_imp.esw_rgmii_mii_mode_sel */
    mii_mode_sel = readl(ESW_BUSIF_IMP_BASE_ADDR + ESW_BUSIF_IMP_ESW_RGMII_MII_MODE_SEL_OFFSET);
    if (mii_mode)
    {
        /* Configure pad for MII mode */
        mii_mode_sel |= (0x1 << port); // Port 0 is bit 0, port 1 is bit 1
    }
    else
    {
        /* Configure pad for RGMII mode */
        mii_mode_sel &= ~(0x1 << port); // Port 0 is bit 0, port 1 is bit 1
    }
    writel(mii_mode_sel, ESW_BUSIF_IMP_BASE_ADDR + ESW_BUSIF_IMP_ESW_RGMII_MII_MODE_SEL_OFFSET);
    ///////////////////////////////////////////////////////////////////////////

    if (mii_mode)
    {
    }
    else
    {
#ifndef CONFIG_BCM11140_FPGA
        /* Disable the RGMII transmit timing delay on the external PHY0 */
        ethHw_miiSet( port, ETHHW_MII_FLAGS_EXT, 0x1c, 0x0c00 );
        ethHw_miiGet( port, ETHHW_MII_FLAGS_EXT, 0x1c, &reg32 );
        reg32 |= 0x8000; /* write enable */
        reg32 &= ~0x0200; /* turn off bit 9 to disable RGMII Tx delay */
        ethHw_miiSet( port, ETHHW_MII_FLAGS_EXT, 0x1c, reg32 );
#else
        ethHw_miiGet( port, ETHHW_MII_FLAGS_EXT, 0x9, &reg32 );
        reg32 &= ~0x00000300;   /* Disable 1000-half/full advertisement */
        ethHw_miiSet( port, ETHHW_MII_FLAGS_EXT, 0x9, reg32 );
#endif

        /* Enable out of band signaling on the external PHY0 */
        ethHw_miiSet( port, ETHHW_MII_FLAGS_EXT, 0x18, 0x7007 );
        ethHw_miiGet( port, ETHHW_MII_FLAGS_EXT, 0x18, &reg32 );
        reg32 |= 0x8000; /* write enable */
        reg32 &= ~0x0020; /* turn off bit 5 for outband signaling */
        ethHw_miiSet( port, ETHHW_MII_FLAGS_EXT, 0x18, reg32 );
    }
}

static int configurePhy(int port)
{
    uint32_t phyid_lsw;
    uint32_t phyid_msw;
    int retry = 0;

    /* At this point, MDIO addresses and PHYs are rset, so begin PHY identification
     *
     * Loop in case PHY gets stuck and isn't responding on MDIO properly
     */
    /* Now find the PHY ID */
    while (1)
    {
        ethHw_miiGet( port, ETHHW_MII_FLAGS_EXT, 0x02, &phyid_msw );
        ethHw_miiGet( port, ETHHW_MII_FLAGS_EXT, 0x03, &phyid_lsw );

        if ((phyid_msw == 0xFFFF) || (phyid_lsw == 0xFFFF))
        {
            if (retry ++ > 30)
            {
                printf("Read port %d PHY ID failed\n", port);
                return -1;
            }

            mdelay( 100 );
        }
        else
        {
            printf("port %d PHY ID: 0x%04X, 0x%04X\n", port, phyid_msw, phyid_lsw);
            break;
        }
   }

   phy_id = MAKE_PHY_ID(phyid_msw, phyid_lsw);

   if (phy_id == PHY_BCM54612E)
   {
       /* Set interface to RGMII */
       configureMode(port, 0);
   }
   else if (phy_id == PHY_BCM5241)
   {
       /* Set interface to MII */
       configureMode(port, 1);
       printf("BCM5241 has no required initialziation\n");
   }
   else /* Other  PHY */
   {
       printf("PHY has no initialization\n");
   }

   return 0;
}

#define BYTE_ALIGNMENT 8	/* 8 byte alignment requirement */
#define ALIGN_MASK (BYTE_ALIGNMENT - 1)

int bcm11140_eth_register(u8 dev_num)
{
   struct eth_device *dev;
   int rc;

   dev = (struct eth_device *) malloc(sizeof(struct eth_device));
   if (dev == NULL) {
      return -1;
   }

   printf(banner);

   /* Allocate space for Rx & TX buffers/ */
   if (!(rx_tx_buffer = malloc(SZ_1M+BYTE_ALIGNMENT))) {
       error( "Failed to initialize RC/TX buffers\n" );
       return -1;
   }

   /* 
    * Note that u-boot malloc returns 8-byte aligned buffers by design
    * so there is no need to perform alignment logic to get 64-bit alignment.
    * But let's do it anyways in case the malloc logic ever changes or this
    * code gets copied to some other bootloader or utility.
    */
   aligned_rx_tx_buffer = (unsigned char*)0x87c10000;

   memset(dev, 0, sizeof(*dev));
   sprintf(dev->name, "%s-%hu", BCM11140_ETH_DEV_NAME, dev_num);
   
   /* Initialization */
   debug( "Ethernet initialization ..." );

#ifdef CONFIG_BCM11140_ETH
   uint32_t periph_misc_reg2;

   periph_misc_reg2 = readl(CHIPREGS_BASE_ADDR + CHIPREG_PERIPH_MISC_REG2_OFFSET);
   periph_misc_reg2 &= ~CHIPREG_PERIPH_MISC_REG2_RGMII_SEL_MASK; // set it to 3.3V GMII
   //periph_misc_reg2 |= 0x1; // set it to 2.5V RGMII
   //periph_misc_reg2 |= 0x2; // set it to 1.5V HSTL
   //periph_misc_reg2 |= 0x3; // set it to 1.8V HSTL(non-statndard
   writel(periph_misc_reg2, CHIPREGS_BASE_ADDR + CHIPREG_PERIPH_MISC_REG2_OFFSET);
   debug("\r\n=== periph_misc_reg2(0x3500_4038)=0x%x ===", periph_misc_reg2);
   debug("\r\n1:0	rgmii_sel	RW	Determines the mode of the RGMII pads");
   debug("\r\nDecode is as follows:");
   debug("\r\n00 -> 3.3V GMII");
   debug("\r\n01 -> 2.5V RGMII");
   debug("\r\n10 -> 1.5V HSTL Class I");
   debug("\r\n11 -> 1.8V HSTL Class I (non-standard)\n");

   rc = ethPhyStart();
   if( rc == ETHHW_RC_NONE )
   {
      debug( "PHY initialization successful\n" );
   }
   else
   {
      error( "PHY initialization failed\n" );
      return -1;
   }
#endif

   rc = ethHw_Init();
   debug("Ethernet initialization %s (rc=%i)\n",
          ETHHW_RC_SUCCESS(rc) ? "successful" : "failed", rc);

   /* Disable forwarding to internal port (this must be done before
    * forwarding is enabled on the external ports)
    */
   ethHw_macEnableSet(ETHHW_PORT_INT, 0, 0);
   /* Disable internal port */
   ethHw_impEnableSet(0);

   /* STP not used so put external ports in forwarding state */
   /* TODO:  If STP support is required, this state control will need 
    * to be moved to the STP application
    */
   ethHw_stpStateSet(ETHHW_PORT_0, ETHHW_STP_STATE_FORWARDING);
   ethHw_stpStateSet(ETHHW_PORT_1, ETHHW_STP_STATE_FORWARDING);

   dev->iobase = 0;

   dev->init = bcm11140_eth_open;
   dev->halt = bcm11140_eth_close;
   dev->send = bcm11140_eth_send;
   dev->recv = bcm11140_eth_rcv;
   dev->write_hwaddr = bcm11140_eth_write_hwaddr;

   eth_register(dev);

#ifdef CONFIG_CMD_MII
   miiphy_register(dev->name, bcm11140_miiphy_read, bcm11140_miiphy_write);
#endif

   debug( "Basic ethernet functionality initialized\n" );

#ifdef BCM11140_ETH_DEBUG
   bcm11140_eth_info();
#endif

   return 1;
}
