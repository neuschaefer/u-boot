#include <common.h>
#include <asm/io.h>
#include <asm/mach-types.h>
#include <mmc.h>
#include <i2c.h>
#include <asm/arch/brcm_rdb_kpm_clk_mgr_reg.h>
#include <asm/arch/brcm_rdb_padctrlreg.h>
#include <asm/arch/brcm_rdb_hsotg_ctrl.h>
#include <asm/arch/brcm_rdb_hsotg.h>
#include <asm/arch/brcm_rdb_sysmap_a5.h>

#include <asm/kona-common/clk.h>

#include <fastboot.h>

extern void init_clock_framework(void);
extern int kona_mmc_init(int dev_index);

#ifdef CONFIG_HW_WATCHDOG
extern void hw_watchdog_init(void);
extern void hw_watchdog_disable(void);
#endif /* CONFIG_HW_WATCHDOG */

#define NUM_FASTBOOT_PARTITIONS 21

DECLARE_GLOBAL_DATA_PTR;


/*****************************************
 * board_init -early hardware init
 *****************************************/
int board_init (void)
{
	gd->bd->bi_arch_number = CONFIG_MACH_TYPE;      /* board id for linux */
	gd->bd->bi_boot_params = LINUX_BOOT_PARAM_ADDR; /* adress of boot parameters */


#ifdef CONFIG_HW_WATCHDOG
	hw_watchdog_init();
#endif /* CONFIG_HW_WATCHDOG*/

	return 0;
}

/*****************************************************************
 * misc_init_r - miscellaneous platform dependent initializations
 ******************************************************************/
int misc_init_r (void)
{
#ifdef CONFIG_HW_WATCHDOG
	char *s;

	/* disable watchdog if environment is set */
	if (((s = getenv ("watchdog")) != NULL) && (strncmp (s, "off", 3) == 0))
		hw_watchdog_disable();
#endif /* CONFIG_HW_WATCHDOG*/

	return(0);
}

/**********************************************
 * dram_init - sets uboots idea of sdram size
 **********************************************/
int dram_init (void)
{
	gd->ram_size = PHYS_SDRAM_1_SIZE;
	return 0;
}

void dram_init_banksize (void)
{
	gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
	gd->bd->bi_dram[0].size = PHYS_SDRAM_1_SIZE;
}
#ifdef CONFIG_KONA_MMC

