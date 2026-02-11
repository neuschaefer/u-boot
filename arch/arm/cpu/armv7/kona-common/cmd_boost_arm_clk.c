#include <common.h>
#include <command.h>
#include <asm/io.h>
#include <asm/arch/brcm_rdb_kproc_clk_mgr_reg.h>
#include <asm/arch/brcm_rdb_sysmap.h>


/*******************************************************************/
/* boost_arm_clock - Boost ARM clock */
/*******************************************************************/
static int boosted;

#define outw(addr, val) writel(val, addr)
#define inw(addr) readl(addr)

/* enable/disable access to PROC_CLK MGR */
static inline void proc_clk_enable_access(int enable)
{
	if (enable)
		writel (0x00A5A501, PROC_CLK_BASE_ADDR+KPROC_CLK_MGR_REG_WR_ACCESS_OFFSET);
	else
		writel (0, PROC_CLK_BASE_ADDR+KPROC_CLK_MGR_REG_WR_ACCESS_OFFSET);
}
/* swtich ARM to policy x */
static inline void proc_clk_switch_to_policy(int policy)
{
	if (policy>=0 && policy<=7) {
		int val;

		/* Software update enable for policy related data */
		outw (PROC_CLK_BASE_ADDR + KPROC_CLK_MGR_REG_LVM_EN_OFFSET, KPROC_CLK_MGR_REG_LVM_EN_POLICY_CONFIG_EN_MASK);
		do {
			val = inw (PROC_CLK_BASE_ADDR + KPROC_CLK_MGR_REG_LVM_EN_OFFSET);
		} while (val & KPROC_CLK_MGR_REG_LVM_EN_POLICY_CONFIG_EN_MASK);

		val = (policy<<24) | (policy<<16) | (policy<<8) | policy;

		/* program policy ID */
		outw (PROC_CLK_BASE_ADDR + KPROC_CLK_MGR_REG_POLICY_FREQ_OFFSET, val);
		outw (PROC_CLK_BASE_ADDR + KPROC_CLK_MGR_REG_POLICY_CTL_OFFSET,
			KPROC_CLK_MGR_REG_POLICY_CTL_GO_MASK |
			KPROC_CLK_MGR_REG_POLICY_CTL_GO_ATL_MASK);

		/*polling ctrl go bit back to 0 */
		do {
			val = inw (PROC_CLK_BASE_ADDR + KPROC_CLK_MGR_REG_POLICY_CTL_OFFSET);
		} while (val & KPROC_CLK_MGR_REG_POLICY_CTL_GO_MASK);
	}

}

/* post divider for policy 6 and 7*/
static inline void proc_clk_set_policy_div(int policy, int div)
{
	if (policy==6) {
		outw (PROC_CLK_BASE_ADDR + KPROC_CLK_MGR_REG_PLLARMC_OFFSET, div);
	}
	else if( policy==7) {
		outw (PROC_CLK_BASE_ADDR + KPROC_CLK_MGR_REG_PLLARMCTRL5_OFFSET, div);
		outw (PROC_CLK_BASE_ADDR + KPROC_CLK_MGR_REG_PLLARMCTRL5_OFFSET,
			div|KPROC_CLK_MGR_REG_PLLARMC_PLLARM_LOAD_EN_MASK);
		outw (PROC_CLK_BASE_ADDR + KPROC_CLK_MGR_REG_PLLARMCTRL5_OFFSET, div);
	}
}

/* enable arm_pll  */
static inline void enable_arm_pll(int freq, int xtal)
{
#define	VC0_FREQ_THRE 	(1750*1000*1000)
#define	NDIV_SHIFT		20
	int pdiv = 1, ndiv_int, ndiv_frac;

	if (freq >= VC0_FREQ_THRE) {
		// required if VCO > 1.75Ghz
		outw (PROC_CLK_BASE_ADDR + KPROC_CLK_MGR_REG_PLLARMCTRL3_OFFSET, 0x08102000);
	}

	ndiv_int= freq / xtal;
	ndiv_frac = ( ((uint64_t)(freq % xtal)) << NDIV_SHIFT) / xtal + 1;

	// fraction
	outw (PROC_CLK_BASE_ADDR + KPROC_CLK_MGR_REG_PLLARMB_OFFSET, ndiv_frac<<KPROC_CLK_MGR_REG_PLLARMB_PLLARM_NDIV_FRAC_SHIFT);

	// integer
	outw (PROC_CLK_BASE_ADDR + KPROC_CLK_MGR_REG_PLLARMA_OFFSET,
		(pdiv<<KPROC_CLK_MGR_REG_PLLARMA_PLLARM_PDIV_SHIFT)
		| (ndiv_int<<KPROC_CLK_MGR_REG_PLLARMA_PLLARM_NDIV_INT_SHIFT)
		| KPROC_CLK_MGR_REG_PLLARMA_PLLARM_SOFT_RESETB_MASK
		| KPROC_CLK_MGR_REG_PLLARMA_PLLARM_SOFT_POST_RESETB_MASK);

	/* wait for pll to lock */
	while (! (inw(PROC_CLK_BASE_ADDR +KPROC_CLK_MGR_REG_PLLARMA_OFFSET)
		& KPROC_CLK_MGR_REG_PLLARMA_PLLARM_LOCK_MASK) );
}
static void cpu_clock_init (unsigned long pll, int div)
{
	unsigned long xtal;

	xtal = XTAL_RATE;

	if (boosted)
		return;

	/* enable write access */
	proc_clk_enable_access(1);

	/* boost PLL to 2x rate*/
	enable_arm_pll(pll, xtal);

	/* set up divider */
	proc_clk_set_policy_div (7, div); // policy 7 = 2000/2 = 1000Mhz

	/* switch to policy 7  */
	proc_clk_switch_to_policy (7);

	proc_clk_enable_access(0);

	boosted = 1;
}

static int do_boost_arm_clk (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	unsigned long pll_rate, pll_div;
	int     rc = 0;
	char *c = 0;

	if ((c = getenv ("arm_pll_rate")) != NULL)
		pll_rate = simple_strtoul(c, NULL, 10);
	else
		pll_rate = DEFAULT_PLL_RATE;

	if ((c = getenv ("arm_pll_div")) != NULL)
		pll_div = simple_strtoul(c, NULL, 10);
	else
		pll_div = DEFAULT_PLL_DIV;

	printf ("boost arm clock pll_rate %lu div %lu arm_clk %lu\n", pll_rate, pll_div, pll_rate/pll_div);
	cpu_clock_init (pll_rate, pll_div);
	return rc;
}

U_BOOT_CMD( boost_arm_clk, CONFIG_SYS_MAXARGS, 1, do_boost_arm_clk,
	"Boost ARM clock",
	"Boost ARM clock, pll value is taken from env - arm_pll"
);
