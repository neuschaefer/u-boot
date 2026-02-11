#include <fastboot.h>

#include <common.h>
#include <asm/io.h>
#include <asm/arch/brcm_rdb_sysmap.h>
#include <asm/arch/brcm_rdb_hsotg_ctrl.h>
#include <asm/arch/brcm_rdb_hsotg.h>
#include <asm/arch/brcm_rdb_root_rst_mgr_reg.h>

#include <usb_defs.h>
#include <usbdevice.h>
#include <usbdescriptors.h>

#include <watchdog.h>

#define wfld_set(addr, fld_val, fld_mask)	(writel(((readl(addr) & ~(fld_mask)) | (fld_val)),(addr)))
#define wfld_clear(addr, fld_mask)		(writel((readl(addr) & ~(fld_mask)),(addr)))

/* FIXME - At some point need to get a proper broadcom product id and use that */
#if 0
  #define DEVICE_VENDOR_ID 	0x0A5C 	/* broadcom corporation */
  #define DEVICE_PRODUCT_ID 	0xBABE
#else
  #define DEVICE_VENDOR_ID 	0x18D1 	/* google */
  #define DEVICE_PRODUCT_ID 	0x0D02  /* nexus one */
#endif

/* String 0 is the language id */
#define DEVICE_STRING_PRODUCT_INDEX       1
#define DEVICE_STRING_SERIAL_NUMBER_INDEX 2
#define DEVICE_STRING_CONFIG_INDEX        3
#define DEVICE_STRING_INTERFACE_INDEX     4
#define DEVICE_STRING_MANUFACTURER_INDEX  5
#define DEVICE_STRING_MAX_INDEX           DEVICE_STRING_MANUFACTURER_INDEX
#define DEVICE_STRING_LANGUAGE_ID         0x0409 /* English (United States) */

/* In high speed mode packets are 512
   In full speed mode packets are 64 */
#define RX_ENDPOINT_MAXIMUM_PACKET_SIZE      (0x0040)
#define TX_ENDPOINT_MAXIMUM_PACKET_SIZE      (0x0200)

static char* device_strings[DEVICE_STRING_MANUFACTURER_INDEX+1] = {
	/* Language = */	"",
	/* Product = */ 	CONFIG_FASTBOOT_BOARDNAME,
	/* Serial = */ 		"1234567890",
	/* Config = */		"Android Fastboot",
	/* Interface = */	"Android Fastboot",
	/* Manufacturer = */	"Broadcom Corporation", };

static struct cmd_fastboot_interface *fastboot_interface = NULL;


/********************************************************************************/
/*             Local Variables                                                  */
/********************************************************************************/

static struct usb_device_descriptor dfu_dev_descriptor __attribute__ ((__aligned__ (4)))= {
	.bLength		= 0x12,
	.bDescriptorType	= USB_DT_DEVICE,
	.bcdUSB			= 0x100,
	.bDeviceClass		= 0xff,
	.bDeviceSubClass	= 0xff,
	.bDeviceProtocol	= 0xff,
	.bMaxPacketSize0	= 64,     /* From enum speed */
	.idVendor		= DEVICE_VENDOR_ID, /* Get these from OTP */
	.idProduct		= DEVICE_PRODUCT_ID,
	.bcdDevice		= 0x0001,
	.iManufacturer		= DEVICE_STRING_MANUFACTURER_INDEX,
	.iProduct		= DEVICE_STRING_PRODUCT_INDEX,
	.iSerialNumber		= DEVICE_STRING_SERIAL_NUMBER_INDEX,
	.bNumConfigurations	= 0x1,
};


struct full_configuration_descriptor {
	struct usb_configuration_descriptor c;
	struct usb_interface_descriptor i;
	struct usb_endpoint_descriptor e1;
	struct usb_endpoint_descriptor e2;
} __attribute__ ((packed));

static struct full_configuration_descriptor full_desc __attribute__ ((__aligned__ (4)))= {
	.c 	= {
			.bLength		= 0x9,
			.bDescriptorType	= USB_DT_CONFIG,
			.wTotalLength		= sizeof(struct usb_configuration_descriptor)
						+ sizeof(struct usb_interface_descriptor)
						+ sizeof(struct usb_endpoint_descriptor)
						+ sizeof(struct usb_endpoint_descriptor),
			.bNumInterfaces		= 1,
			.bConfigurationValue	= 1,
			.iConfiguration		= DEVICE_STRING_CONFIG_INDEX,
			.bmAttributes		= BMATTRIBUTE_RESERVED | BMATTRIBUTE_SELF_POWERED,
			.bMaxPower		= 0, },
	.i 	= {
			.bLength		= 0x9,
			.bDescriptorType	= USB_DT_INTERFACE,
			.bInterfaceNumber	= 0x0,
			.bAlternateSetting	= 0x0,   /* CheckMe */
			.bNumEndpoints		= 0x2,
			.bInterfaceClass	= FASTBOOT_INTERFACE_CLASS,
			.bInterfaceSubClass	= FASTBOOT_INTERFACE_SUB_CLASS,
			.bInterfaceProtocol	= FASTBOOT_INTERFACE_PROTOCOL,
			.iInterface		= DEVICE_STRING_INTERFACE_INDEX, },
	.e1 	= {
			.bLength 		= sizeof(struct usb_endpoint_descriptor),
			.bDescriptorType 	= USB_DT_ENDPOINT,
			.bEndpointAddress	= USB_DIR_IN | 1,
			.bmAttributes		= USB_ENDPOINT_XFER_BULK,
			.wMaxPacketSize		= TX_ENDPOINT_MAXIMUM_PACKET_SIZE,
			.bInterval		= 0x00, },
	.e2	= {
			.bLength 		= sizeof(struct usb_endpoint_descriptor),
			.bDescriptorType 	= USB_DT_ENDPOINT,
			.bEndpointAddress	= USB_DIR_OUT | 1,
			.bmAttributes		= USB_ENDPOINT_XFER_BULK,
			.wMaxPacketSize		= RX_ENDPOINT_MAXIMUM_PACKET_SIZE,
			.bInterval		= 0x00,
		},
};


/********************************************************************************/
/*             Local Definitions                                                */
/********************************************************************************/
#define EP0_SETUP_BUF_SIZE      4
#define EP0_IN_BUF_SIZE         32
#define EP0_OUT_BUF_SIZE        4

#ifdef CONFIG_SAMOA
#define RX_FIFO_SIZE            512     /* 512 x 4 bytes */
#define TX_FIFO_SIZE            32      /*  32 x 4 bytes */
#define TX_FIFO_OFFSET          RX_FIFO_SIZE
#else
#define RX_FIFO_SIZE            512     /* 512 x 4 bytes */
#define TX_FIFO_SIZE            256     /* 256 x 4 bytes */
#define TX_FIFO_OFFSET          RX_FIFO_SIZE
#endif

