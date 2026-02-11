#include <common.h>
#include <asm/io.h>
#include <asm/mach-types.h>
#include <mmc.h>
#include <asm/gpio.h>

#include <asm/arch/brcm_rdb_sysmap.h>
#include <asm/arch/brcm_rdb_pwrmgr.h>

#ifdef CONFIG_CAPRI_USB_HACK
#include <asm/arch/brcm_rdb_hsotg_ctrl.h>
#endif

/* Clocks Framework From Linux */
#include <asm/kona-common/clk.h>
extern void init_clock_framework(void);

/* MAP Clock Code */
#include <asm/arch/ccu_inline.h>
#include <asm/arch/ccu_sdio_inline.h>
#include <asm/arch/ccu_bsc_inline.h>
#include <asm/arch/ccu_esub_inline.h>

/* DDR speeds */
#include <asm/arch/chal_memc.h>
#include <asm/arch/chal_memc_ddr3.h>
#include <asm/arch/chal_sdram_inline.h>

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

#ifdef CONFIG_PINMUX
int pinmux_init(void);
#endif

extern const uint32_t boot_core1_stub;
extern const uint32_t boot_core1_stub_end;

#define BOOTROM_PATCH_FCN_REG_ADDR	0x340447f4
#define	BOOTROM_PATCH_FCN	0x3404bf00	//0x34053f00

extern int kona_keypad_init(void);

extern int bcm11140_eth_register(u8 dev_num);

#define SDIO_HW_DEFAULT_PLL_SELECT                    2  // ref_52m_clk
#define SDIO_HW_DEFAULT_MGR_CLK_DIVIDER               0  // divided by 1 (52MHz)

DECLARE_GLOBAL_DATA_PTR;

int pwm_clock_init(void)
{
	struct clk *pwm_apb, *pwm;

	/* Enable pwm clocks */
	pwm_apb = clk_get("pwm_apb_clk");
	if (pwm_apb)
		clk_enable(pwm_apb);
	else
	{
		printf("Couldn't find pwm_apb_clk\n");
		return -1;
	}

	pwm = clk_get("pwm_clk");
	if (pwm)
		clk_enable(pwm);
	else
	{
		printf("Couldn't find pwm_clk\n");
		return -1;
	}
	return 0;
}

unsigned char _esn_mac[10];
unsigned char _esn_mac_extra[32];

/*****************************************
 * board_init - early hardware init
 *****************************************/
int board_init (void)
{
	icache_enable();
	dcache_enable();

#if defined(CONFIG_BCM11130_RAY) || defined(CONFIG_BCM11130_RAY_JFFS2) || defined(CONFIG_BCM11130_ROKU_AUSTIN)
/* SWMAPYVR-824 - Temporarily disable LPDDR2 PHY eLDO for 23x23 packages. Add to hwconf later */
	writel(0, CHIPREGS_BASE_ADDR+CHIPREG_SYS_DDRLDO_CONTROL1_OFFSET);
#endif

	gd->bd->bi_arch_number = CONFIG_MACH_TYPE;      /* board id for linux */
	gd->bd->bi_boot_params = LINUX_BOOT_PARAM_ADDR; /* adress of boot parameters */

#ifdef CONFIG_PINMUX
	pinmux_init();
#endif

#define STATUS_LED_1 45 /* Blue on non-smartphone boards, Red on smartphone boards */
#define STATUS_LED_2 46 /* Green */
	/* Enable STATUS_LED_1, disable STATUS_LED_2 just before launching the kernel */
	gpio_request(STATUS_LED_1, "STATUS_LED_1");
	gpio_request(STATUS_LED_2, "STATUS_LED_2");
	gpio_direction_output(STATUS_LED_1, 1);
	gpio_direction_output(STATUS_LED_2, 0);

#ifdef CONFIG_HW_WATCHDOG
	hw_watchdog_init();
#endif /* CONFIG_HW_WATCHDOG*/

#ifdef CONFIG_KONA_KEYPAD
	/* No need to enable clocks since gpiokp_apb_clk is on by default */
	kona_keypad_init();
#endif /* CONFIG_CONFIG_KONA_KEYPAD */

	init_clock_framework();

	pwm_clock_init();
	memcpy(_esn_mac, (void*)0x87c00070, 10);
	memcpy(_esn_mac_extra, (void*)0x87c00080, 32);
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
	gd->ram_size = CONFIG_PHYS_SDRAM_1_SIZE - CONFIG_PHYS_SDRAM_RSVD_SIZE;

	return 0;
}

void dram_init_banksize (void)
{
	gd->bd->bi_dram[0].start = CONFIG_PHYS_SDRAM_1;
	gd->bd->bi_dram[0].size = CONFIG_PHYS_SDRAM_1_SIZE;
}

#if defined(CONFIG_KONA_MMC)
/*******************************************
 * mmc_init - Initializes mmc
 *********************************************/
