#include <common.h>
#include <mmc.h>
#include <asm/io.h>
#include <asm/arch/hardware.h>
#include <asm/arch/brcm_rdb_sysmap.h>
#include <asm/arch/brcm_rdb_kpm_clk_mgr_reg.h>

#include "kona_mmc.h"

#if 0
#define debug printf
#undef writel
#undef readl
static inline void writel(u32 val, void * addr)
{
	printf("Write [0x%08x] = 0x%08x\n", (u32)addr, val);
	*(volatile unsigned int *)addr = (volatile unsigned int)val;
}
static inline u32 readl(void * addr)
{
	volatile unsigned int val = *(volatile unsigned int *)addr;
	printf("Read  [0x%08x] = 0x%08x\n", (u32)addr, val);
	return (u32)val;
}
#endif

/* support 4 mmc hosts */
#define KONA_MAX_MMC_DEV 4
struct mmc mmc_dev[KONA_MAX_MMC_DEV];
struct mmc_host mmc_host[KONA_MAX_MMC_DEV];

static unsigned int kona_get_base_clock_freq(unsigned int reg_offset)
{
	unsigned int sdio_div;
	unsigned int clk_freq;
	unsigned int clk_div;

	sdio_div = readl(reg_offset);

	clk_freq = ((sdio_div & 0x7) < 3) ? 52000000 : 96000000;
	clk_div = ((sdio_div >> 4) & 0x3FFF) + 1;

	return clk_freq / clk_div;
}

static void mmc_prepare_data(struct mmc_host *host, struct mmc_data *data)
{
	unsigned int temp = 0;

	debug("data->dest: %08x\n", (u32)data->dest);
	v7_dma_flush_range(data->dest,data->dest+data->blocks*data->blocksize);

	writel((u32)data->dest, &host->reg->sysad);

	// Set up block size and block count. 
	// For KONA it is required to set HSBS field of block register tp 0x7.
	// Problem : When data is DMA across page boundaries, if HSBS field is set to 0,
	//           we see that transfer doesn't finish, and we see a hang.
	//           E.g : let's say a buffer in memory has a start address of 0x86234EF0.
	//                 During a read operation, data is read from SD in sizes of 512 bytes ( 0x200).
	//                 So now DMA of data will happen across address 0x86234F00, which is
	//                 a 4K page boundary. 
	//                 We see failures in this case when HSBS field is set to 0.
	//                 Solution is to set HSBS field val to 0x7 so that boundary DMA transactions are safe.
	//
	temp = ( 7 << EMMCSDXC_BLOCK_HSBS_SHIFT ) | data->blocksize | ( data->blocks << EMMCSDXC_BLOCK_BCNT_SHIFT ) ;
	writel(temp, &host->reg->blkcnt_sz);
}

static void kona_mmc_clear_all_intrs(struct mmc_host *host)
{
	writel(0xFFFFFFFF, &host->reg->norintsts ) ; // Clear all interrupts.
	udelay(1000);
}

static int kona_mmc_send_cmd(struct mmc *mmc, struct mmc_cmd *cmd,
			struct mmc_data *data)
{
	struct mmc_host *host = (struct mmc_host *)mmc->priv;
	int flags = 0 ;
	int i = 0 ;
	unsigned int timeout;
	unsigned int mask;
	unsigned int retry = 10000;
	static int readCmd_count = 0;
	/* Wait max 10 ms */
	timeout = 10;

	/*
	 * PRNSTS
	 * CMDINHDAT[1]	: Command Inhibit (DAT)
	 * CMDINHCMD[0]	: Command Inhibit (CMD)
	 */
	mask = (1 << EMMCSDXC_PSTATE_CMDINH_SHIFT);  // Set command inhibit.
	if ((data != NULL) || (cmd->resp_type & MMC_RSP_BUSY))
		mask |= (1 << EMMCSDXC_PSTATE_DATINH_SHIFT); // Set dat inhibit.

	/*
	 * We shouldn't wait for data inihibit for stop commands, even
	 * though they might use busy signaling
	 */
	if (data)
		mask &= ~(1 << EMMCSDXC_PSTATE_DATINH_SHIFT);