#define TURN_AROUND_TIME        9

#define EP_MISMATCH_CNT         1
#define PERIODIC_FRM_INTERVAL   1
#define BC11_CFG_VDP_OFF        0x55570000

#define EP1_IN_BUF_SIZE        	512
#define EP1_OUT_BUF_SIZE        512

/********************************************************************************/
/*             Private Data                                                     */
/********************************************************************************/
static uint32_t usb_speed;
static uint32_t ep0_setup_buf[EP0_SETUP_BUF_SIZE] __attribute__ ((__aligned__ (4)));
static uint32_t ep0_out_buf[EP0_OUT_BUF_SIZE] __attribute__ ((__aligned__ (4)));
static uint32_t ep0_in_buf[EP0_IN_BUF_SIZE] __attribute__ ((__aligned__ (4)));

static uint8_t ep1_out_buf[2][EP1_OUT_BUF_SIZE] __attribute__ ((__aligned__ (4)));
static uint8_t ep1_in_buf[EP1_IN_BUF_SIZE] __attribute__ ((__aligned__ (4)));

static int ep1_out_buf_sel = 0;



/*******************************************************/

/**
 * @brief: usb_wait_for_vbus - wait for vbus turning 5.0v by polling STAT2
 * @param[in]:  none
 * @param[out]: none
 * @return: none 
 */
//static inline void usb_wait_for_vbus(void)
static void usb_wait_for_vbus(void)
{
	u32 val;
#ifdef CONFIG_FASTBOOT_NO_PMU
	/* 
	 * If there is no PMU, then the VBUS signal from the connector will
	 * not necessarily be connected to STAT2. We can get around this by
	 * telling the usb core to proceed irregardless by triggering the 
	 * core with internal STAT1/STAT2 signals in software
	 */
	val = readl(HSOTG_CTRL_BASE_ADDR+HSOTG_CTRL_USBOTGCONTROL_OFFSET);
	val |=   (HSOTG_CTRL_USBOTGCONTROL_REG_OTGSTAT2_MASK 
		| HSOTG_CTRL_USBOTGCONTROL_REG_OTGSTAT1_MASK 
		| HSOTG_CTRL_USBOTGCONTROL_OTGSTAT_CTRL_MASK);
	writel(val, HSOTG_CTRL_BASE_ADDR+HSOTG_CTRL_USBOTGCONTROL_OFFSET);
#else
	/* Polling OTGSTAT2 bit, device shall not do anything before vbus is powered by host */
	/*
	usb 2.0 spec
	7.1.5.1
	...
	The voltage source on the pull-up resistor must be derived from or controlled by the power supplied on the USB
	cable such that when VBUS is removed, the pull-up resistor does not supply current on the data line to which it is
	attached.
	*/
	do {
		val = readl(HSOTG_CTRL_BASE_ADDR+HSOTG_CTRL_USBOTGCONTROL_OFFSET);
	} while (! (val & HSOTG_CTRL_USBOTGCONTROL_OTGSTAT2_MASK) );
#endif
}

#ifdef CONFIG_CAPRI
#define HSOTG_CTRL_STATUS_OFFSET HSOTG_CTRL_BC_STATUS_OFFSET
#define HSOTG_CTRL_STATUS_SHP_MASK HSOTG_CTRL_BC_STATUS_SDP_MASK
#define HSOTG_CTRL_CFG_OFFSET HSOTG_CTRL_BC_CFG_OFFSET
#define HSOTG_CTRL_CFG_OVWR_KEY_MASK HSOTG_CTRL_BC_CFG_BC_OVWR_KEY_MASK
#define HSOTG_CTRL_CFG_SW_OVWR_EN_MASK HSOTG_CTRL_BC_CFG_SW_OVWR_EN_MASK
#define HSOTG_CTRL_CFG_OVWR_SET_M0_MASK HSOTG_CTRL_BC_CFG_BC_OVWR_SET_M0_MASK
#define HSOTG_CTRL_CFG_OVWR_SET_P0_MASK HSOTG_CTRL_BC_CFG_BC_OVWR_SET_P0_MASK
#else
#define HSOTG_CTRL_STATUS_OFFSET HSOTG_CTRL_BC11_STATUS_OFFSET
#define HSOTG_CTRL_STATUS_SHP_MASK HSOTG_CTRL_BC11_STATUS_SHP_MASK
#define HSOTG_CTRL_CFG_OFFSET HSOTG_CTRL_BC11_CFG_OFFSET
#define HSOTG_CTRL_CFG_OVWR_KEY_MASK HSOTG_CTRL_BC11_CFG_BC11_OVWR_KEY_MASK
#define HSOTG_CTRL_CFG_SW_OVWR_EN_MASK HSOTG_CTRL_BC11_CFG_SW_OVWR_EN_MASK
#define HSOTG_CTRL_CFG_OVWR_SET_M0_MASK HSOTG_CTRL_BC11_CFG_BC11_OVWR_SET_M0_MASK
#define HSOTG_CTRL_CFG_OVWR_SET_P0_MASK HSOTG_CTRL_BC11_CFG_BC11_OVWR_SET_P0_MASK
#endif

/**
 * @brief: usb_turn_off_vdp - disable vdp
 * @param[in]:  none
 * @param[out]: none
 * @return: none 
 */
//static inline void usb_turn_off_vdp(void)
static void usb_turn_off_vdp(void)
{
	// Check if it is standard host port (SHP).
	if ( readl(HSOTG_CTRL_BASE_ADDR+HSOTG_CTRL_STATUS_OFFSET) & HSOTG_CTRL_STATUS_SHP_MASK) {
		udelay(60000);     // 50 ms + 20 %

		/* force turn off VDP, enable sw_ovwr_set to take over the bc11 switches directly */
		/* 0x55570000 */
		wfld_set(HSOTG_CTRL_BASE_ADDR+HSOTG_CTRL_CFG_OFFSET,
			BC11_CFG_VDP_OFF,
			HSOTG_CTRL_CFG_OVWR_KEY_MASK|
			HSOTG_CTRL_CFG_SW_OVWR_EN_MASK|
			HSOTG_CTRL_CFG_OVWR_SET_M0_MASK|
			HSOTG_CTRL_CFG_OVWR_SET_P0_MASK);

		udelay(160);    // Allow time for switches to disengage.
	}
	else {
		udelay(120000);    // 100 ms + 20 %
	}
}

/**
 * @brief: usb_soft_reset - Soft Reset USB Core
 * @param[in]:  none
 * @param[out]: none
 * @return: OK or ERROR ID
 */
