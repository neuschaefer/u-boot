#include <common.h>
#include <asm/io.h>
#include <asm/mach-types.h>
#include <mmc.h>
#include <i2c.h>
#include <asm/arch/brcm_rdb_kpm_clk_mgr_reg.h>
#include <asm/arch/brcm_rdb_padctrlreg.h>
#include <asm/arch/brcm_rdb_hsotg_ctrl.h>
#include <asm/arch/brcm_rdb_hsotg.h>
#include <asm/arch/brcm_rdb_sysmap.h>
#include <asm/arch/brcm_rdb_chipreg.h>

#include <asm/kona-common/clk.h>

#include <fastboot.h>

extern void init_clock_framework(void);
extern int kona_mmc_init(int dev_index);
extern int do_fastboot (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]);

#ifdef CONFIG_BCM59055_WDT
void bcm59055_wdt_init(void);
#endif

#ifdef CONFIG_HW_WATCHDOG
extern void hw_watchdog_init(void);
extern void hw_watchdog_disable(void);
#endif /* CONFIG_HW_WATCHDOG */

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


	init_clock_framework();

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
	unsigned int lock2;

	/* SDIO2 Clock 48 MHz = 96 MHz / 2 */
	struct clk *sdio2_clk, *sdio2_ahb_clk;

	sdio2_clk = clk_get("sdio2_clk");
	if (sdio2_clk) {
		clk_set_rate(sdio2_clk, 48000000);
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

	/* Write down DMA Boundary (256k) */
	writel(	(readl(SDIO2_BASE_ADDR + 0x4) & ~0x00007000) | 0x6,
		(SDIO2_BASE_ADDR + 0x4));


	/* Mux GPIO to SDIO 2 (MMC0) */
	lock2 = readl(PAD_CTRL_BASE_ADDR + PADCTRLREG_ACCESS_LOCK2_OFFSET);
	writel(0xA5A501, PAD_CTRL_BASE_ADDR + PADCTRLREG_WR_ACCESS_OFFSET);
	writel(	0, (PAD_CTRL_BASE_ADDR + PADCTRLREG_ACCESS_LOCK2_OFFSET));

	writel(	(0 << PADCTRLREG_MMC0CK_PINSEL_MMC0CK_SHIFT) |
		(0x3 << PADCTRLREG_MMC0CK_SEL_0_MMC0CK_SHIFT),
		PAD_CTRL_BASE_ADDR + PADCTRLREG_MMC0CK_OFFSET);

	writel(	(0 << PADCTRLREG_MMC0CMD_PINSEL_MMC0CMD_SHIFT) |
		(0x3 << PADCTRLREG_MMC0CMD_SEL_0_MMC0CMD_SHIFT) |
		PADCTRLREG_MMC0CMD_PUP_MMC0CMD_MASK,
		PAD_CTRL_BASE_ADDR + PADCTRLREG_MMC0CMD_OFFSET);

	writel(	(0 << PADCTRLREG_MMC0DAT0_PINSEL_MMC0DAT0_SHIFT) |
		(0x3 << PADCTRLREG_MMC0DAT0_SEL_0_MMC0DAT0_SHIFT) |
		PADCTRLREG_MMC0DAT0_PUP_MMC0DAT0_MASK,
		PAD_CTRL_BASE_ADDR + PADCTRLREG_MMC0DAT0_OFFSET);

	writel(	(0 << PADCTRLREG_MMC0DAT1_PINSEL_MMC0DAT1_SHIFT) |
		(0x3 << PADCTRLREG_MMC0DAT1_SEL_0_MMC0DAT1_SHIFT) |
		PADCTRLREG_MMC0DAT1_PUP_MMC0DAT1_MASK,
		PAD_CTRL_BASE_ADDR + PADCTRLREG_MMC0DAT1_OFFSET);

	writel(	(0 << PADCTRLREG_MMC0DAT2_PINSEL_MMC0DAT2_SHIFT) |
		(0x3 << PADCTRLREG_MMC0DAT2_SEL_0_MMC0DAT2_SHIFT) |
		PADCTRLREG_MMC0DAT2_PUP_MMC0DAT2_MASK,
		PAD_CTRL_BASE_ADDR + PADCTRLREG_MMC0DAT2_OFFSET);

	writel(	(0 << PADCTRLREG_MMC0DAT3_PINSEL_MMC0DAT3_SHIFT) |
		(0x3 << PADCTRLREG_MMC0DAT3_SEL_0_MMC0DAT3_SHIFT) |
		PADCTRLREG_MMC0DAT3_PUP_MMC0DAT3_MASK,
		PAD_CTRL_BASE_ADDR + PADCTRLREG_MMC0DAT3_OFFSET);

	writel(	(0 << PADCTRLREG_MMC0DAT4_PINSEL_MMC0DAT4_SHIFT) |
		(0x3 << PADCTRLREG_MMC0DAT4_SEL_0_MMC0DAT4_SHIFT) |
		PADCTRLREG_MMC0DAT4_PUP_MMC0DAT4_MASK,
		PAD_CTRL_BASE_ADDR + PADCTRLREG_MMC0DAT4_OFFSET);

	writel(	(0 << PADCTRLREG_MMC0DAT5_PINSEL_MMC0DAT5_SHIFT) |
		(0x3 << PADCTRLREG_MMC0DAT5_SEL_0_MMC0DAT5_SHIFT) |
		PADCTRLREG_MMC0DAT5_PUP_MMC0DAT5_MASK,
		PAD_CTRL_BASE_ADDR + PADCTRLREG_MMC0DAT5_OFFSET);

	writel(	(0 << PADCTRLREG_MMC0DAT6_PINSEL_MMC0DAT6_SHIFT) |
		(0x3 << PADCTRLREG_MMC0DAT6_SEL_0_MMC0DAT6_SHIFT) |
		PADCTRLREG_MMC0DAT6_PUP_MMC0DAT6_MASK,
		PAD_CTRL_BASE_ADDR + PADCTRLREG_MMC0DAT6_OFFSET);

	writel(	(0 << PADCTRLREG_MMC0DAT7_PINSEL_MMC0DAT7_SHIFT) |
		(0x3 << PADCTRLREG_MMC0DAT7_SEL_0_MMC0DAT7_SHIFT) |
		PADCTRLREG_MMC0DAT7_PUP_MMC0DAT7_MASK,
		PAD_CTRL_BASE_ADDR + PADCTRLREG_MMC0DAT7_OFFSET);

	writel(	(0 << PADCTRLREG_MMC0RST_PINSEL_MMC0RST_SHIFT) |
		(0x3 << PADCTRLREG_MMC0RST_SEL_0_MMC0RST_SHIFT) |
		PADCTRLREG_MMC0RST_PUP_MMC0RST_MASK,
		PAD_CTRL_BASE_ADDR + PADCTRLREG_MMC0RST_OFFSET);

	writel(	0xA5A501, PAD_CTRL_BASE_ADDR + PADCTRLREG_WR_ACCESS_OFFSET);
	writel(	lock2, (PAD_CTRL_BASE_ADDR + PADCTRLREG_ACCESS_LOCK2_OFFSET));

}