static void mmc_enable(void)
{
	unsigned int lock1, lock2;

	/* SDIO2 Clock 26 MHz = 52 MHz / 2 */
	struct clk *sdio2_clk, *sdio2_ahb_clk;

#if 0
	sdio2_clk = clk_get("sdio2_clk");
	if (sdio2_clk) {
		clk_set_rate(sdio2_clk, 26000000);
		clk_enable(sdio2_clk);
	}
	else
		printf("Couldn't find sdio2_clk\n");

	sdio2_ahb_clk = clk_get("sdio2_ahb_clk");
	if (sdio2_ahb_clk)
		clk_enable(sdio2_ahb_clk);
	else
		printf("Couldn't find sdio2_ahb_clk\n");

	udelay(1000);
#endif

	/* Write down DMA Boundary (256k) */
	writel(	(readl(KONA_PA_SDIO2 + 0x4) & ~0x00007000) | 0x6,
		(KONA_PA_SDIO2 + 0x4));


	/* Mux GPIO to SDIO 2 (MMC) */
	lock1 = readl(KONA_PAD_CTRL_BASE_ADDR + PADCTRLREG_ACCESS_LOCK1_OFFSET);
	lock2 = readl(KONA_PAD_CTRL_BASE_ADDR + PADCTRLREG_ACCESS_LOCK2_OFFSET);

	writel(0xA5A501, KONA_PAD_CTRL_BASE_ADDR + PADCTRLREG_WR_ACCESS_OFFSET);
	writel(	0, (KONA_PAD_CTRL_BASE_ADDR + PADCTRLREG_ACCESS_LOCK1_OFFSET));
	writel(	0, (KONA_PAD_CTRL_BASE_ADDR + PADCTRLREG_ACCESS_LOCK2_OFFSET));

	writel(	(0 << PADCTRLREG_MMC_CLK_PINSEL_MMC_CLK_SHIFT) |
		(0x3 << PADCTRLREG_MMC_CLK_SEL_0_MMC_CLK_SHIFT),
		KONA_PAD_CTRL_BASE_ADDR + PADCTRLREG_MMC_CLK_OFFSET);

	writel(	(0 << PADCTRLREG_MMC_CMD_PINSEL_MMC_CMD_SHIFT) |
		(0x3 << PADCTRLREG_MMC_CMD_SEL_0_MMC_CMD_SHIFT) |
		PADCTRLREG_MMC_CMD_PUP_MMC_CMD_MASK,
		KONA_PAD_CTRL_BASE_ADDR + PADCTRLREG_MMC_CMD_OFFSET);

	writel(	(0 << PADCTRLREG_MMC_DAT0_PINSEL_MMC_DAT0_SHIFT) |
		(0x3 << PADCTRLREG_MMC_DAT0_SEL_0_MMC_DAT0_SHIFT) |
		PADCTRLREG_MMC_DAT0_PUP_MMC_DAT0_MASK,
		KONA_PAD_CTRL_BASE_ADDR + PADCTRLREG_MMC_DAT0_OFFSET);

	writel(	(0 << PADCTRLREG_MMC_DAT1_PINSEL_MMC_DAT1_SHIFT) |
		(0x3 << PADCTRLREG_MMC_DAT1_SEL_0_MMC_DAT1_SHIFT) |
		PADCTRLREG_MMC_DAT1_PUP_MMC_DAT1_MASK,
		KONA_PAD_CTRL_BASE_ADDR + PADCTRLREG_MMC_DAT1_OFFSET);

	writel(	(0 << PADCTRLREG_MMC_DAT2_PINSEL_MMC_DAT2_SHIFT) |
		(0x3 << PADCTRLREG_MMC_DAT2_SEL_0_MMC_DAT2_SHIFT) |
		PADCTRLREG_MMC_DAT2_PUP_MMC_DAT2_MASK,
		KONA_PAD_CTRL_BASE_ADDR + PADCTRLREG_MMC_DAT2_OFFSET);

	writel(	(0 << PADCTRLREG_MMC_DAT3_PINSEL_MMC_DAT3_SHIFT) |
		(0x3 << PADCTRLREG_MMC_DAT3_SEL_0_MMC_DAT3_SHIFT) |
		PADCTRLREG_MMC_DAT3_PUP_MMC_DAT3_MASK,
		KONA_PAD_CTRL_BASE_ADDR + PADCTRLREG_MMC_DAT3_OFFSET);

	writel(	(0 << PADCTRLREG_MMC_DAT4_PINSEL_MMC_DAT4_SHIFT) |
		(0x3 << PADCTRLREG_MMC_DAT4_SEL_0_MMC_DAT4_SHIFT) |
		PADCTRLREG_MMC_DAT4_PUP_MMC_DAT4_MASK,
		KONA_PAD_CTRL_BASE_ADDR + PADCTRLREG_MMC_DAT4_OFFSET);

	writel(	(0 << PADCTRLREG_MMC_DAT5_PINSEL_MMC_DAT5_SHIFT) |
		(0x3 << PADCTRLREG_MMC_DAT5_SEL_0_MMC_DAT5_SHIFT) |
		PADCTRLREG_MMC_DAT5_PUP_MMC_DAT5_MASK,
		KONA_PAD_CTRL_BASE_ADDR + PADCTRLREG_MMC_DAT5_OFFSET);

	writel(	(0 << PADCTRLREG_MMC_DAT6_PINSEL_MMC_DAT6_SHIFT) |
		(0x3 << PADCTRLREG_MMC_DAT6_SEL_0_MMC_DAT6_SHIFT) |
		PADCTRLREG_MMC_DAT6_PUP_MMC_DAT6_MASK,
		KONA_PAD_CTRL_BASE_ADDR + PADCTRLREG_MMC_DAT6_OFFSET);

	writel(	(0 << PADCTRLREG_MMC_DAT7_PINSEL_MMC_DAT7_SHIFT) |
		(0x3 << PADCTRLREG_MMC_DAT7_SEL_0_MMC_DAT7_SHIFT) |
		PADCTRLREG_MMC_DAT7_PUP_MMC_DAT7_MASK,
		KONA_PAD_CTRL_BASE_ADDR + PADCTRLREG_MMC_DAT7_OFFSET);

	writel(	(0 << PADCTRLREG_MMC_RST_PINSEL_MMC_RST_SHIFT) |
		(0x3 << PADCTRLREG_MMC_RST_SEL_0_MMC_RST_SHIFT) |
		PADCTRLREG_MMC_RST_PUP_MMC_RST_MASK,
		KONA_PAD_CTRL_BASE_ADDR + PADCTRLREG_MMC_RST_OFFSET);

	writel(	0xA5A501, KONA_PAD_CTRL_BASE_ADDR + PADCTRLREG_WR_ACCESS_OFFSET);
	writel(	lock1, (KONA_PAD_CTRL_BASE_ADDR + PADCTRLREG_ACCESS_LOCK1_OFFSET));
	writel(	lock2, (KONA_PAD_CTRL_BASE_ADDR + PADCTRLREG_ACCESS_LOCK2_OFFSET));

}