static int usb_soft_reset(void)
{
	uint32_t val;

	/* Add hclk soft reset after interface setting */
	writel( HSOTG_RSTCTL_CSFTRST_MASK, HSOTG_BASE_ADDR + HSOTG_RSTCTL_OFFSET);
	//printf("USB HSOTG Core Soft Reset\n");

	udelay(1000);

	/* Poll until Reset complete and AHB idle */
	do {
		val = readl(HSOTG_BASE_ADDR + HSOTG_RSTCTL_OFFSET);
	} while ( (val&HSOTG_RSTCTL_CSFTRST_MASK) || 
	(!(val&HSOTG_RSTCTL_AHBIDLE_MASK)) );

	return 0;
}

/**
 * @brief: setup_device_fifo - Configure USB FIFO
 * @param[in]:  none
 * @param[out]: none
 * @return: OK or ERROR ID
 */
static void setup_device_fifo(void)
{
	/*Receive FIFO size register    ; 512*4bytes, number is in terms of 32bits */
	writel((RX_FIFO_SIZE << HSOTG_RXFSIZ_RXFDEP_SHIFT), HSOTG_BASE_ADDR + HSOTG_RXFSIZ_OFFSET);

	/*Receive FIFO size register
	256*4bytes --TxFIFO depth
	512*4bytes -- TX RAM start address
	, number is in terms of 32bits */

	writel( (TX_FIFO_SIZE << HSOTG_NPTXFSIZ_NPTXFDEP_SHIFT)|
		(TX_FIFO_OFFSET << HSOTG_NPTXFSIZ_NPTXFSTADDR_SHIFT ),
		HSOTG_BASE_ADDR + HSOTG_NPTXFSIZ_OFFSET);
}


/**
 * @brief: dfu_setup_device_mode - Configure USB device mode
 * @param[in]:  none
 * @param[out]: none
 * @return: OK or ERROR ID
 */
//static inline int dfu_setup_device_mode(void)
static int dfu_setup_device_mode(void)
{
	setup_device_fifo();

	writel( HSOTG_AHBCFG_NPTXFEMPLVL_MASK|		// int indicates TXFIFO empty
		HSOTG_AHBCFG_DMAEN_MASK|                // DMA mode
		HSOTG_AHBCFG_GLBLINTRMSK_MASK,		// Unmask the interrupt assertion to the application
		HSOTG_BASE_ADDR + HSOTG_AHBCFG_OFFSET);

	writel( TURN_AROUND_TIME << HSOTG_CFG_USBTRDTIM_SHIFT, // turn around time
		HSOTG_BASE_ADDR + HSOTG_CFG_OFFSET);

	/* UTMI+ 8bit interface */
	wfld_clear( HSOTG_BASE_ADDR + HSOTG_CFG_OFFSET, HSOTG_CFG_ULPI_UTMI_SEL_MASK);

	writel( (EP_MISMATCH_CNT << HSOTG_DCFG_EPMISCNT_SHIFT)|		// IN Endpoint Mismatch Count
		(PERIODIC_FRM_INTERVAL << HSOTG_DCFG_PERFRINT_SHIFT),	// Periodic Frame Interval
		HSOTG_BASE_ADDR + HSOTG_DCFG_OFFSET);

	/* check if OTG is in device mode */
	if (readl(HSOTG_BASE_ADDR + HSOTG_INTSTS_OFFSET) & HSOTG_INTSTS_CURMOD_MASK) {
		//printf("Fail: not in device mode\n\r");
		return 1;
	}

	return 0;
}

/**
 * @brief: usb_phy_disconnect - disonnect USB Phy from Host
 * @param[in]:  none
 * @param[out]: none
 * @return: OK or ERROR ID
 */
//static inline int usb_phy_disconnect(void)
static int usb_phy_disconnect(void)
{
	/* Soft Disconnect */
	wfld_set(HSOTG_BASE_ADDR+HSOTG_DCTL_OFFSET,
		HSOTG_DCTL_SFTDISCON_MASK,
		HSOTG_DCTL_SFTDISCON_MASK);

	/* set Phy to non-driving (reset) mode */
	wfld_set(HSOTG_CTRL_BASE_ADDR+HSOTG_CTRL_PHY_P1CTL_OFFSET,
	HSOTG_CTRL_PHY_P1CTL_NON_DRIVING_MASK,
	HSOTG_CTRL_PHY_P1CTL_NON_DRIVING_MASK);
	return 0;
}


/**
 * @brief: usb_ep0_setup_prime - Prepare receiving EP0 Setup packet.
 * @param[in]:  none
 * @param[out]: none
 * @return: OK or ERROR ID
 */
int usb_ep0_setup_prime(void)
{
	u32 mask;
	/* Device OUT Endpoint 0 DMA Address Register */
	writel((uint32_t)ep0_setup_buf, HSOTG_BASE_ADDR + HSOTG_DOEPDMA0_OFFSET); // write buf to dev OUT ENP0 DMA addr register

	/* Device OUT Endpoint 0 Transfer Size Register */
	writel(	(0x3<<HSOTG_DOEPTSIZ0_SUPCNT_SHIFT) |	// 3 SUPCnt
		(0x3<<HSOTG_DOEPTSIZ0_PKTCNT_SHIFT) |	// 3 PktCnt
		(0x8<<HSOTG_DOEPTSIZ0_XFERSIZE_SHIFT),	// XferSize 8 bytes
		HSOTG_BASE_ADDR + HSOTG_DOEPTSIZ0_OFFSET);


	/* Device Control OUT Endpoint 0 Control Register */
	mask = HSOTG_DOEPCTL0_EPENA_MASK|               // Endpoint Enable
		HSOTG_DOEPCTL0_CNAK_MASK;                   // clear NAK bit

	wfld_set (HSOTG_BASE_ADDR + HSOTG_DOEPCTL0_OFFSET, mask, mask);

	return 0;
}


