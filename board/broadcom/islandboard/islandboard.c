#include <common.h>
#include <asm/io.h>
#include <asm/mach-types.h>
#include <mmc.h>
#include <asm/gpio.h>

#include <asm/kona-common/clk.h>
extern void init_clock_framework(void);

#ifdef CONFIG_FASTBOOT
#include <fastboot.h>
#endif /* CONFIG_FASTBOOT */

#ifdef CONFIG_KONA_MMC
extern int kona_mmc_init(int dev_index);
#endif /* CONFIG_KONA_MMC */

#ifdef CONFIG_HW_WATCHDOG
#define OFFSTR "off"
extern void hw_watchdog_init(void);
extern void hw_watchdog_disable(void);
#endif /* CONFIG_HW_WATCHDOG */


DECLARE_GLOBAL_DATA_PTR;

/*****************************************
 * board_init - early hardware init
 *****************************************/
int board_init (void)
{
	gd->bd->bi_arch_number = CONFIG_MACH_TYPE;      /* board id for linux */
	gd->bd->bi_boot_params = LINUX_BOOT_PARAM_ADDR; /* adress of boot parameters */

	gpio_request(105, "LED");
	gpio_direction_output(105, 1);

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
	if (((s = getenv ("watchdog")) != NULL) && (strncmp (s, OFFSTR, strlen(OFFSTR)) == 0))
	{
		printf("Disabling watchdog\n");
		hw_watchdog_disable();
	}
#endif /* CONFIG_HW_WATCHDOG*/

	return 0;
}

/**********************************************
 * dram_init - sets uboots idea of sdram size
 **********************************************/
int dram_init (void)
{
    gd->ram_size = PHYS_SDRAM_1_SIZE; //get_ram_size((long *)PHYS_SDRAM_1, PHYS_SDRAM_1_SIZE);
	return 0;
}

void dram_init_banksize (void)
{
	gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
	gd->bd->bi_dram[0].size = PHYS_SDRAM_1_SIZE;
}

#ifdef CONFIG_KONA_MMC
/*******************************************
 * board_mmc_init - Initializes mmc
 *********************************************/
int board_mmc_init(bd_t *bis)
{
	int sts;
	struct clk *sdio2_clk, *sdio2_ahb_clk;
	struct clk *sdio3_clk, *sdio3_ahb_clk;

	/* Enable SDIO2 Clocks */
	sdio2_ahb_clk = clk_get("sdio2_ahb_clk");
	if (sdio2_ahb_clk)
		clk_enable(sdio2_ahb_clk);
	else
		printf("Couldn't find sdio2_ahb_clk\n");

	sdio2_clk = clk_get("sdio2_clk");
	if (sdio2_clk) {
		clk_set_rate(sdio2_clk, 48000000);
		clk_enable(sdio2_clk);
	}
	else
		printf("Couldn't find sdio2_clk\n");

	/* Register eMMC - SDIO2 */
	sts = kona_mmc_init(2);
	if(sts)
		return sts;

	/* Enable SDIO3 Clocks */
	sdio3_ahb_clk = clk_get("sdio3_ahb_clk");
	if (sdio3_ahb_clk)
		clk_enable(sdio3_ahb_clk);
	else
		printf("Couldn't find sdio3_ahb_clk\n");

	sdio3_clk = clk_get("sdio3_clk");
	if (sdio3_clk) {
		clk_set_rate(sdio2_clk, 48000000);
		clk_enable(sdio3_clk);
	}
	else
		printf("Couldn't find sdio3_clk\n");

	/* Register SD Card - SDIO3 */
	sts = kona_mmc_init(3);
	return sts;
}
#endif

static void enable_usb_clocks(void)
{
	struct clk *otg_clk;

	otg_clk = clk_get("usb_otg_clk");
	if (otg_clk)
		clk_enable(otg_clk);
	else
		printf("Couldn't find usb_otg_clk\n");
}

static void enable_i2c_clocks(void)
{
	struct clk *bsc1_clk, *bsc1_apb_clk;
	struct clk *bsc2_clk, *bsc2_apb_clk;
	struct clk *ssp0_clk, *ssp0_apb_clk;

	/* Enable BSC1 I2C Clocks */
	bsc1_apb_clk = clk_get("bsc1_apb_clk");
	if (bsc1_apb_clk)
		clk_enable(bsc1_apb_clk);
	else
		printf("Couldn't find bsc1_apb_clk\n");

	bsc1_clk = clk_get("bsc1_clk");
	if (bsc1_clk)
		clk_enable(bsc1_clk);
	else
		printf("Couldn't find bsc1_clk\n");


	/* Enable BSC2 I2C Clocks */
	bsc2_apb_clk = clk_get("bsc2_apb_clk");
	if (bsc2_apb_clk)
		clk_enable(bsc2_apb_clk);
	else
		printf("Couldn't find bsc2_apb_clk\n");

	bsc2_clk = clk_get("bsc2_clk");
	if (bsc2_clk)
		clk_enable(bsc2_clk);
	else
		printf("Couldn't find bsc2_clk\n");

	/* Enable SSP0 I2C Clocks */
	ssp0_apb_clk = clk_get("ssp0_apb_clk");
	if (ssp0_apb_clk)
		clk_enable(ssp0_apb_clk);
	else
		printf("Couldn't find ssp0_apb_clk\n");

	ssp0_clk = clk_get("ssp0_clk");
	if (ssp0_clk)
		clk_enable(ssp0_clk);
	else
		printf("Couldn't find ssp0_clk\n");
}

/*****************************************
 * board_late_init - late hardware init
 *****************************************/
int board_late_init (void)
{
	struct mmc *mmc;

	/* Enable USB OTG Clock */
	enable_usb_clocks();

	/* Enable I2C Clocks */
	enable_i2c_clocks();

#ifdef CONFIG_KONA_MMC
	mmc = find_mmc_device(ISLANDBOARD_SYS_SD_DEV);
	if (!mmc)
		puts("No MMC card found\n");
	else if (mmc_init(mmc))
		puts("MMC init failed\n");
#endif /* CONFIG_KONA_MMC */

	return 0;
}