int board_mmc_init(bd_t *bis)
{
	int sts;

	struct clk *sdio1_clk, *sdio1_ahb_clk, *sdio1_sleep_clk;

	/* Enable sdio1 Clocks */
	sdio1_ahb_clk = clk_get("sdio1_ahb_clk");
	if (sdio1_ahb_clk)
		clk_enable(sdio1_ahb_clk);
	else
		printf("Couldn't find sdio1_ahb_clk\n");

	sdio1_sleep_clk = clk_get("sdio1_sleep_clk");
	if (sdio1_sleep_clk)
		clk_enable(sdio1_sleep_clk);
	else
		printf("Couldn't find sdio1_sleep_clk\n");

	sdio1_clk = clk_get("sdio1_clk");
	if (sdio1_clk) {
		clk_set_rate(sdio1_clk, 52000000);
		clk_enable(sdio1_clk);
	}
	else
		printf("Couldn't find sdio1_clk\n");


	struct clk *sdio2_clk, *sdio2_ahb_clk, *sdio2_sleep_clk;

	/* Enable SDIO2 Clocks */
	sdio2_ahb_clk = clk_get("sdio2_ahb_clk");
	if (sdio2_ahb_clk)
		clk_enable(sdio2_ahb_clk);
	else
		printf("Couldn't find sdio2_ahb_clk\n");

	sdio2_sleep_clk = clk_get("sdio2_sleep_clk");
	if (sdio2_sleep_clk)
		clk_enable(sdio2_sleep_clk);
	else
		printf("Couldn't find sdio2_sleep_clk\n");


	sdio2_clk = clk_get("sdio2_clk");
	if (sdio2_clk) {
		clk_set_rate(sdio2_clk, 52000000);
		clk_enable(sdio2_clk);
	}
	else
		printf("Couldn't find sdio2_clk\n");

	/* Register eMMC - SDIO2 */
	sts = kona_mmc_init(2);
	if(sts)
		return sts;

	struct clk *sdio4_clk, *sdio4_ahb_clk, *sdio4_sleep_clk;
	/* Enable SDIO4 Clocks */
	sdio4_ahb_clk = clk_get("sdio4_ahb_clk");
	if (sdio4_ahb_clk)
		clk_enable(sdio4_ahb_clk);
	else
		printf("Couldn't find sdio4_ahb_clk\n");

	/* Enable sleep clock */
	sdio4_sleep_clk = clk_get("sdio4_sleep_clk");

	if (sdio4_sleep_clk)
		clk_enable(sdio4_sleep_clk);
	else
		printf("Couldn't find sdio4_sleep_clk\n");

	sdio4_clk = clk_get("sdio4_clk");
	if (sdio4_clk) {
		clk_set_rate(sdio4_clk, 48000000);
		clk_enable(sdio4_clk);
	}
	else
		printf("Couldn't find sdio4_clk\n");

	/* Register SD Card - SDIO4 kona_mmc_init assumes 0 based index */
	sts = kona_mmc_init(4);
	return sts;
}
#endif /* CONFIG_KONA_MMC */

static void enable_audio_clocks(void)
{
	struct clk *audioh_156m_clk;
	struct clk *audioh_26m_clk;
	struct clk *audioh_2p4m_clk;
	struct clk *audioh_apb_clk;
	struct clk *caph_srcmixer_clk;
	struct clk *ssp6_apb_clk;
	struct clk *ssp6_audio_clk;

	audioh_156m_clk = clk_get("audioh_156m_clk");
	if (audioh_156m_clk)
		clk_enable(audioh_156m_clk);
	else
		printf("Couldn't find audioh_156m_clk\n");

	audioh_26m_clk = clk_get("audioh_26m_clk");
	if (audioh_26m_clk)
		clk_enable(audioh_26m_clk);
	else
		printf("Couldn't find audioh_26m_clk\n");

	audioh_2p4m_clk = clk_get("audioh_2p4m_clk");
	if (audioh_2p4m_clk)
		clk_enable(audioh_2p4m_clk);
	else
		printf("Couldn't find audioh_2p4m_clk\n");

	audioh_apb_clk = clk_get("audioh_apb_clk");
	if (audioh_apb_clk)
		clk_enable(audioh_apb_clk);
	else
		printf("Couldn't find audioh_apb_clk\n");

	caph_srcmixer_clk = clk_get("caph_srcmixer_clk");
	if (caph_srcmixer_clk)
		clk_enable(caph_srcmixer_clk);
	else
		printf("Couldn't find caph_srcmixer_clk\n");

	ssp6_apb_clk = clk_get("ssp6_apb_clk");
	if (ssp6_apb_clk)
		clk_enable(ssp6_apb_clk);
	else
		printf("Couldn't find ssp6_apb_clk\n");

	ssp6_audio_clk = clk_get("ssp6_audio_clk");
	if (ssp6_audio_clk)
	{
		clk_set_rate(ssp6_audio_clk, 15360000);
		clk_enable(ssp6_audio_clk);
	}
	else
	{
		printf("Couldn't find ssp6_audio_clk\n");
	}

	if (ssp6_audio_clk)
		clk_enable(ssp6_audio_clk);
	else
		printf("Couldn't find ssp6_audio_clk\n");

}

/* 
 * For boards with LDO power daughtercards instead of PMUs, it is 
 * possible to program ARM-A9 and the VC4 voltages using
 * I2C control of the MAX8649 regulators. 
 */

int i2c_read (uchar chip, uint addr, int alen, uchar *buffer, int len);
int i2c_write (uchar chip, uint addr, int alen, uchar *buffer, int len);