int usb_ep1_out_prime(void)
{
	/* toggle which buffer is used */
	ep1_out_buf_sel = ep1_out_buf_sel ? 0 : 1;

	/* Device OUT Endpoint 1 DMA Address Register */
	writel((uint32_t)ep1_out_buf[ep1_out_buf_sel], HSOTG_BASE_ADDR + HSOTG_DOEPDMA1_OFFSET); // write buf to dev OUT ENP0 DMA addr register

	/* Device OUT Endpoint 1 Transfer Size Register */
	writel(	(0x1<<HSOTG_DOEPTSIZ1_PKTCNT_SHIFT) |	// 1 PktCnt
		(512<<HSOTG_DOEPTSIZ1_XFERSIZE_SHIFT),	// XferSize 512 bytes
		HSOTG_BASE_ADDR + HSOTG_DOEPTSIZ1_OFFSET);


	/* Device OUT Endpoint 1 Control Register */
	writel(	(1<<HSOTG_DOEPCTL1_EPENA_SHIFT) |	// Endpoint Enable
		(2<<HSOTG_DOEPCTL1_EPTYPE_SHIFT) |	// Bulk Endpoint
		(1<<HSOTG_DOEPCTL1_CNAK_SHIFT) |	// clear NAK bit
		(1<<HSOTG_DOEPCTL1_USBACTEP_SHIFT) |	// active
		(TX_ENDPOINT_MAXIMUM_PACKET_SIZE<<HSOTG_DOEPCTL1_MPS_SHIFT),	// maximum packet size
		HSOTG_BASE_ADDR + HSOTG_DOEPCTL1_OFFSET);

	writel(0x00030003, HSOTG_BASE_ADDR + HSOTG_DAINTMSK_OFFSET);

	return 0;
}



/**
 * @brief: usb_clr_interrupt - Clear USB interrupt.
 * @param[in]:  none
 * @param[out]: none
 * @return: OK or ERROR ID
 */
static int usb_clr_interrupt(void)
{
	// clear all interrupt
	writel(0xffffffff, HSOTG_BASE_ADDR + HSOTG_INTSTS_OFFSET);
	return 0;
}

/**
 * @brief: usb_phy_connect - Connect USB Phy to host
 * @param[in]:  none
 * @param[out]: none
 * @return: OK or ERROR ID
 */
//static inline int usb_phy_connect(void)
static int usb_phy_connect(void)
{
#ifdef CONFIG_CAPRI
/*
Note that CAPRI defines OEB_IS_TXEB as follows:
#define    HSOTG_CTRL_PHY_P1CTL_USB11_OEB_IS_TXEB_SHIFT                   15
#define    HSOTG_CTRL_PHY_P1CTL_USB11_OEB_IS_TXEB_MASK                    0x00008000

and ISLAND defines PLL_SUSPEND_ENABLE for the same bits... FIXME
#define    HSOTG_CTRL_PHY_P1CTL_PLL_SUSPEND_ENABLE_SHIFT                  15
#define    HSOTG_CTRL_PHY_P1CTL_PLL_SUSPEND_ENABLE_MASK                   0x00008000

So we need to fix this for Capri...

*/
#else
	/* clear bit 15 RDB error */
	wfld_clear(HSOTG_CTRL_BASE_ADDR+HSOTG_CTRL_PHY_P1CTL_OFFSET,
		HSOTG_CTRL_PHY_P1CTL_PLL_SUSPEND_ENABLE_MASK);
#endif

	/* set Phy to driving mode */
	wfld_clear(HSOTG_CTRL_BASE_ADDR+HSOTG_CTRL_PHY_P1CTL_OFFSET,
		HSOTG_CTRL_PHY_P1CTL_NON_DRIVING_MASK);

	udelay(100);
    
	/* Clear Soft Disconnect */
	wfld_clear(HSOTG_BASE_ADDR+HSOTG_DCTL_OFFSET,
		HSOTG_DCTL_SFTDISCON_MASK);

	/* S/W reset Phy, actively low */
	wfld_clear(HSOTG_CTRL_BASE_ADDR+HSOTG_CTRL_PHY_P1CTL_OFFSET,
		HSOTG_CTRL_PHY_P1CTL_SOFT_RESET_MASK);

	udelay(10000);
    
	wfld_set(HSOTG_CTRL_BASE_ADDR+HSOTG_CTRL_PHY_P1CTL_OFFSET,
		HSOTG_CTRL_PHY_P1CTL_SOFT_RESET_MASK,
		HSOTG_CTRL_PHY_P1CTL_SOFT_RESET_MASK);
    
	return 0;
}


/**
 * @brief: usb_handle_ep0_in_int - Process USB EP0-IN interrupt.
 * @param[in]:  none
 * @param[out]: none
 * @return: OK or ERROR ID
 */
int usb_handle_ep0_in_int(void)
{
	u32 diep0int;

	/* clear Device IN Endpoint 0 Interrupt Register */
	diep0int = readl(HSOTG_BASE_ADDR + HSOTG_DIEPINT0_OFFSET);
	writel(diep0int, HSOTG_BASE_ADDR + HSOTG_DIEPINT0_OFFSET);

	return 0;
}

int usb_handle_ep1_in_int(void)
{
	u32 diepint1 = readl(HSOTG_BASE_ADDR + HSOTG_DIEPINT1_OFFSET);

	//printf("diepint1 = 0x%08X\n", diepint1);
	writel(diepint1, HSOTG_BASE_ADDR + HSOTG_DIEPINT1_OFFSET);

	return 0;
}

/**
 * @brief: usb_set_addr - Perform USB Set-Address request.
 * @param[in]:  uint32_t    dev_addr - Address received from PC host
 * @param[out]: none
 * @return: OK or ERROR ID
 */
int usb_set_addr(u32 dev_addr)
{
	/* Device Configuration Register */
	writel((EP_MISMATCH_CNT<<HSOTG_DCFG_EPMISCNT_SHIFT) |		// IN Endpoint Mismatch Count
		(PERIODIC_FRM_INTERVAL<<HSOTG_DCFG_PERFRINT_SHIFT) |	// Periodic Frame Interval
		(dev_addr<<HSOTG_DCFG_DEVADDR_SHIFT),			// Device Address
		HSOTG_BASE_ADDR + HSOTG_DCFG_OFFSET);

	return 0;
}


/**
 * @brief: usb_ep0_send_data - Send packet on EP0-IN endpoint
 * @param[in]:  uint8_t     *data_buf - Data packet to send
 * @param[in]:  uint32_t    cnt - Number of bytes to send
 * @param[out]: none
 * @return: OK or ERROR ID
 */
int usb_ep0_send_data(uint8_t *data_buf, uint32_t cnt)
{
	if (cnt>EP0_IN_BUF_SIZE*4) {
		//printf("EP0_IN overflow\n\r");
		return 1;
	}
	else if(cnt>0) {
		/* use local DMA buffer */
		memcpy(ep0_in_buf, data_buf, cnt);
	}

	/* reply EP0 IN buffer, byte cnt */
	writel((uint32_t)ep0_in_buf,				// write buf to dev IN ENP0 DMA addr register
		HSOTG_BASE_ADDR + HSOTG_DIEPDMA0_OFFSET);
	writel((1<<HSOTG_DIEPTSIZ0_PKTCNT_SHIFT)|		// 1 pakcnt
		(cnt<<HSOTG_DIEPTSIZ0_XFERSIZE_SHIFT),		// XferSize cnt
		HSOTG_BASE_ADDR + HSOTG_DIEPTSIZ0_OFFSET);


	/* Device Control IN Endpoint 0 Control Register  */
	writel( HSOTG_DIEPCTL0_EPENA_MASK|		// Enable ENP. For OUT ENP, indicate data is ready to xfer on it. i
							// For IN ENP, application prepare memory ready to receive data.
		HSOTG_DIEPCTL0_CNAK_MASK|		// clear NAK BIT.
		HSOTG_DIEPCTL0_USBACTEP_MASK|		// USB ACTIVE ENP. always set to 1, indicate ENP0 is always active in all configurations and interfaces
		(0<<HSOTG_DIEPCTL0_NEXTEP_SHIFT),	// next ENP. Applies to NP IN ENP only
		HSOTG_BASE_ADDR + HSOTG_DIEPCTL0_OFFSET);

	return 0;
}