/*******************************************
 * sd_enable - power it on, set up clock and 
 * pinmux  
 *********************************************/
static void sd_enable(void)
{
	unsigned char val = 0;
	unsigned int lock2;
	struct clk *sdio1_clk, *sdio1_ahb_clk;

#if 0
	/* PMU is connected on I2C bus 2 */
	i2c_set_bus_num(2);

	/* Enable HVLDO4 and HVLDO6 */
	i2c_write (0x08, 0xA5, 1, &val, 1) ;
	i2c_write (0x08, 0xA7, 1, &val, 1) ;

	/* Set HVLDO4 and HVLDO6 voltage to 3.0v */
	i2c_read (0x08, 0xB5, 1, &val,  1) ;
	val |= 0x06;
	i2c_write (0x08, 0xB5, 1, &val, 1) ;

	i2c_read (0x08, 0xB7, 1, &val,  1) ;
	val |= 0x06;
	i2c_write (0x08, 0xB7, 1, &val, 1) ;

	/* SDIO1 Clock */

	sdio1_clk = clk_get("sdio1_clk");
	if (sdio1_clk)
		clk_enable(sdio1_clk);
	else
		printf("Couldn't find sdio1_clk\n");

	sdio1_ahb_clk = clk_get("sdio1_ahb_clk");
	if (sdio1_ahb_clk)
		clk_enable(sdio1_ahb_clk);
	else
		printf("Couldn't find sdio1_ahb_clk\n");
#endif

	/* Mux GPIO to SDIO1 (SD) */
	lock2 = readl(KONA_PAD_CTRL_BASE_ADDR + PADCTRLREG_ACCESS_LOCK2_OFFSET);
	writel(0xA5A501, KONA_PAD_CTRL_BASE_ADDR + PADCTRLREG_WR_ACCESS_OFFSET);
	writel(0x0, KONA_PAD_CTRL_BASE_ADDR + PADCTRLREG_ACCESS_LOCK2_OFFSET);

	writel(	(0 << PADCTRLREG_SD_CLK_PINSEL_SD_CLK_SHIFT) |
		(0x3 << PADCTRLREG_SD_CLK_SEL_0_SD_CLK_SHIFT) |
		PADCTRLREG_SD_CLK_PDN_SD_CLK_MASK,
		KONA_PAD_CTRL_BASE_ADDR + PADCTRLREG_MMC_CLK_OFFSET);

	writel(	(0 << PADCTRLREG_SD_CMD_PINSEL_SD_CMD_SHIFT) |
		(0x3 << PADCTRLREG_SD_CMD_SEL_0_SD_CMD_SHIFT) |
		PADCTRLREG_SD_CMD_PUP_SD_CMD_MASK,
		KONA_PAD_CTRL_BASE_ADDR + PADCTRLREG_MMC_CMD_OFFSET);

	writel(	(0 << PADCTRLREG_SD_DAT0_PINSEL_SD_DAT0_SHIFT) |
		(0x3 << PADCTRLREG_SD_DAT0_SEL_0_SD_DAT0_SHIFT) |
		PADCTRLREG_SD_DAT0_PUP_SD_DAT0_MASK,
		KONA_PAD_CTRL_BASE_ADDR + PADCTRLREG_MMC_DAT0_OFFSET);

	writel(	(0 << PADCTRLREG_SD_DAT1_PINSEL_SD_DAT1_SHIFT) |
		(0x3 << PADCTRLREG_SD_DAT1_SEL_0_SD_DAT1_SHIFT) |
		PADCTRLREG_SD_DAT1_PUP_SD_DAT1_MASK,
		KONA_PAD_CTRL_BASE_ADDR + PADCTRLREG_MMC_DAT1_OFFSET);

	writel(	(0 << PADCTRLREG_SD_DAT2_PINSEL_SD_DAT2_SHIFT) |
		(0x3 << PADCTRLREG_SD_DAT2_SEL_0_SD_DAT2_SHIFT) |
		PADCTRLREG_SD_DAT2_PUP_SD_DAT2_MASK,
		KONA_PAD_CTRL_BASE_ADDR + PADCTRLREG_MMC_DAT2_OFFSET);

	writel(	(0 << PADCTRLREG_SD_DAT3_PINSEL_SD_DAT3_SHIFT) |
		(0x3 << PADCTRLREG_SD_DAT3_SEL_0_SD_DAT3_SHIFT) |
		PADCTRLREG_SD_DAT3_PUP_SD_DAT3_MASK,
		KONA_PAD_CTRL_BASE_ADDR + PADCTRLREG_MMC_DAT3_OFFSET);

	writel(0xA5A501, KONA_PAD_CTRL_BASE_ADDR + PADCTRLREG_WR_ACCESS_OFFSET);
	writel(lock2, KONA_PAD_CTRL_BASE_ADDR + PADCTRLREG_ACCESS_LOCK2_OFFSET);

}