/* PMU I2C chip address */
#define PMU_ADDR		0x08
#define PMU_ADDR1		0x0c
#define PMU_SDLDO_PM_CTRL1_REG  0x6a
#define PMU_SDXLDO_PM_CTRL1_REG 0x6c
#define PMU_SDXLDO_CTRL         0x9c
#define PMU_A9_REG		0xc0
#define PMU_VC4_REG		0xd2
#define PMU_HDMICTRL1_REG	0x5A /* Actually 0x15A, but with PMU_ADDR1 it is 0x5A */
#define PMU_VDDVAR_REG		0xc9

/* ARM A9 core voltages */
#define LDO_A9_ADDR	0x62
#define VDD_A9_MIN	120	/* 1.20V */
#define VDD_A9_DEFAULT	CONFIG_VDD_A9_DEFAULT
#define VDD_A9_MAX	130	/* 1.30V */

/* VC4/LPDDR2 core voltages */
#define LDO_VC4_ADDR	0x60
#define VDD_VC4_MIN	114	/* 1.14V */
#define VDD_VC4_DEFAULT	CONFIG_VDD_VC4_DEFAULT
#define VDD_VC4_MAX	130	/* 1.30V */

#define LDO_MODE2_REG 	2

/* Convert voltage to ldo register setting */
/* e.g. 123 = 0xB0 (1.23V) */
static inline uchar ldo_volts2reg(int volts)
{
	uchar reg;
	reg = volts - 75;
	reg |= 0x80;	/* Set op_mode2 - forced pwm mode, ignore external sync */
	return reg;
}
/* Convert ldo register setting for voltage to a human readable number */
/* e.g. 0xB0 = 123  (1.23V) */
static inline int ldo_reg2volts(int val)
{
	int volts;
	val &= 0x3f;	/* mask out upper bits */
	volts = 75 + val;
	return volts;	/* actually returns volts*100 */
}
/* Convert voltage to pmu register setting */
/* e.g. 123 = 0x28 (1.23V) */
/* 
 * Note that 0x27 = 1.23V in the datasheet, but because of losses on the board, 
 * we use 0x28 for 1.24V initially but it drops to 1.23V where it meets the cpu
 * so we adjust for it here. So the 84 factor is changed to 83 to accomodate this.
 */
static inline uchar pmu_volts2reg(int volts)
{
	uchar reg;
	reg = (volts - 83) | 0x40;	/* keep persistence setting */
	return reg;
}
/* Convert pmu register setting for voltage to a human readable number */
/* e.g. 0x27 = 123  (1.23V) */
static inline int pmu_reg2volts(int val)
{
	int volts;
	val &= 0x3f;	/* mask out upper bits */
	volts = 83 + val;
	return volts;	/* actually returns volts*100 */
}
static inline int intVolts(int volts)
{
	return volts/100;
}
static inline int fracVolts(int volts)
{
	return volts - 100;
}

typedef uchar (*VOLTS2REG)(int);
typedef int (*REG2VOLTS)(int);

#if 0
/* Debug functions - replace i2c functions with these if needed */
static int test_i2c_write(unsigned char chip, unsigned int addr, int alen, unsigned char *buffer, int len)
{
	int rc;
	printf("%s chip=0x%x addr=0x%x alen=%d *buffer=0x%02x len=%d\n", __func__, chip, addr, alen, *buffer, len);
	rc = i2c_write(chip, addr, alen, buffer, len);
	printf("%s i2c_read returned with rc=%d\n", __func__, rc);
	return rc;
}	
static int test_i2c_read (unsigned char chip, unsigned int addr, int alen, unsigned char *buffer, int len)
{
	int rc;
	printf("%s chip=0x%x addr=0x%x alen=%d buffer=%p len=%d\n", __func__, chip, addr, alen, buffer, len);
	rc = i2c_read(chip, addr, alen, buffer, len);
	printf("%s i2c_read returned with rc=%d *buffer=0x%02x\n", __func__, rc, *buffer);
	return rc;
}
#endif