	while (readl(&host->reg->prnsts) & mask) {
		if (timeout == 0) {
			printf("%s : timeout error %d\n", __func__, cmd->cmdidx);
			return TIMEOUT ;
		}
		timeout--;
		udelay(1000);
	}

	// Set up block cnt, and block size.
	if (data)
       		mmc_prepare_data(host, data);

	debug("cmd->arg: %08x\n", cmd->cmdarg);
	if ( cmd->cmdidx == 17 )
	{
		/* print out something to indicate we are alive. 
		 */
		readCmd_count++;
		if (0 == (readCmd_count % 100))
			printf("."); 
	}

	writel(cmd->cmdarg, &host->reg->argument);

	flags = 0 ;
	if ( data ) 
	{
		flags = (1 << EMMCSDXC_CMD_BCEN_SHIFT) | (1 << EMMCSDXC_CMD_DMA_SHIFT);
		if (data->blocks > 1)
			flags |= (1 << EMMCSDXC_CMD_MSBS_SHIFT); // Multiple block select.
		if (data->flags & MMC_DATA_READ)
			flags |= (1 << EMMCSDXC_CMD_DTDS_SHIFT); // 1= read, 0=write.
	}

	if ((cmd->resp_type & MMC_RSP_136) && (cmd->resp_type & MMC_RSP_BUSY))
		return -1;

	/*
	 * CMDREG
	 * CMDIDX[29:24]: Command index
	 * DPS[21]	: Data Present Select
	 * CCHK_EN[20]	: Command Index Check Enable
	 * CRC_EN[19]	: Command CRC Check Enable
	 * RTSEL[1:0]
	 *	00 = No Response
	 *	01 = Length 136
	 *	10 = Length 48
	 *	11 = Length 48 Check busy after response
	 */
	if (!(cmd->resp_type & MMC_RSP_PRESENT))
		flags |= (0 << EMMCSDXC_CMD_RTSEL_SHIFT);
	else if (cmd->resp_type & MMC_RSP_136)
		flags |= (1 << EMMCSDXC_CMD_RTSEL_SHIFT);
	else if (cmd->resp_type & MMC_RSP_BUSY)
		flags |= (3 << EMMCSDXC_CMD_RTSEL_SHIFT);
	else
		flags |= (2 << EMMCSDXC_CMD_RTSEL_SHIFT);

	if (cmd->resp_type & MMC_RSP_CRC)
	{
		/* Skip CRC check of cmd2 and cmd10 to fix Hynix device.
		 * Vendor comfirmed to have this workaround
		 */
		if(cmd->cmdidx != MMC_CMD_ALL_SEND_CID &&
		   cmd->cmdidx != MMC_CMD_SEND_CID) {
			flags |= (1 << EMMCSDXC_CMD_CRC_EN_SHIFT);
		}
	}
	if (cmd->resp_type & MMC_RSP_OPCODE)
		flags |= (1 << EMMCSDXC_CMD_CCHK_EN_SHIFT);
	if (data)
		flags |= (1 << EMMCSDXC_CMD_DPS_SHIFT);

	debug("cmd: %d\n", cmd->cmdidx);
	flags |= ( cmd->cmdidx << EMMCSDXC_CMD_CIDX_SHIFT ) ;

	writel(flags, &host->reg->cmdreg);

	for (i = 0; i < retry; i++) {
		mask = readl(&host->reg->norintsts);
		/* Command Complete */
		if (mask & (1 << 0)) {
			if (!data)
				writel(mask, &host->reg->norintsts);
			break;
		}
		udelay(1);
	}