/*******************************************
 * add_fastboot_partitions - Add eMMC partitions to Fastboot table
 *********************************************/
void add_fastboot_partitions(void)
{
	fastboot_ptentry ptn[NUM_FASTBOOT_PARTITIONS] = {
		/* ABI Partition */
		{
			.name   = "abi",
			.start  = 0x0,
			.length = 0x0008000,
			.flags  = 0,
		},

		/* ABI Extended Partition */
		{
			.name   = "abi-ext",
			.start  = 0x40,
			.length = 0x0008000,
			.flags  = 0,
		},

		/* HW Configuration Partition */
		{
			.name   = "hwconf",
			.start  = 0x80,
			.length = 0x0008000,
			.flags  = 0,
		},

		/* Loader Partition */
		{
			.name   = "loader",
			.start  = 0x100,
			.length = 0x0020000,
			.flags  = 0,
		},

		/* Customer Cert Partition */
		{
			.name   = "custom-cert",
			.start  = 0x200,
			.length = 0x0010000,
			.flags  = 0,
		},

		/* Boot Paramaters Partition */
		{
			.name   = "boot-parm",
			.start  = 0x280,
			.length = 0x0010000,
			.flags  = 0,
		},

		/* Param Dependent Partition */
		{
			.name   = "sys-parm-dep",
			.start  = 0x600,
			.length = 0x0010000,
			.flags  = 0,
		},

		/* Param Independent Partition */
		{
			.name   = "sys-parm-ind",
			.start  = 0x400,
			.length = 0x0040000,
			.flags  = 0,
		},

		/* Param SPML Independent Partition */
		{
			.name   = "parm-spml-ind",
			.start  = 0x680,
			.length = 0x0010000,
			.flags  = 0,
		},

		/* Param SPML Dependent Partition */
		{
			.name   = "parm-spml-dep",
			.start  = 0x880,
			.length = 0x0010000,
			.flags  = 0,
		},

		/* UMTS Calibration Partition */
		{
			.name   = "umts-cal",
			.start  = 0xe80,
			.length = 0x0020000,
			.flags  = 0,
		},

		/* CP Boot Partition */
		{
			.name   = "cp-boot",
			.start  = 0xf80,
			.length = 0x0080000,
			.flags  = 0,
		},

		/* CP Image Partition */
		{
			.name   = "cp-image",
			.start  = 0x1000,
			.length = 0xd00000,
			.flags  = 0,
		},

		/* CP POD Partition */
		{
			.name   = "cp-pod",
			.start  = 0x7800,
			.length = 0x100000,
			.flags  = 0,
		},

		/* DSP PRAM Partition */
		{
			.name   = "dsp-pram",
			.start  = 0xf900,
			.length = 0x0080000,
			.flags  = 0,
		},

		/* DSP DRAM Partition */
		{
			.name   = "dsp-dram",
			.start  = 0x10000,
			.length = 0x0200000,
			.flags  = 0,
		},

		/* U-Boot Raw Partition */
		{
			.name   = "u-boot",
			.start  = 0x11000,
			.length = 0x100000, 	/* Length in Bytes 1 MB */
			.flags  = FASTBOOT_PTENTRY_FLAGS_GPT_ENTRY,
		},

		/* U-Boot Environment Partition */
		{
			.name   = "u-boot-env",
			.start  = 0x11a00,
			.length = 0x0001000,
			.flags  = FASTBOOT_PTENTRY_FLAGS_GPT_ENTRY,
		},

		/* Kernel Raw Partition */
		{
			.name   = "kernel",
			.start  = 0x12000,
			.length = 0x1400000,	/* Length in Bytes 20 MB */
			.flags  = FASTBOOT_PTENTRY_FLAGS_GPT_ENTRY,
		},

		/* ~ 10 MB of Free Space for Recovery Image */

		/* System Ext4 Partition */
		{
			.name   = "system",
			.start  = 0x20000,
			.length = 0x8300000,	/* Length in Bytes 131 MB */
			.flags  = FASTBOOT_PTENTRY_FLAGS_GPT_ENTRY |
				  FASTBOOT_PTENTRY_FLAGS_SPARSE_IMG,
		},

		/* User Data Ext4 Partition */
		{
			.name   = "userdata",
			.start  = 0x61800,
			.length = 0x2000000,	/* Length in Bytes 32 MB */
			.flags  = FASTBOOT_PTENTRY_FLAGS_GPT_ENTRY |
				  FASTBOOT_PTENTRY_FLAGS_SPARSE_IMG,
		},

	};
	int i;
	for (i = 0; i < NUM_FASTBOOT_PARTITIONS; i++)
		fastboot_flash_add_ptn(&ptn[i]);
}