static void set_a9_vc4_voltages(uchar a9_i2c_addr, uchar vc_i2c_addr, int a9_reg, int vc_reg, VOLTS2REG volts2reg, REG2VOLTS reg2volts)
{
	/* 
	 * The A9 core and VC4 have scalable voltage controls.
	 * We default both of them to 1.23V, but the safe
	 * ranges for sustained operation/testing are:
	 * VDDVARA9:  1.20V - 1.30V	(ARM)
	 * VDDVAR:    1.14V - 1.30V	(VC4)
	 *
	 * We define environment variables "vdda9" and "vddvc4" for
	 * overriding the default 1.23V settings. For example, 
	 * you can do the following to set the arm and vc4 to 
	 * 1.29 and 1.19 volts respectively.
	 *
	 * u-boot> setenv vdd_a9 129    
	 * u-boot> setenv vdd_vc4 119
	 *
	 * Note that for sustained operation above 1.30 volts
	 * may damage the chip and is not supported. For specialized
	 * voltage testing and the risk of permanently damaging
	 * your board, feel free to use the i2c menu at your own risk.
	 */
	char *s;
	int vdd_a9 = VDD_A9_DEFAULT;
	int vdd_vc4 = VDD_VC4_DEFAULT;
	uchar alen = 1;
	uchar data;
	uchar size = 1;

	if ((s = getenv ("vdd_a9")) != NULL) {
		vdd_a9 = simple_strtoul (s, NULL, 10);
	}
	if ((vdd_a9 < VDD_A9_MIN) || (vdd_a9 > VDD_A9_MAX)) {
		printf("vdd_a9 voltage must be >= %d and <= %d, using default of %d\n",
		VDD_A9_MIN, VDD_A9_MAX, VDD_A9_DEFAULT);
	}
	else {
		data = volts2reg(vdd_a9);
		if (i2c_write(a9_i2c_addr, a9_reg, alen, (uchar *)&data, size) != 0)
			puts ("Error writing vdd_a9 voltage via i2c\n");
	}

	if ((s = getenv ("vdd_vc4")) != NULL) {
		vdd_vc4 = simple_strtoul (s, NULL, 10);
	}
	if ((vdd_vc4 < VDD_VC4_MIN) || (vdd_vc4 > VDD_VC4_MAX)) {
		printf("freq must be >= %dV and <= %dV, using default of %dV\n",
		VDD_VC4_MIN, VDD_VC4_MAX, VDD_VC4_DEFAULT);
	}
	else {
		data = volts2reg(vdd_vc4);
		if (i2c_write(vc_i2c_addr, vc_reg, alen, (uchar *)&data, size) != 0)
			puts ("Error writing vdd_vc4 voltage via i2c\n");
	}

	printf("\nVoltage Info:\n");
	if (i2c_read(a9_i2c_addr, a9_reg, alen, (uchar *)&data, size) != 0)
		puts ("Error reading voltage via i2c\n");
	else
		printf("    VDD A9  = %d.%dV\n", intVolts(reg2volts(data)), fracVolts(reg2volts(data)));

	if (i2c_read(vc_i2c_addr, vc_reg, alen, (uchar *)&data, size) != 0)
		puts ("Error reading voltage via i2c\n");
	else
		printf("    VDD VC4 = %d.%dV\n", intVolts(reg2volts(data)), fracVolts(reg2volts(data)));
}

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

#define MHZ_DEFAULT	CONFIG_MHZ_A9_DEFAULT
#define MHZ_DEFAULT_A2	CONFIG_MHZ_A9_DEFAULT_A2
#define MHZ_MIN		500	/* Minimum frequency */

/* Note that mdiv=1 is not stable so disable the switch for now */
//#define MHZ_BREAK	1145	/* Break frequency for mdiv=1 */
#define MHZ_BREAK	1300	/* Break frequency for mdiv=1 */

#define MHZ_MAX		1300	/* Maximum allowed, in practice it is lower */

static void adjust_arm_clocks(void)
{
	uint32_t freq_id;
	uint32_t freq_mhz;
	uint32_t div;
	uint32_t pl310_div;

	char *s;
	int mhz;
	int pdiv = 1;
	int mdiv = 2;
	int ndiv_int;
	int mdiv_override = 0;

	if ((chipregHw_getChipId() & 0xff) == 0xa2) {
		mhz = MHZ_DEFAULT_A2;
	} else {
		mhz = MHZ_DEFAULT;
	}

	if ((s = getenv ("mhz")) != NULL)
	{
		mhz = simple_strtoul (s, NULL, 10);
	}
	if ((s = getenv ("mdiv")) != NULL)
	{
		mdiv = simple_strtoul (s, NULL, 10);
		mdiv_override = 1;
	}

	if ((mhz < MHZ_MIN) || (mhz > MHZ_MAX))
	{
		printf("freq must be >= %d MHz and <= %d MHz, using default of %dMHz\n",
			MHZ_MIN, MHZ_MAX, MHZ_DEFAULT);
		mhz = MHZ_DEFAULT;
	}
	/*
	 * Don't change mdiv if it was set in an env variable.
	 * This allows tweaking the frequency at which mdiv goes to 1
	 */
	if ((mhz >= MHZ_BREAK) && !mdiv_override)
	{
		mdiv = 1;
	}

	/*
	 * f = 26MHz * ndiv_int / (pdiv * mdiv)
	 *
	 * ndiv_int = f * (pdiv * mdiv) / 26MHz
	 *
	 * For MHZ_MIN to MHZ_BREAK-1, use pdiv=1, mdiv=2. e.g. ndiv_int = f * 2 / 26
	 * At or above MHZ_BREAK, use pdiv=1, mdiv=1       e.g. ndiv_int = f * 1 / 26
	 */
	ndiv_int = mhz * pdiv * mdiv / 26;

	ccu_set_kproc_pll(ndiv_int, 0, pdiv, mdiv);

	/* Choose maximum A9 PLL frequency for all policies */
	ccu_set_kproc_policy_freq( ccu_policy_3, ccu_kproc_policy_freq_aclkh_aclkhdiv2_aclkhdiv3 );
	ccu_set_kproc_policy_freq( ccu_policy_2, ccu_kproc_policy_freq_aclkh_aclkhdiv2_aclkhdiv3 );
	ccu_set_kproc_policy_freq( ccu_policy_1, ccu_kproc_policy_freq_aclkh_aclkhdiv2_aclkhdiv3 );
	ccu_set_kproc_policy_freq( ccu_policy_0, ccu_kproc_policy_freq_aclkh_aclkhdiv2_aclkhdiv3 );

	freq_id  = ccu_get_kproc_policy_freq(ccu_policy_3);
	freq_mhz = ccu_get_kproc_policy_freq_a9_hz(freq_id) / 1000000;
	div      = ccu_get_kproc_axi_div(freq_id)+1;

	/*
	 * When CPU freqeuncy is > 800 MHz, set PL310 L2 divisor to / 3.
	 * when CPU frequency is <= 800 MHz, set PL310 L2 divisor to / 2.
	 */
	if (freq_mhz > 800)
		ccu_set_kproc_pl310_div(2); /* real divisor val = 2 + 1 = 3 */
	else
		ccu_set_kproc_pl310_div(1);

	pl310_div = ccu_get_kproc_pl310_div();

	printf("\nCPU Info:\n");
	printf("    Freq ID  = %4d\n", freq_id);
	printf("    Cpu freq = %4d MHz\n", freq_mhz);
	printf("    AXI freq = %4d MHz (Cpu freq divided by %d)\n", freq_mhz/div, div);
	printf("    ndiv_int=%d pdiv=%d mdiv=%d pl310_div=%d\n\n", ndiv_int, pdiv, mdiv, pl310_div+1);
}