int usb_ep1_in_prime(void)
{
	/* Device Control IN Endpoint 1 Control Register  */
	writel( (2<<HSOTG_DIEPCTL1_EPTYPE_SHIFT)|	// Bulk Endpoint
		(1<<HSOTG_DIEPCTL1_CNAK_SHIFT)|		// clear NAK BIT.
		(1<<HSOTG_DIEPCTL1_USBACTEP_SHIFT)|	// USB ACTIVE ENP.
		(1<<HSOTG_DIEPCTL1_NEXTEP_SHIFT)|	// next ENP. Applies to NP IN EP only
		(TX_ENDPOINT_MAXIMUM_PACKET_SIZE<<HSOTG_DIEPCTL1_MPS_SHIFT),
		HSOTG_BASE_ADDR + HSOTG_DIEPCTL1_OFFSET);

	return 0;
}

int usb_send_data_ep1_in(uint8_t *data_buf, uint32_t cnt)
{

	if (cnt>EP1_IN_BUF_SIZE) {
		//printf("EP1_IN overflow\n\r");
		return 1;
	}
	else if(cnt>0) {
		/* use local DMA buffer */
		memcpy(ep1_in_buf, data_buf, cnt);
	}

	/* reply EP1 IN buffer, byte cnt */
	writel((uint32_t)ep1_in_buf,				// write buf to dev IN ENP1 DMA addr register
		HSOTG_BASE_ADDR + HSOTG_DIEPDMA1_OFFSET);

	writel(	(1<<HSOTG_DIEPTSIZ1_PKTCNT_SHIFT)|		// 1 pakcnt
		(cnt<<HSOTG_DIEPTSIZ1_XFERSIZE_SHIFT),		// XferSize cnt
		HSOTG_BASE_ADDR + HSOTG_DIEPTSIZ1_OFFSET);


	/* Device Control IN Endpoint 0 Control Register  */
	writel( (1<<HSOTG_DIEPCTL1_EPENA_SHIFT)|	// Enable ENP
		(2<<HSOTG_DIEPCTL1_EPTYPE_SHIFT)|	// Bulk Endpoing
		(1<<HSOTG_DIEPCTL1_CNAK_SHIFT)|		// clear NAK BIT.
		(1<<HSOTG_DIEPCTL1_USBACTEP_SHIFT)|	// USB ACTIVE ENP. always set to 1, indicate ENP0 is always active in all configurations and interfaces
		(1<<HSOTG_DIEPCTL1_NEXTEP_SHIFT)|	// next ENP. Applies to NP IN EP only
		(TX_ENDPOINT_MAXIMUM_PACKET_SIZE<<HSOTG_DIEPCTL1_MPS_SHIFT),
		HSOTG_BASE_ADDR + HSOTG_DIEPCTL1_OFFSET);

	return 0;
}

/**
 * @brief: usb_ep0_recv_data - Receive packet on EP0-OUT endpoint
 * @param[in]:  uint8_t     *data_buf - Data packet buffer
 * @param[in]:  uint32_t    cnt - Number of bytes to receive
 * @param[out]: none
 * @return: OK or ERROR ID
 */
int usb_ep0_recv_data(uint8_t *data_buf, uint32_t cnt)
{
	u32 mask;

	if (cnt>EP0_OUT_BUF_SIZE*4) {
		//printf("EP0_OUT overflow\n\r");
		return 1;
	}

	/* Device OUT Endpoint 0 DMA Address Register */
	writel((u32)ep0_out_buf, HSOTG_BASE_ADDR + HSOTG_DOEPDMA0_OFFSET);

	/* Device OUT Endpoint 0 Transfer Size Register */
	writel((1<<HSOTG_DOEPTSIZ0_PKTCNT_SHIFT) | cnt,	// cnt bytes, 1 packet, 1 OUT
		HSOTG_BASE_ADDR + HSOTG_DOEPTSIZ0_OFFSET);

	/* Device Control OUT Endpoint 0 Control Register */
	mask = HSOTG_DOEPCTL0_EPENA_MASK |	// Endpoint Enable
		HSOTG_DOEPCTL0_CNAK_MASK;	// Clear NAK
	wfld_set (HSOTG_BASE_ADDR + HSOTG_DOEPCTL0_OFFSET, mask, mask);

	/* wait for interrupt */
	if(cnt>0)
		memcpy(data_buf, ep0_out_buf, cnt);

	return 0;
}


/**
 * @brief: usb_ep0_send_zlp - Send zero length packet on EP0-IN endpoint
 * @param[in]:  none
 * @param[out]: none
 * @return: OK or ERROR ID
 */
int usb_ep0_send_zlp(void)
{
    usb_ep0_send_data(NULL, 0);
    return 0;
}

/**
 * @brief: usb_ep0_recv_zlp - Receive zero length packet on EP0-OUT endpoint.
 * @param[in]:  none
 * @param[out]: none
 * @return: OK or ERROR ID
 */
int usb_ep0_recv_zlp(void)
{
    usb_ep0_recv_data(NULL, 0);
    return 0;
}

/**
 * @brief: usb_ep0_send_stall - Send STALL condition on EP0.
 * @param[in]:  none
 * @param[out]: none
 * @return: OK or ERROR ID
 */
int usb_ep0_send_stall(void)
{
	//printf("STALL\n\r");

	/* Device Control IN Endpoint 0 Control Register */
	wfld_set(HSOTG_BASE_ADDR + HSOTG_DIEPCTL0_OFFSET,
		HSOTG_DIEPCTL0_STALL_MASK, HSOTG_DIEPCTL0_STALL_MASK);       // STALL

	usb_ep0_setup_prime();
	return 0;
}

/**
 * @brief: dfu_ep0_handler - Decode EP0 setup commands and process CH9 requests.
 * @param[in]:  uint8_t  *setup_buf - EP0 packet buffer.
 * @param[out]: none
 * @return: none
 */