/*******************************************
 * sd_enable - power it on, set up clock and 
 * pinmux  
 *********************************************/
static void sd_enable(void)
{
	unsigned char val = 0;
	unsigned int lock3;
	struct clk *sdio1_clk, *sdio1_ahb_clk;

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

	/* Mux GPIO to SDIO1 (SD) */
	lock3 = readl(PAD_CTRL_BASE_ADDR + PADCTRLREG_ACCESS_LOCK3_OFFSET);
	writel(0xA5A501, PAD_CTRL_BASE_ADDR + PADCTRLREG_WR_ACCESS_OFFSET);
	writel(0x0, PAD_CTRL_BASE_ADDR + PADCTRLREG_ACCESS_LOCK3_OFFSET);

	writel(0x43, (PAD_CTRL_BASE_ADDR + PADCTRLREG_SDCK_OFFSET));
	writel(0x23, (PAD_CTRL_BASE_ADDR + PADCTRLREG_SDCMD_OFFSET));
	writel(0x23, (PAD_CTRL_BASE_ADDR + PADCTRLREG_SDDAT0_OFFSET));
	writel(0x23, (PAD_CTRL_BASE_ADDR + PADCTRLREG_SDDAT1_OFFSET));
	writel(0x23, (PAD_CTRL_BASE_ADDR + PADCTRLREG_SDDAT2_OFFSET));
	writel(0x23, (PAD_CTRL_BASE_ADDR + PADCTRLREG_SDDAT3_OFFSET));

	writel(0xA5A501, PAD_CTRL_BASE_ADDR + PADCTRLREG_WR_ACCESS_OFFSET);
	writel(lock3, PAD_CTRL_BASE_ADDR + PADCTRLREG_ACCESS_LOCK3_OFFSET);

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

	/* Register SD Card - SDIO1 */
	sd_enable();
	sts = kona_mmc_init(1);

	if (sts)
	{
		puts("SD card init failed");
	}
	
	return sts;
}

/*****************************************
 * board_init -early hardware init
 *****************************************/
int board_late_init (void)
{
	int value;
	struct mmc *mmc = find_mmc_device(RHEABOARD_SYS_SD_DEV);

	struct clk* otg_clk = clk_get("usb_otg_clk");
#ifdef CONFIG_BCM59055_WDT
	bcm59055_wdt_init();
#endif
	if (otg_clk)
		clk_enable(otg_clk);
	else
		printf("Couldn't find usb_otg_clk\n");

	value = readl(CHIPREGS_BASE_ADDR + CHIPREG_SPARE_CONTROL_HARD_RST1_OFFSET) & 0xF;
	printf("value at SPARE_RST1 =%x\n", value);

	/* 
	 *  Check for USB boot. Boot ROM sets the least significant nible to 1 if it is a USB boot
	 */
	if(value == 0x01) {
		printf("Moving to fastboot mode\n");
		do_fastboot(0,0,0,0);
	}

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