#ifdef CONFIG_CMD_SPM
extern void spmInit(void);
extern void spmStart(void);
extern void spmStop(void);
extern void spmShow(void);

static void spmBootupMsg(void)
{
	spmInit();
	spmStart();
   	udelay(500000);
	spmShow();
	spmStop();
}	
#endif

#ifdef CONFIG_PRINT_DDR_INFO
static CHAL_MEMC_HANDLE getMemcHandle (char* controllerId)
{
   MEMC_ID idVal = MEMC_ID_SYS;
   CHAL_MEMC_HANDLE memHandle = NULL;
   _Bool dualChipSelect = FALSE;
  

   if ( strcmp(controllerId, "sys") == 0 )
   {
      idVal = MEMC_ID_SYS;
#if 0
      CHAL_SDRAM_CONFIG_T sdramSysConfig = CFG_GLOBAL_SDRAM_SYSEMI_CONFIG;
      if ( sdramSysConfig.sdram_type == MEMC_DDR_TYPE_LPDDR2)
      {
          dualChipSelect = sdramSysConfig.dev_config.lpddr2_dev_config.dual_chip_select;
      }
#else	
         dualChipSelect = TRUE;
#endif
   }
   else if ( strcmp(controllerId, "vc") == 0 )
   {
      idVal = MEMC_ID_VC;
#if 0
      CHAL_SDRAM_CONFIG_T sdramVcConfig = CFG_GLOBAL_SDRAM_VCEMI_CONFIG;
      if ( sdramVcConfig.sdram_type == MEMC_DDR_TYPE_LPDDR2)
      {
          dualChipSelect = sdramVcConfig.dev_config.lpddr2_dev_config.dual_chip_select;
      }
#else	
         dualChipSelect = TRUE;
#endif
   }
   else
   {
      printf("Invalid controller ID\n");
      return NULL;
   }
   memHandle = chal_sdram_getMemcHandle(idVal);

   /* Use the sdram configuration to determine dual chip select is configured */
   chal_sdram_dual_chipselect_config ( idVal, dualChipSelect );

   return memHandle;
}


#define CON_CMD_RC_FAILURE      -1     /* Failure Return Code (RC) */
#define CON_CMD_RC_SUCCESS	0     /* Success Return Code (RC) */