void dfu_ep0_handler(u8 *setup_buf)
{
	uint8_t bmRequestType, bRequest;
	uint16_t wLength, desc_type, desc_index;
	struct usb_device_request ctrl;

	memcpy(&ctrl, setup_buf, 8);

	wLength = ctrl.wLength;
	bmRequestType = ctrl.bmRequestType;
	bRequest = ctrl.bRequest;

	/* Handle class command ourselves */
	if(ctrl.bmRequestType & USB_TYPE_CLASS) {
		//printf("USB_TYPE_CLASS\n");
		usb_ep0_send_stall();
	}
	/* CH9 command */
	else {
		switch (ctrl.bRequest) {
		case USB_REQ_SET_ADDRESS:
			//printf("SET_ADDR = %d\n", ctrl.wValue);
			usb_set_addr(ctrl.wValue);
			usb_ep0_setup_prime();
			usb_ep0_send_zlp();
		return;

		case USB_REQ_CLEAR_FEATURE:
		case USB_REQ_SET_FEATURE:
			//printf("USB_SET/CLR_FEATURE\n");

			if (ctrl.wValue == USB_TEST_MODE) {
				unsigned int testctl = ctrl.wIndex >> 8;

				usb_ep0_setup_prime();
				usb_ep0_send_zlp();

				/* Wait till after status phase to set test mode */
				wfld_set(HSOTG_BASE_ADDR+HSOTG_DCTL_OFFSET,
					testctl,
					HSOTG_DCTL_TSTCTL_MASK);
			} else {
				usb_ep0_setup_prime();
				usb_ep0_send_zlp();
			}

			return;

		case USB_REQ_GET_CONFIGURATION:
			//printf("GET_CONFIG\n");
			usb_ep0_setup_prime();
			usb_ep0_send_zlp();
			break;

		case USB_REQ_SET_CONFIGURATION:
			//printf("SET_CONFIG\n");
			usb_ep0_setup_prime();
			usb_ep1_out_prime();
			usb_ep1_in_prime();
			usb_ep0_send_zlp();
			break;

		case USB_REQ_GET_DESCRIPTOR:
			//printf("GET_DESC -> ");
			desc_type = ctrl.wValue >> 8;
			desc_index = ctrl.wValue & 0xff;
			switch (desc_type) {
			case USB_DT_DEVICE:
				/* Reply Device Descriptor */
				//printf("DEVICE DESC \n\r");
				usb_ep0_send_data((uint8_t *)&dfu_dev_descriptor,
					min(sizeof(dfu_dev_descriptor), wLength));
				usb_ep0_recv_zlp();
				break;
			case USB_DT_CONFIG:
				/* Reply Configuration Descriptor */
				//printf("CONFIG DESC \n\r");
				usb_ep0_send_data((uint8_t *)&full_desc,
					min(sizeof(full_desc), wLength));
				usb_ep0_recv_zlp();
				break;
			case USB_DT_STRING:
				//printf("STRING DESC \n\r");
				if (desc_index == 0) { /* LANGUAGE */
					u8 temp[100];
					temp[0] = 4;
					temp[1] = USB_DT_STRING;
					temp[2] = DEVICE_STRING_LANGUAGE_ID & 0xFF;
					temp[3] = DEVICE_STRING_LANGUAGE_ID >> 8;

					usb_ep0_send_data(temp,4);
					usb_ep0_recv_zlp();
				}
				else if (desc_index <= DEVICE_STRING_MAX_INDEX) {
					int index;
					int sl = strlen( &device_strings[desc_index][0]);
					u8 temp[100];
					temp[0] = 2 + (2 * sl);
					temp[1] = USB_DT_STRING;

					for (index = 0; index < sl; index++) {
						temp[2 + (2*index) + 0] = device_strings[desc_index][index];
						temp[2 + (2*index) + 1] = 0;
					}

					usb_ep0_send_data(temp,2 + (2*sl));
					usb_ep0_recv_zlp();
				}
				else {
					//printf("bad string index\n");
					usb_ep0_send_stall();
				}
				break;
			default:
				//printf("bad descriptor request\n");
				usb_ep0_send_stall();
				break;
			}
		break;

		case USB_REQ_SET_INTERFACE:
		case USB_REQ_GET_INTERFACE:
			//printf("USB_SET/GET_INTERFACE\n\r");
			usb_ep0_setup_prime();
			usb_ep0_send_zlp();
			break;

		case USB_REQ_GET_STATUS:
		case USB_REQ_SET_DESCRIPTOR:
		case USB_REQ_SYNCH_FRAME:
			//printf("USB MISC CH9, ignore\n\r");
			usb_ep0_setup_prime();
			usb_ep0_send_zlp();
			break;

		default:
			//printf("Unknown EP0 cmd\n\r");
			usb_ep0_send_stall();
		break;
		}
	}  /* if-else */
}


/**
 * @brief: usb_handle_ep0_out_int - Process USB EP0-OUT interrupt.
 * @param[in]:  none
 * @param[out]: none
 * @return: OK or ERROR ID
 */
static int usb_handle_ep0_out_int(void)
{
	u32 doep0int;

	/* clear Device OUT Endpoint 0 Interrupt Register */
	doep0int = readl(HSOTG_BASE_ADDR + HSOTG_DOEPINT0_OFFSET);
	writel(doep0int, HSOTG_BASE_ADDR + HSOTG_DOEPINT0_OFFSET);

 	/* Timeout Condition */
	if(doep0int & HSOTG_DOEPINT0_TIMEOUT_MASK) {
		dfu_ep0_handler((uint8_t *)ep0_setup_buf);
	}
	/* Transfer Completed Interrupt */
	else if (doep0int & HSOTG_DOEPINT0_XFERCOMPL_MASK) {
		/* Re-Arm EP0-Setup */
		usb_ep0_setup_prime();
	}
	else
	{
		//printf("doep0int = 0x%08X ... not timeout or transfer complete?\n", doep0int);
		return 1;

	}
	return 0;
}