	if (i == retry) {
                // printf("%s: waiting for status update\n", __func__);
		// Set CMDRST and DATARST bits. 
		// Problem :
		// -------
		// When a command 8 is sent in case of MMC card, it will not respond, and CMD INHIBIT bit
		// of PRSTATUS register will be set to 1, causing no more commands to be sent from host controller.
		// This causes things to stall. 
		// Solution :
		// ---------
		// In order to avoid this situation, we clear the CMDRST and DATARST bits in the case when card 
		// doesn't respond back to a command sent by host controller.
		writel( ( ( 0x3 << EMMCSDXC_CTRL1_CMDRST_SHIFT )| ( readl(&host->reg->ctrl1_clkcon_timeout_swrst))), &host->reg->ctrl1_clkcon_timeout_swrst);
		while (( 0x3 << EMMCSDXC_CTRL1_CMDRST_SHIFT )& readl(&host->reg->ctrl1_clkcon_timeout_swrst));
		kona_mmc_clear_all_intrs(host) ;
		return TIMEOUT;
	}

	if (mask & (1 << 16)) {
		/* Timeout Error */
		debug("timeout: %08x cmd %d\n", mask, cmd->cmdidx);
		// Clear up the CMD inhibit and DATA inhibit bits.
		return TIMEOUT;
	} else if (mask & (1 << 15)) {
		/* Error Interrupt */
		debug("error: %08x cmd %d\n", mask, cmd->cmdidx);
		return -1;
	}

	if (cmd->resp_type & MMC_RSP_PRESENT) {
		if (cmd->resp_type & MMC_RSP_136) {
			/* CRC is stripped so we need to do some shifting. */
			for (i = 0; i < 4; i++) {
				unsigned int offset =
					(unsigned int)(&host->reg->rspreg3 - i);
				cmd->response[i] = readl(offset) << 8;

				if (i != 3) {
					cmd->response[i] |=
						readb(offset - 1);
				}
				debug("cmd->resp[%d]: %08x\n",
						i, cmd->response[i]);
			}
		} else if (cmd->resp_type & MMC_RSP_BUSY) {
			retry = 100000;

			if (cmd->cmdidx == 38)
				retry *= 1000;

			for (i = 0; i < retry; i++) {
				/* PRNTDATA[23:20] : DAT[3:0] Line Signal */
				if (readl(&host->reg->prnsts)
					& (1 << 20))	/* DAT[0] */
					break;
				udelay(1);
			}

			if (i == retry) {
				printf("%s: card is still busy\n", __func__);
				return TIMEOUT;
			}

			cmd->response[0] = readl(&host->reg->rspreg0);
			debug("cmd->resp[0]: %08x\n", cmd->response[0]);
		} else {
			cmd->response[0] = readl(&host->reg->rspreg0);
			debug("cmd->resp[0]: %08x\n", cmd->response[0]);
		}
	}

	if (data) {
		while (1) {
			mask = readl(&host->reg->norintsts);

			if (mask & EMMCSDXC_INTR_ERRIRQ_MASK) {
				/* Error Interrupt */
				writel(EMMCSDXC_INTR_ERRIRQ_MASK, &host->reg->norintsts);
				printf("%s: error during transfer: 0x%08x\n", __func__, mask);
				return -1;
			} else if (mask & EMMCSDXC_INTR_DMAIRQ_MASK) {
				/* DMA Interrupt */
				writel(EMMCSDXC_INTR_DMAIRQ_MASK, &host->reg->norintsts);
				writel(readl(&host->reg->sysad), &host->reg->sysad);
				debug("DMA end\n");
			} else if (mask & EMMCSDXC_INTR_TXDONE_MASK) {
				/* Transfer Complete */
				debug("r/w is done\n");
				break;
			}
		}
		writel(mask, &host->reg->norintsts);
#ifdef CONFIG_ISLAND_SV
		if(cmd->cmdidx==8) {
			/* use 1m delay to work with all SV boards */
			udelay(1000);
		}
#endif
	}

	// Clear all interrupts as per FPGA code.
	writel(0xFFFFFFFF, &host->reg->norintsts ) ;
	// udelay(1000);
	return 0;
}