/*******************************************
 * mmc_init - Initializes mmc 
 *********************************************/
int board_mmc_init(bd_t *bis)
{
	int sts;

	/* Register eMMC - SDIO2 */
	mmc_enable();
	sts = kona_mmc_init(2);

	if(sts)
	{
		puts("eMMC init failed");
		return sts;
	}

	add_fastboot_partitions();

	/* Register SD Card - SDIO1 */
	sd_enable();
	sts = kona_mmc_init(1);

	if (sts)
	{
		puts("SD card init failed");
	}
	
	return sts;
}

static void otg_enable(void)
{
	writel( 0xA5A501,
		KONA_MST_CLK_BASE_ADDR + KPM_CLK_MGR_REG_WR_ACCESS_OFFSET);

	writel( KPM_CLK_MGR_REG_LVM_EN_POLICY_CONFIG_EN_MASK,
		KONA_MST_CLK_BASE_ADDR + KPM_CLK_MGR_REG_LVM_EN_OFFSET);

	while ( readl(KONA_MST_CLK_BASE_ADDR + KPM_CLK_MGR_REG_LVM_EN_OFFSET) &
		KPM_CLK_MGR_REG_LVM_EN_POLICY_CONFIG_EN_MASK) {;}

	writel( readl(KONA_MST_CLK_BASE_ADDR + KPM_CLK_MGR_REG_KPM_POLICY0_MASK_OFFSET) |
		KPM_CLK_MGR_REG_KPM_POLICY0_MASK_USBH_POLICY0_MASK_MASK,
		KONA_MST_CLK_BASE_ADDR + KPM_CLK_MGR_REG_KPM_POLICY0_MASK_OFFSET);

	writel( readl(KONA_MST_CLK_BASE_ADDR + KPM_CLK_MGR_REG_KPM_POLICY1_MASK_OFFSET) |
		KPM_CLK_MGR_REG_KPM_POLICY1_MASK_USBH_POLICY1_MASK_MASK,
		KONA_MST_CLK_BASE_ADDR + KPM_CLK_MGR_REG_KPM_POLICY1_MASK_OFFSET);

	writel( readl(KONA_MST_CLK_BASE_ADDR + KPM_CLK_MGR_REG_KPM_POLICY2_MASK_OFFSET) |
		KPM_CLK_MGR_REG_KPM_POLICY2_MASK_USBH_POLICY2_MASK_MASK,
		KONA_MST_CLK_BASE_ADDR + KPM_CLK_MGR_REG_KPM_POLICY2_MASK_OFFSET);

	writel( readl(KONA_MST_CLK_BASE_ADDR + KPM_CLK_MGR_REG_KPM_POLICY3_MASK_OFFSET) |
		KPM_CLK_MGR_REG_KPM_POLICY3_MASK_USBH_POLICY3_MASK_MASK,
		KONA_MST_CLK_BASE_ADDR + KPM_CLK_MGR_REG_KPM_POLICY3_MASK_OFFSET);

	writel( readl(KONA_MST_CLK_BASE_ADDR + KPM_CLK_MGR_REG_POLICY_CTL_OFFSET) |
		KPM_CLK_MGR_REG_POLICY_CTL_GO_MASK | KPM_CLK_MGR_REG_POLICY_CTL_GO_AC_MASK,
		KONA_MST_CLK_BASE_ADDR + KPM_CLK_MGR_REG_POLICY_CTL_OFFSET);

	while ( readl(KONA_MST_CLK_BASE_ADDR + KPM_CLK_MGR_REG_POLICY_CTL_OFFSET) &
		KPM_CLK_MGR_REG_POLICY_CTL_GO_MASK) {;}

	// Enable the USBH AHB clock
	writel( readl(KONA_MST_CLK_BASE_ADDR + KPM_CLK_MGR_REG_USB_EHCI_CLKGATE_OFFSET) |
		KPM_CLK_MGR_REG_USB_EHCI_CLKGATE_USBH_AHB_CLK_EN_MASK,
		KONA_MST_CLK_BASE_ADDR + KPM_CLK_MGR_REG_USB_EHCI_CLKGATE_OFFSET);

	// Disable write access to the KONA clock
	writel( 0xA5A500,
		KONA_MST_CLK_BASE_ADDR + KPM_CLK_MGR_REG_WR_ACCESS_OFFSET);
}


/*****************************************
 * board_init -early hardware init
 *****************************************/
int board_late_init (void)
{
	struct mmc *mmc = find_mmc_device(SAMOABOARD_SYS_SD_DEV);

#if 0
	struct clk* otg_clk = clk_get("usb_otg_clk");
	if (otg_clk)
		clk_enable(otg_clk);
	else
		printf("Couldn't find usb_otg_clk\n");
#endif

	otg_enable();


	if (!mmc) {
		puts("No MMC card found\n");
		return -1;
	}

	if (mmc_init(mmc)) {
		puts("MMC init failed\n");
		return  -1;
	}
	return 0;
}

#endif