static int usb_handle_ep1_out_int(void)
{
	u32 doepint1;

	/* clear Device OUT Endpoint 1 Interrupt Register */
	doepint1 = readl(HSOTG_BASE_ADDR + HSOTG_DOEPINT1_OFFSET);
	writel(doepint1, HSOTG_BASE_ADDR + HSOTG_DOEPINT1_OFFSET);

	//printf("doepint1 = 0x%08X\n", doepint1);

 	/* Timeout Condition */
	if(doepint1 & HSOTG_DOEPINT1_TIMEOUT_MASK) {
		//printf("timeout\n");
	}
	/* Transfer Completed Interrupt */
	else if (doepint1 & HSOTG_DOEPINT1_XFERCOMPL_MASK) {

		/* Get pointer to current buffer */
		uint8_t* buffer = ep1_out_buf[ep1_out_buf_sel];

		/* Compute size of transfer */
		int transfer_size = 512 - (readl(HSOTG_BASE_ADDR + HSOTG_DOEPTSIZ1_OFFSET) & HSOTG_DOEPTSIZ1_XFERSIZE_MASK);

		/* Stuff a NULL in to help terminate command strings (may be as long as 64 bytes) */
		if (transfer_size != 512)
			buffer[transfer_size] = 0;

		/* Before processing received data, re-arm EP1-OUT (toggles active buffer) */
		usb_ep1_out_prime();

		/* Now, process data */
		fastboot_interface->rx_handler(buffer, transfer_size);

	}
	else
	{
		//printf(" ... not timeout or transfer complete?\n");
		return 1;

	}
	return 0;
}

/**
 * @brief: PowerUp - PowerUp USB.
 * @param[in]:  none
 * @param[out]: none
 * @return: none
 */
static inline void PowerUp(void)
{
	// clear stop PHY clock
	writel( (0 << HSOTG_PCGCR_STOPPCLK_SHIFT), HSOTG_BASE_ADDR + HSOTG_PCGCR_OFFSET);
}

/**
 * @brief: usb_chirp_enum - Perform USB CHIRP.
 * @param[in]:  none
 * @param[out]: none
 * @return: OK or ERROR ID
 */
static inline int usb_chirp_enum(void)
{
	/* Enumerated Speed.
		00: High speed (PHY clock is running at 30 or 60 MHz)
		01: Full speed (PHY clock is running at 30 or 60 MHz)
		10: Low speed (PHY clock is running at 6 MHz)
		11: Full speed (PHY clock is running at 48 MHz)
	*/
	usb_speed = readl( HSOTG_BASE_ADDR + HSOTG_DSTS_OFFSET );
	usb_speed = (usb_speed & HSOTG_DSTS_ENUMSPD_MASK) >> HSOTG_DSTS_ENUMSPD_SHIFT;

	return 0;
}




/**************************/
/* Fastboot specific code */
/**************************/

void fastboot_shutdown(void)
{

}

int fastboot_is_highspeed(void)
{
	// Just return 0 for now
	// if (0 == ((readl(HSOTG_BASE_ADDR + HSOTG_DSTS_OFFSET) & HSOTG_DSTS_ENUMSPD_MASK) >> HSOTG_DSTS_ENUMSPD_SHIFT))
	// 	return 1;
	return 0;
}

int fastboot_tx_status(const char *buffer, unsigned int buffer_size)
{
	// This sends a packet via the bulk endpoint

	usb_send_data_ep1_in((uint8_t*) buffer, buffer_size);

	return 0;
}

/* Adds a string of board specific variables */
int fastboot_getvar(const char *rx_buffer, char *tx_buffer)
{
	/* Place board specific variables here */
	return 0;
}

int fastboot_init(struct cmd_fastboot_interface *interface) 
{
	uint32_t mask;
	int res = 0;

	/* The interface structure */
	fastboot_interface = interface;
	fastboot_interface->product_name                  = device_strings[DEVICE_STRING_PRODUCT_INDEX];
	fastboot_interface->serial_no                     = device_strings[DEVICE_STRING_SERIAL_NUMBER_INDEX];
	fastboot_interface->nand_block_size               = 2048;
	fastboot_interface->transfer_buffer               = (unsigned char *) CFG_FASTBOOT_TRANSFER_BUFFER;
	fastboot_interface->transfer_buffer_size          = CFG_FASTBOOT_TRANSFER_BUFFER_SIZE;

	fastboot_interface->reset_handler();

	/* On RHEA, C-Boot messes with USB... */
	/* In order for this to work you have to halt it before it gets too far along. */
	/* The USBOTGCONTROL register can be cleaned up by writing it back to its POR value */
	/* but if RSTCTL has been tainted its too late */

#if 0
	if (0x80000000 != readl(HSOTG_BASE_ADDR + HSOTG_RSTCTL_OFFSET)) {
		printf("You let C-Boot run too far before halting via JTAG...\n");
		printf("The USB is in a weird state... reboot and try again.\n");
		return 1;
	}
#endif

	writel(0x04008C4C, HSOTG_CTRL_BASE_ADDR + HSOTG_CTRL_USBOTGCONTROL_OFFSET);

	/* wait for clock settle down before checking vbus */
	udelay(32);

	/* wait for vbus */
	usb_wait_for_vbus();

	/* turn off VDP */
	usb_turn_off_vdp();

	/* usbotgcontrol bits usbotgcontrol registers */
	mask = HSOTG_CTRL_USBOTGCONTROL_UTMIOTG_IDDIG_SW_MASK|     // SW sets device mode bit 26
		HSOTG_CTRL_USBOTGCONTROL_USB_ON_MASK |              // turn on OTG core
		HSOTG_CTRL_USBOTGCONTROL_USB_ON_IS_HCLK_EN_MASK |   // use usb_on as source of AHB clock enable
		HSOTG_CTRL_USBOTGCONTROL_USB_HCLK_EN_DIRECT_MASK;   // explicit source of AHB clock enable

	wfld_set (HSOTG_CTRL_BASE_ADDR + HSOTG_CTRL_USBOTGCONTROL_OFFSET, mask, mask);

	udelay(1000);

	usb_soft_reset();
	udelay(1000);

	/* Initialize OTG device core */
	res = dfu_setup_device_mode();
	if (res) {
		//printf("Fail: not in device mode\n\r");
		usb_phy_disconnect();
		return 1;
	}

	/* Device All Endpoints Interrupt Mask Register */
	writel( HSOTG_INTMSK_WKUPINTMSK_MASK |      // Resume/Remote Wakeup Detected
		HSOTG_INTMSK_DISCONNINTMSK_MASK|    // Disconnect Detected
		HSOTG_INTMSK_OEPINTMSK_MASK|        // OUT Endpoints
		HSOTG_INTMSK_INEPINTMSK_MASK|       // IN Endpoints
		HSOTG_INTMSK_ENUMDONEMSK_MASK|      // Enumeration Done
		HSOTG_INTMSK_USBRSTMSK_MASK|        // USB Reset
		HSOTG_INTMSK_USBSUSPMSK_MASK|       // USB Suspend
		HSOTG_INTMSK_ERLYSUSPMSK_MASK|      // Early Suspend
		HSOTG_INTMSK_OTGINTMSK_MASK,        // OTG Interrupt
		HSOTG_BASE_ADDR + HSOTG_INTMSK_OFFSET);

	/* Device IN Endpoint Common Interrupt Mask Register */
	writel(	HSOTG_DIEPMSK_TIMEOUTMSK_MASK|      // Timeout Condition
		HSOTG_DIEPMSK_AHBERRMSK_MASK|       // AHB Error
		HSOTG_DIEPMSK_EPDISBLDMSK_MASK|     // Endpoint Disabled
		HSOTG_DIEPMSK_XFERCOMPLMSK_MASK,    // Transfer Completed
		HSOTG_BASE_ADDR + HSOTG_DIEPMSK_OFFSET);

	/* Device OUT Endpoint Common Interrupt Mask Register */
	writel( 
		HSOTG_DOEPMSK_SETUPMSK_MASK|        // SETUP Phase Done
		HSOTG_DOEPMSK_AHBERRMSK_MASK|       // AHB Error
		HSOTG_DOEPMSK_EPDISBLDMSK_MASK|     // Endpoint Disabled
		HSOTG_DOEPMSK_XFERCOMPLMSK_MASK,    // Transfer Completed
		HSOTG_BASE_ADDR + HSOTG_DOEPMSK_OFFSET);


	memset(ep0_setup_buf, 0, EP0_SETUP_BUF_SIZE);

	usb_ep0_setup_prime();
	usb_clr_interrupt();

	usb_phy_connect();

	/* dataline or Vbus Pulsing */
	mask = 	HSOTG_CFG_SRPCAP_MASK|	// HNP-Capable
		HSOTG_CFG_HNPCAP_MASK;	// SRP-Capable
	wfld_set (HSOTG_BASE_ADDR + HSOTG_CFG_OFFSET, mask, mask);

	mask = HSOTG_OTGCTL_SESREQ_MASK;
	wfld_set (HSOTG_BASE_ADDR + HSOTG_OTGCTL_OFFSET, mask, mask);

	usb_speed = 0;

	return 0;
}