static void kona_mmc_change_clock(struct mmc_host *host, uint clock)
{
	int div = 0;
	unsigned int clk;
	unsigned long timeout;

	clk = readl(&host->reg->ctrl1_clkcon_timeout_swrst) ;
	clk = clk & 0xFFFF0000  ; // Clean up all bits related to clock.
	writel(clk, &host->reg->ctrl1_clkcon_timeout_swrst);
	clk = 0 ;

	div = host->base_clock_freq/clock/2 ;
	div = (host->base_clock_freq % clock) ? div+1 : div;
#if defined(CONFIG_SAMOA_FPGA) || defined(CONFIG_BCM11140_FPGA)
	div =0; // SDIO clk divider does not work with SAMOA FPGA. It is set to 0
#endif
	debug("div: %d\n", div);

	// Write divider value, and enable internal clock.
	clk = readl(&host->reg->ctrl1_clkcon_timeout_swrst) | (div << EMMCSDXC_CTRL1_SDCLKSEL_SHIFT) | (1 << EMMCSDXC_CTRL1_ICLKEN_SHIFT) ;
	writel(clk, &host->reg->ctrl1_clkcon_timeout_swrst);

	/* Wait for clock to stabilize */
	/* Wait max 10 ms */
	timeout = 10;
	while (!(readl(&host->reg->ctrl1_clkcon_timeout_swrst) & (1 << EMMCSDXC_CTRL1_ICLKSTB_SHIFT))) {
		if (timeout == 0) {
			printf("%s: timeout error\n", __func__);
			return;
		}
		timeout--;
		udelay(1000);
	}

	// Enable sdio clock now.
	clk |= (1 << EMMCSDXC_CTRL1_SDCLKEN_SHIFT) | readl(&host->reg->ctrl1_clkcon_timeout_swrst) ;
	writel(clk, &host->reg->ctrl1_clkcon_timeout_swrst);

	host->clock = clock;
}

static void kona_mmc_set_ios(struct mmc *mmc)
{
	struct mmc_host *host = mmc->priv;
	unsigned char ctrl;

	debug("bus_width: %x, clock: %d\n", mmc->bus_width, mmc->clock);

	if (mmc->clock)
		kona_mmc_change_clock(host, mmc->clock);

	// Width and edge setting.
	// WIDTH : 
	ctrl = readl(&host->reg->ctrl_host_pwr_blk_wak);

#ifdef CONFIG_EMMC_8BIT
	if (mmc->bus_width == 8) {
		ctrl &= ~(1 << EMMCSDXC_CTRL_DXTW_SHIFT);
		ctrl |= (1 << EMMCSDXC_CTRL_SDB_SHIFT);
	} else {
#endif
		/*  1 = 4-bit mode , 0 = 1-bit mode */
		if (mmc->bus_width == 4)
			ctrl |= (1 << EMMCSDXC_CTRL_DXTW_SHIFT);
		else
			ctrl &= ~(1 << EMMCSDXC_CTRL_DXTW_SHIFT);
#ifdef CONFIG_EMMC_8BIT
	}
#endif

	if(mmc->card_caps & MMC_MODE_HS)
#ifdef CONFIG_SAMOA_FPGA
		ctrl &= ~(1 << EMMCSDXC_CTRL_HSEN_SHIFT);
#else
		ctrl |= (1 << EMMCSDXC_CTRL_HSEN_SHIFT);
#endif
	else
		ctrl &= ~(1 << EMMCSDXC_CTRL_HSEN_SHIFT);
	writel(ctrl, &host->reg->ctrl_host_pwr_blk_wak);


}

static void kona_mmc_reset(struct mmc_host *host)
{
	unsigned int timeout;

	/* Software reset for all * 1 = reset * 0 = work */
	writel((1 << EMMCSDXC_CTRL1_RST_SHIFT), &host->reg->ctrl1_clkcon_timeout_swrst);

	host->clock = 0;

	/* Wait max 100 ms */
	timeout = 100;

	/* hw clears the bit when it's done */
	while (readl(&host->reg->ctrl1_clkcon_timeout_swrst) & (1 << EMMCSDXC_CTRL1_RST_SHIFT)) {
		if (timeout == 0) {
			printf("%s: timeout error\n", __func__);
			return;
		}
		timeout--;
		udelay(1000);
	}
}