/* Print DDR information */
static int ddrInfo (char* controllerId) /* "sys" or "vc" */
{
   CHAL_MEMC_HANDLE memHandle = NULL;
   CHAL_LPDDR2_DEV_INFO_T configValue;
   MEMC_CS_CONNECTION cs;
   uint32_t rdValue;
   uint32_t size;
   uint32_t i;

   memHandle = getMemcHandle( controllerId );
   if ( memHandle == NULL  )
   {
      return CON_CMD_RC_FAILURE;
   }
   if ( memHandle->memc_ddr_type == MEMC_DDR_TYPE_LPDDR2 )
   {
#if 0
      printf ("   ******************\n");
      printf ("   Type: LPDDR2\n");
      for (i=0; i<MEMC_CS_MAX; i++)
      {
         cs = memHandle->mem_device[i].dev_cs;
         if (cs == MEMC_CS_NONE)
            continue;

         chal_lpddr2_get_dev_info (memHandle, cs, &configValue );

         printf ("   Chip Select: %d\n", cs);
         /* Vendor ID */
         printf ("   Vendor Id: %d\n", configValue.vendor_id);
         printf ("   Mfr: %s\n", configValue.vendor_name);

         printf ("   Mem Typ: ");
         switch(configValue.type)
         {
         case LPDDR2_TYPE_S4:
            printf ("LPDDR2 S4");
            break;
         case LPDDR2_TYPE_S2:
            printf ("LPDDR2 S2");
            break;
         case LPDDR2_TYPE_NVM:
            printf ("LPDDR2 NVM");
            break;
         }
         printf ("\n");

         /* Log Memory density */
         chal_lpddr2_get_mem_size(memHandle, cs, (uint32_t*)&size);
         printf ("   Density(MB): ");
         printf ("%d\n", size);

         /* Log I/O width */
         printf ("   I/O width: ");
         switch(configValue.io_width)
         {
         case LPDDR2_WIDTH_X32:
            printf ("32");
            break;
         case LPDDR2_WIDTH_X16:
            printf ("16");
            break;
         case LPDDR2_WIDTH_X8:
            printf ("8");
            break;
         }
         printf ("\n");

         chal_memc_get_clock_speed( memHandle, &rdValue );
         printf ("   Clock Speed: %d Hz\n", rdValue);
         printf ("   ******************\n");
      }
#endif
   }
   else if ( memHandle->memc_ddr_type == MEMC_DDR_TYPE_DDR3 )
   {
      chal_memc_ddr3_get_dev_info ( memHandle );

      printf ("   ******************\n");
      printf ("   Type: DDR3\n");

      /* JEDEC Name */
      printf ("   Jedec: %s\n", memHandle->mem_device_ddr3.dev_info.jedec_name);

      printf ("      CL: %d\n", memHandle->mem_device_ddr3.dev_info.cl);
      printf ("      CWL: %d\n", memHandle->mem_device_ddr3.dev_info.cwl);
      printf ("      WR: %d\n", memHandle->mem_device_ddr3.dev_info.wr);

      printf ("   Chip Width : ");
      switch(memHandle->mem_device_ddr3.dev_config.chip_width)
      {
      case CHAL_MEMC_DDR3_CHIP_WIDTH_8:
         printf ("8");
         break;
      case CHAL_MEMC_DDR3_CHIP_WIDTH_16:
         printf ("16");
         break;
      }
      printf (" bits\n");

      printf ("   Bus Width : ");
      switch(memHandle->mem_device_ddr3.dev_config.bus_width)
      {
      case CHAL_MEMC_DDR3_BUS_WIDTH_8:
         printf ("8");
         break;
      case CHAL_MEMC_DDR3_BUS_WIDTH_16:
         printf ("16");
         break;
      case CHAL_MEMC_DDR3_BUS_WIDTH_32:
         printf ("32");
         break;
      }
      printf (" bits\n");

      printf ("   Rank : ");
      switch(memHandle->mem_device_ddr3.dev_config.rank)
      {
      case CHAL_MEMC_DDR3_RANK_SINGLE:
         printf ("Single");
         break;
      case CHAL_MEMC_DDR3_RANK_DUAL:
         printf ("Dual");
         break;
      }
      printf ("\n");

      printf ("   Each Chip Size : ");
      switch(memHandle->mem_device_ddr3.dev_config.chip_size)
      {
      case CHAL_MEMC_DDR3_CHIP_SIZE_1Gb:
         printf ("1Gb");
         break;
      case CHAL_MEMC_DDR3_CHIP_SIZE_2Gb:
         printf ("2Gb");
         break;
      case CHAL_MEMC_DDR3_CHIP_SIZE_4Gb:
         printf ("4Gb");
         break;
      case CHAL_MEMC_DDR3_CHIP_SIZE_8Gb:
         printf ("8Gb");
         break;
      }
      printf ("\n");

      /* Convert bytes to MBytes*/
      printf ("   Total Bytes: %d MB\n", (uint32_t)(memHandle->total_bytes>>20));

      chal_memc_get_clock_speed( memHandle, &rdValue );
      printf ("   Clock Speed: %d Hz\n", rdValue);
      printf ("   ******************\n");
   }
   
   return CON_CMD_RC_SUCCESS;
}
#endif

/* Hold ESC key down during bootup to recover bad env variable settings */
static int escKey = 0;

/*****************************************
 * board_late_init -late hardware init
 *****************************************/