int fastboot_poll(void) {

	uint32_t regval, intsts;

	WATCHDOG_RESET();

	intsts = readl(HSOTG_BASE_ADDR + HSOTG_INTSTS_OFFSET);

	if(intsts) {

		/* IN Endpoints Interrupt */
		if(intsts & HSOTG_INTSTS_IEPINT_MASK) {
			u32 daint = readl(HSOTG_BASE_ADDR + HSOTG_DAINT_OFFSET);

			//printf("daint = 0x%08X - I\n",daint);

			if (daint & (1<<(0 + HSOTG_DAINTMSK_INEPMSK_SHIFT)))
				usb_handle_ep0_in_int();
			if (daint & (1<<(1 + HSOTG_DAINTMSK_INEPMSK_SHIFT)))
				usb_handle_ep1_in_int();	
		}

		/* OUT Endpoints Interrupt */
		if(intsts &  HSOTG_INTSTS_OEPINT_MASK) {
			u32 daint = readl(HSOTG_BASE_ADDR + HSOTG_DAINT_OFFSET);

			//printf("daint = 0x%08X - O\n",daint);
			
			if (daint & (1<<(0 + HSOTG_DAINTMSK_OUTEPMSK_SHIFT)))
				usb_handle_ep0_out_int();
			if (daint & (1<<(1 + HSOTG_DAINTMSK_OUTEPMSK_SHIFT))) {
				usb_handle_ep1_out_int();
			}
		}

		/* IOTG Interrupt */
		if(intsts &HSOTG_INTSTS_OTGINT_MASK) {
			//printf("G");
			/* clear TG Interrupt Register */
			regval = readl( HSOTG_BASE_ADDR + HSOTG_OTGINT_OFFSET);
			writel(regval, HSOTG_BASE_ADDR + HSOTG_OTGINT_OFFSET);

			/* Session Request Success Status Change */
			if(regval & HSOTG_OTGINT_SESREQSUCSTSCHNG_MASK) {
				//printf("r\n");
			}

			/* Session End Detected */
			if(regval & HSOTG_OTGINT_SESENDDET_MASK) {
				//printf("e\n");

				udelay(2500);
				wfld_set (HSOTG_BASE_ADDR + HSOTG_OTGCTL_OFFSET, HSOTG_OTGCTL_SESREQ_MASK, HSOTG_OTGCTL_SESREQ_MASK);
			}

			/* ebounce Done */
			if(regval & HSOTG_OTGINT_DBNCEDONE_MASK) {
				//printf("d\n");
			}

			/* A-Device Timeout Change */
			if(regval & HSOTG_OTGINT_ADEVTOUTCHG_MASK){
				//printf("a\n");
			}
		}

		/* Enumeration Done */
		if(intsts & HSOTG_INTSTS_ENUMDONE_MASK) {
			//printf("E\n");
			usb_chirp_enum();
		}

		/* Early Suspend */
		if(intsts & HSOTG_INTSTS_ERLYSUSP_MASK)  {
			//printf("eS\n");
		}

		/* USB Suspend */
		if(intsts & HSOTG_INTSTS_USBSUSP_MASK) {
			//printf("Su\n");
		}

		/* WkUpInt */
		if(intsts & HSOTG_INTSTS_WKUPINT_MASK) {
			PowerUp();
			//printf("Rm\n");
		}

	}

	return 0;
}

int get_reset_reason(char* buf, int buflen)
{
    static unsigned long reset_status;
    static int got_reset_status = 0;

    if (!got_reset_status) {
        got_reset_status = 1;
        unsigned password = 0xa5a5;
        /* Enable write access to ROOT_RST registers. */
        writel(ROOT_RST_MGR_REG_WR_ACCESS_RSTMGR_ACC_MASK |
                (password << ROOT_RST_MGR_REG_WR_ACCESS_PASSWORD_SHIFT),
                ROOT_RST_BASE_ADDR + ROOT_RST_MGR_REG_WR_ACCESS_OFFSET);
        reset_status = readl(ROOT_RST_BASE_ADDR + ROOT_RST_MGR_REG_RSTSTS_SFTCLR_OFFSET);
        /* Write the same value back to zero the bits. */
        writel(reset_status, ROOT_RST_BASE_ADDR + ROOT_RST_MGR_REG_RSTSTS_SFTCLR_OFFSET);
    }
    if (reset_status & ROOT_RST_MGR_REG_RSTSTS_WRMRST_WDRST_DET_MASK)
        *buf++ = 'w';
    if (reset_status & ROOT_RST_MGR_REG_RSTSTS_CHIPWRMRST_DET_MASK)
        *buf++ = 's';
    if (reset_status & ROOT_RST_MGR_REG_RSTSTS_PORRST_DET_MASK)
        *buf++ = 'p';
    else if (reset_status & ROOT_RST_MGR_REG_RSTSTS_CHIPRST_DET_MASK)
        // chip reset without power-on reset must be button reset
        *buf++ = 'b';
    *buf = '\0';
    return 0;
}