static int kona_mmc_core_init(struct mmc *mmc)
{
	struct mmc_host *host = (struct mmc_host *)mmc->priv;
	unsigned int mask;

#ifndef CONFIG_CAPRI
	// For kona a hardware reset before anything else.
	// TBD : Remove this, it is needed in case of Uboot bring up only.
	writel( EMMCSDXC_CORECTRL_EN_MASK ,&host->reg_p3->corectrl) ;

	// Set the reset bit, wait for some time, and clear the reset bit. 
	// Set the reset bit.
	mask = readl(&host->reg_p3->corectrl) | EMMCSDXC_CORECTRL_RESET_MASK ;
	writel( mask , &host->reg_p3->corectrl) ;
	udelay(10) ;

	// Clear the reset bit.
	mask = mask & ~(EMMCSDXC_CORECTRL_RESET_MASK) ;
	writel( mask , &host->reg_p3->corectrl) ;
	udelay(10) ;
#else
	unsigned int timeout;
	
	if (readl(&host->reg->ctrl1_clkcon_timeout_swrst) & (1 << EMMCSDXC_CTRL1_RST_SHIFT))
	{
		printf("%s: sd host controller reset error\n", __func__);
		return -1;
	}

	// For kona a hardware reset before anything else.
	mask = readl(&host->reg_p3->corectrl) | EMMCSDXC_CORECTRL_RESET_MASK ;
	writel( mask , &host->reg_p3->corectrl) ;

	// Wait max 100 ms
	timeout = 1000;
	do
	{
		if (timeout == 0) {
			printf("%s: reset timeout error\n", __func__);
			return -1;
		}
	timeout--;
	udelay(100);
	} while (0 == (readl(&host->reg_p3->corectrl) | EMMCSDXC_CORECTRL_RESET_MASK));

	// Clear the reset bit.
	mask = mask & ~(EMMCSDXC_CORECTRL_RESET_MASK) ;
	writel( mask , &host->reg_p3->corectrl) ;
	udelay(10) ;

	// Enable AHB clock
	mask = readl(&host->reg_p3->corectrl);
	writel(mask | EMMCSDXC_CORECTRL_EN_MASK ,&host->reg_p3->corectrl) ;

	// Enable interrupts
	writel(EMMCSDXC_COREIMR_IP_MASK ,&host->reg_p3->coreimr) ;

	
#endif

	// Set power now.
	mask = readl(&host->reg->ctrl_host_pwr_blk_wak) | ( 7 << EMMCSDXC_CTRL_SDVSEL_SHIFT ) | EMMCSDXC_CTRL_SDPWR_MASK ;
	writel(mask, &host->reg->ctrl_host_pwr_blk_wak ) ;    
#ifndef CONFIG_CAPRI
	kona_mmc_reset(host);
#endif
	host->version = ( readl(&host->reg_p2->hcversirq) | EMMCSDXC_HCVERSIRQ_VENDVER_MASK ) >> EMMCSDXC_HCVERSIRQ_VENDVER_SHIFT ;

	/* mask all */
	writel(0xffffffff, &host->reg->norintstsen);
	writel(0xffffffff, &host->reg->norintsigen);

	writel( ( ( 0xd << EMMCSDXC_CTRL1_DTCNT_SHIFT )| ( readl(&host->reg->ctrl1_clkcon_timeout_swrst))), &host->reg->ctrl1_clkcon_timeout_swrst);	/* TMCLK * 2^26 */

	/*
	* Interrupt Status Enable Register init
	* bit 5 : Buffer Read Ready Status Enable
	* bit 4 : Buffer write Ready Status Enable
	* bit 1 : Transfre Complete Status Enable
	* bit 0 : Command Complete Status Enable
	*/
	mask = readl(&host->reg->norintstsen);
	mask &= ~(0xffff);
	mask |=	(1 << EMMCSDXC_INTREN1_BUFRREN_SHIFT) |
		(1 << EMMCSDXC_INTREN1_BUFWREN_SHIFT) |
		(1 << EMMCSDXC_INTREN1_DMAIRQEN_SHIFT) |
		(1 << EMMCSDXC_INTREN1_TXDONEEN_SHIFT) |
		(1 << EMMCSDXC_INTREN1_CMDDONEEN_SHIFT);
	writel(mask, &host->reg->norintstsen);

	/*
	* Interrupt Signal Enable Register init
	* bit 1 : Transfer Complete Signal Enable
	*/
	mask = readl(&host->reg->norintsigen);
	mask &= ~(0xffff);
	mask |= (1 << EMMCSDXC_INTREN2_TXDONE_SHIFT);
	writel(mask, &host->reg->norintsigen);

	return 0;
}