int board_late_init (void)
{
	struct mmc *mmc;
	uchar data;

	/* ESC key pressed during boot - ignore "mhz", "vdd_a9" and "vdd_vc4" variables. */
	if (tstc()) {
		if (getc() == 0x1b) {
			printf("ESC key detected - ignoring env variable clock/voltage adjustments\n");
			escKey = 1;	
		}
	}
		
	/* Turn on Power Islands (MM, ESUB, Modem) */
	writel(
		readl(PWRMGR_BASE_ADDR + PWRMGR_PI_DEFAULT_POWER_STATE_OFFSET) |
		PWRMGR_PI_DEFAULT_POWER_STATE_PI_MM_SUB2_WAKEUP_OVERRIDE_MASK |
#ifdef CONFIG_BCM11140_ETH
		PWRMGR_PI_DEFAULT_POWER_STATE_PI_ESUB_WAKEUP_OVERRIDE_MASK |
		PWRMGR_PI_DEFAULT_POWER_STATE_PI_ESUB_SUB_WAKEUP_OVERRIDE_MASK |
#endif /* CONFIG_BCM11140_ETH */
		PWRMGR_PI_DEFAULT_POWER_STATE_PI_MODEM_WAKEUP_OVERRIDE_MASK |
		PWRMGR_PI_DEFAULT_POWER_STATE_PI_MM_WAKEUP_OVERRIDE_MASK |
		PWRMGR_PI_DEFAULT_POWER_STATE_PI_MM_SUB_WAKEUP_OVERRIDE_MASK,
		PWRMGR_BASE_ADDR + PWRMGR_PI_DEFAULT_POWER_STATE_OFFSET);

	/* Enable USB OTG Clock */
	enable_usb_clocks();

	/* Enable I2C Clocks */
	enable_i2c_clocks();

	if (i2c_read(PMU_ADDR, 0, 1, (uchar *)&data, 1) == 0) {
		/* enable SD power rails */
		data = 0;
		if (i2c_write(PMU_ADDR, PMU_SDLDO_PM_CTRL1_REG, 1, (uchar *)&data, 1) != 0)
			puts("Error enabling SDLDO rail\n");

		/* program SD I/O voltage to 3.3 V */
		data = 0xB8;
		if (i2c_write(PMU_ADDR, PMU_SDXLDO_CTRL, 1, (uchar *)&data, 1) != 0)
                        puts("Error enabling SDXLDO rail\n");

		data = 0;
		if (i2c_write(PMU_ADDR, PMU_SDXLDO_PM_CTRL1_REG, 1, (uchar *)&data, 1) != 0)
                        puts("Error enabling SDXLDO rail\n");
	}


#if defined(CONFIG_KONA_MMC) 
	mmc = find_mmc_device(BCM11140_SYS_SD_DEV);
	if (!mmc)
		puts("No MMC card found\n");
	else if (mmc_init(mmc))
		puts("MMC init failed\n");
#endif /* CONFIG_KONA_MMC */

	/* Enable AudioH clocks */
	enable_audio_clocks();

	if (!escKey) {
		/* adjust A9 & VC4 voltages if vdd_a9 or vdd_vc4 env variables request it */
		if (i2c_read(PMU_ADDR, 0, 1, (uchar *)&data, 1) == 0) {
			printf("Auto-detected PMU daughtercard\n");
			set_a9_vc4_voltages(PMU_ADDR, PMU_ADDR, PMU_A9_REG, PMU_VC4_REG, pmu_volts2reg, pmu_reg2volts);

			/* Also enable HDMI 5V rail */
			printf("    HDMI    = 5V\n");
			data = 1;
			if (i2c_write(PMU_ADDR1, PMU_HDMICTRL1_REG, 1, (uchar *)&data, 1) != 0)
				puts ("Error enabling HDMI 5V rail\n");
		}
		else {
			printf("Auto-detected LDO daughtercard\n");
#ifndef CONFIG_BCM11130_ROKU_AUSTIN
			set_a9_vc4_voltages(LDO_A9_ADDR, LDO_VC4_ADDR, LDO_MODE2_REG, LDO_MODE2_REG, ldo_volts2reg, ldo_reg2volts);
#endif 
		}	
	}

	if (!escKey) {
   		adjust_arm_clocks();
	}


	{
		uint32_t patch_fcn_size = ((uint32_t) &boot_core1_stub_end - (uint32_t) &boot_core1_stub) / sizeof(uint32_t);

		uint32_t i;
		uint32_t *src_ptr = (uint32_t *) &boot_core1_stub;
		uint32_t *dst_ptr = (uint32_t *)BOOTROM_PATCH_FCN;

		/* SW work-around: setup the MMU tables for core 1. Without this
		 * we get a hang when the bootrom tries to detect/load the patch 
		 * function address in sram.  
		 * 
		 * Note that this is done here as an interim patch until swdev
		 * mode is no longer needed and other modes are fully supported.
		 */

		printf("Setting L2 MMU table for boot ROM -- not set in SWDEV mode\n");
		{
			int i;
			uint32_t *dest = (uint32_t *)0x34044c00;

			/* Setup the page tables for the bootrom. If the bootrom fixes this, then this won't be necessary */
			/* This pattern starts at 0x34044c00 */
			const int mmu_data[] = {
			0x34000023, 0x34001023, 0x34002023, 0x34003023,
			0x34004023, 0x34005023, 0x34006023, 0x34007023,
			0x34008023, 0x34009023, 0x3400a023, 0x3400b023,
			0x3400c023, 0x3400d023, 0x3400e023, 0x3400f023,
			0x34010023, 0x34011023, 0x34012023, 0x34013023,
			0x34014023, 0x34015023, 0x34016023, 0x34017023,
			0x34018023, 0x34019023, 0x3401a023, 0x3401b023,
			0x3401c023, 0x3401d023, 0x3401e023, 0x3401f023,
			0x00000000, 0x00000000, 0x00000000, 0x00000000,
			0x00000000, 0x00000000, 0x00000000, 0x00000000,
			0x00000000, 0x00000000, 0x00000000, 0x00000000,
			0x00000000, 0x00000000, 0x00000000, 0x00000000,
			0x00000000, 0x00000000, 0x00000000, 0x00000000,
			0x00000000, 0x00000000, 0x00000000, 0x00000000,
			0x00000000, 0x00000000, 0x00000000, 0x00000000,
			0x00000000, 0x00000000, 0x00000000, 0x00000000,
			0x34040126, 0x34041126, 0x34042122, 0x34043122,
			0x34044126, 0x34045126, 0x34046126, 0x34047126,
			0x34048126, 0x34049126, 0x3404a126, 0x3404b126,
			0x3404c133, 0x3404d133, 0x3404e133, 0x3404f133,
			0x34050133, 0x34051133, 0x34052133, 0x34053133,
			0x00000000, 0x00000000, 0x00000000, 0x00000000,
			0x00000000, 0x00000000, 0x00000000, 0x00000000,
			0x00000000, 0x00000000, 0x00000000, 0x00000000,
			0x34040126, 0x34041126, 0x34042126, 0x34043126,
			0x34044126, 0x00000000, 0x34045126, 0x34046126,
			0x34047126, 0x34048126, 0x34049126, 0x3404a126,
			0x3404b126, 0x00000000, 0x00000000, 0x00000000
			};

			for (i=0; i<(sizeof(mmu_data)/sizeof(mmu_data[0])); i++)
			{
				*dest++ = mmu_data[i];
			}
			/* zero fill to boundary */
			while (dest < (uint32_t *)0x34045000)
			{
				*dest++ = 0;
			}
		}

		printf("Core 1 patch function addr: 0x%08x\n", (uint32_t) &boot_core1_stub);
		printf("Core 1 patch function end addr: 0x%08x\n", (uint32_t) &boot_core1_stub_end);
		printf("Core 1 patch SRAM loc: 0x%08x\n", (uint32_t)BOOTROM_PATCH_FCN);
		
		/* copy patch function to SRAM */
		for (i=0; i<patch_fcn_size; i++)
		{
			*dst_ptr++ = *src_ptr++;
		}
		*((volatile uint32_t *)BOOTROM_PATCH_FCN_REG_ADDR) = (uint32_t)BOOTROM_PATCH_FCN;

		//print_buffer (0x34040000, (void *)0x34040000, 4, 0x14000/sizeof(uint32_t), 4);

	}
#ifdef CONFIG_CMD_SPM
	/* Show software performanc metrics on boot up */
	spmBootupMsg();
#endif

#ifdef CONFIG_PRINT_DDR_INFO
	/* Show DDR configuration and speed information on boot up */
	ddrInfo("sys");
#ifndef CONFIG_LI_MODE
	ddrInfo("vc");
#endif
#endif

#ifdef CONFIG_CAPRI_USB_HACK
	/* If it looks like we have a PMU then try to set VDDVar to 1.32V */
	if (i2c_read(PMU_ADDR, 0, 1, (uchar*) &data, 1) == 0)
	{
		data = 0x30;
		if (i2c_write(PMU_ADDR, PMU_VDDVAR_REG, 1, (uchar *) &data, 1) != 0)
			printf("Error setting VDDVar to 1.32V\n");
	}

	/* Set OTG PHY's iLDO to 1.04V */
	{
		uint32_t p1ctl = readl(HSOTG_CTRL_BASE_ADDR+HSOTG_CTRL_PHY_P1CTL_OFFSET);
		p1ctl &= ~(HSOTG_CTRL_PHY_P1CTL_AFE_LDOBG_OUTADJ_MASK | HSOTG_CTRL_PHY_P1CTL_AFE_LDOCNTLEN_1P2_I_MASK | HSOTG_CTRL_PHY_P1CTL_AFE_LDOCNTL_1P2_I_MASK);
		p1ctl |= (0x8 << HSOTG_CTRL_PHY_P1CTL_AFE_LDOBG_OUTADJ_SHIFT);		// LDO Output = 1.04v
		p1ctl |= (0x1 << HSOTG_CTRL_PHY_P1CTL_AFE_LDOCNTLEN_1P2_I_SHIFT);	// LDO Control Enable
		p1ctl |= (0x2 << HSOTG_CTRL_PHY_P1CTL_AFE_LDOCNTL_1P2_I_SHIFT);		// LDO output mux to MONPLL
		writel(p1ctl, (HSOTG_CTRL_BASE_ADDR+HSOTG_CTRL_PHY_P1CTL_OFFSET));
	}
#endif /* CONFIG_CAPRI_USB_HACK */

	return 0;
}

#ifdef CONFIG_BCM11140_ETH
int board_eth_init(bd_t *bis)
{
	int rc = -1;

#if 1 /* USE LINUX INSPIRED CLOCK FRAMEWORK */
	struct clk * esub_ccu;

	esub_ccu = clk_get("esub_ccu_clk");
	if (esub_ccu)
		clk_enable(esub_ccu);
	else
		printf("Couldn't find esub_ccu_clk\n");

	// framework can't take pll out of reset- use chal
	ccu_esub_pll_init();

#else /* FALL BACK TO CHAL CCU CODE */
	/* Configure ethernet subsystem clocks */
	ccu_esub_pll_init();
#endif

	rc = bcm11140_eth_register(0);
	return rc;
}
#endif /* CONFIG_BCM11140_ETH */

#ifdef CONFIG_BRCM_KERNEL_ENTRY_HOOK
void kernel_entry_hook(void)
{
	/* Enable STATUS_LED_2, disable STATUS_LED_1 just before launching the kernel */
	gpio_direction_output(STATUS_LED_1, 0);
	gpio_direction_output(STATUS_LED_2, 1);
}
#endif