int kona_mmc_init(int dev_index)
{
	struct mmc *mmc;
	void* mmc_reg_base;
	unsigned int source_clk_reg;
	
	/* 0 based index required for mmc structures */
	dev_index = dev_index - 1;

	switch (dev_index) {
		case 0:
			mmc_reg_base = (void*) SDIO1_BASE_ADDR;
			source_clk_reg = KONA_MST_CLK_BASE_ADDR + KPM_CLK_MGR_REG_SDIO1_DIV_OFFSET;
			break;
		case 1:
			mmc_reg_base = (void*) SDIO2_BASE_ADDR;
			source_clk_reg = KONA_MST_CLK_BASE_ADDR + KPM_CLK_MGR_REG_SDIO2_DIV_OFFSET;
			break;
#ifdef SDIO3_BASE_ADDR
		case 2:
			mmc_reg_base = (void*) SDIO3_BASE_ADDR;
			source_clk_reg = KONA_MST_CLK_BASE_ADDR + KPM_CLK_MGR_REG_SDIO3_DIV_OFFSET;
			break;
#endif			
#ifdef SDIO4_BASE_ADDR
		case 3:
			mmc_reg_base = (void*) SDIO4_BASE_ADDR;
			source_clk_reg = KONA_MST_CLK_BASE_ADDR + KPM_CLK_MGR_REG_SDIO4_DIV_OFFSET;
			break;
#endif
		default:
			printf("Only support up to %d mmc device\n", KONA_MAX_MMC_DEV);
			return -1;
	}

	mmc = &mmc_dev[dev_index];

	sprintf(mmc->name, "KONA SD/MMC");
	mmc->priv = &mmc_host[dev_index];
	mmc->send_cmd = kona_mmc_send_cmd;
	mmc->set_ios = kona_mmc_set_ios;
	mmc->init = kona_mmc_core_init;

	mmc->voltages = MMC_VDD_27_28 | MMC_VDD_28_29 | MMC_VDD_29_30 | MMC_VDD_30_31 | MMC_VDD_31_32 | MMC_VDD_32_33 | MMC_VDD_33_34 | MMC_VDD_34_35 | MMC_VDD_35_36 ;

#ifdef CONFIG_BCM11140_FPGA
	mmc->host_caps = MMC_MODE_4BIT;
	mmc_host[dev_index].base_clock_freq = 400000;
#else
#ifdef CONFIG_EMMC_8BIT
	if (!IS_SD(mmc)) {
		mmc->host_caps = MMC_MODE_8BIT | MMC_MODE_4BIT | MMC_MODE_HS_52MHz | MMC_MODE_HS;
	} else {
#endif
		mmc->host_caps = MMC_MODE_4BIT | MMC_MODE_HS_52MHz | MMC_MODE_HS;
#ifdef CONFIG_EMMC_8BIT
	}
#endif
	mmc_host[dev_index].base_clock_freq = kona_get_base_clock_freq(source_clk_reg);
#endif

#ifdef CONFIG_ISLAND_SV
	/* Island SV board cannot support up to 400kHhz from eMMC initialization */
	mmc->f_min = 100000;
#else
	mmc->f_min = 400000; 
#endif
	mmc->f_max = mmc_host[dev_index].base_clock_freq;
	
	mmc_host[dev_index].clock = 0;
	mmc_host[dev_index].reg = mmc_reg_base;
	mmc_host[dev_index].reg_p2 = mmc_reg_base + EMMCSDXC_SBUSCTRL_OFFSET;
	mmc_host[dev_index].reg_p3 = mmc_reg_base + EMMCSDXC_CORECTRL_OFFSET;

	mmc_register(mmc);

	return 0;
}
